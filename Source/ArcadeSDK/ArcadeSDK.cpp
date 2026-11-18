#include "ArcadeSDK.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <string>
#include <system_error>

#ifdef _WIN32
#  define NOMINMAX
#  include <windows.h>
#  include <shlobj.h>
#else
#  include <fcntl.h>
#  include <sys/file.h>
#  include <unistd.h>
#endif

namespace {
namespace fs = std::filesystem;
using json = nlohmann::json;
constexpr uint64_t maxValue = 9007199254740991ULL;
std::mutex sessionMutex;
std::string gameId;
fs::path profileDirectory;
thread_local std::string lastError;
std::atomic<unsigned long> tempCounter{0};

struct Failure : std::runtime_error {
    ArcadeResult code;
    Failure(ArcadeResult result, const std::string& message)
        : std::runtime_error(message), code(result) {}
};

bool validId(const std::string& id)
{
    if (id.empty())
        return false;
    for (unsigned char c : id) {
        if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9')
              || c == '-' || c == '_' || c == '.'))
            return false;
    }
    return true;
}

fs::path defaultDirectory()
{
#ifdef _WIN32
    PWSTR raw = nullptr;
    if (FAILED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &raw)))
        throw Failure(ARCADE_IO_ERROR, "Could not locate LocalAppData.");
    fs::path result(raw);
    CoTaskMemFree(raw);
    return result / "ArcadePortfolio";
#else
    const char* data = std::getenv("XDG_DATA_HOME");
    if (data && fs::path(data).is_absolute())
        return fs::path(data) / "ArcadePortfolio";
    const char* home = std::getenv("HOME");
    if (!home || !fs::path(home).is_absolute())
        throw Failure(ARCADE_IO_ERROR, "Could not locate the user data directory.");
    return fs::path(home) / ".local" / "share" / "ArcadePortfolio";
#endif
}

class ProfileLock {
public:
    explicit ProfileLock(const fs::path& path)
    {
#ifdef _WIN32
        handle = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS,
                             FILE_ATTRIBUTE_NORMAL, nullptr);
        if (handle == INVALID_HANDLE_VALUE)
            throw Failure(ARCADE_IO_ERROR, "Could not open profile lock.");
        OVERLAPPED overlapped{};
        if (!LockFileEx(handle, LOCKFILE_EXCLUSIVE_LOCK, 0, MAXDWORD, MAXDWORD, &overlapped)) {
            CloseHandle(handle);
            throw Failure(ARCADE_IO_ERROR, "Could not acquire profile lock.");
        }
#else
        fd = open(path.c_str(), O_CREAT | O_RDWR, 0600);
        if (fd < 0)
            throw Failure(ARCADE_IO_ERROR, "Could not open profile lock.");
        while (flock(fd, LOCK_EX) != 0) {
            if (errno == EINTR)
                continue;
            close(fd);
            throw Failure(ARCADE_IO_ERROR, "Could not acquire profile lock.");
        }
#endif
    }
    ~ProfileLock()
    {
#ifdef _WIN32
        OVERLAPPED overlapped{};
        UnlockFileEx(handle, 0, MAXDWORD, MAXDWORD, &overlapped);
        CloseHandle(handle);
#else
        flock(fd, LOCK_UN);
        close(fd);
#endif
    }
    ProfileLock(const ProfileLock&) = delete;
    ProfileLock& operator=(const ProfileLock&) = delete;
private:
#ifdef _WIN32
    HANDLE handle = INVALID_HANDLE_VALUE;
#else
    int fd = -1;
#endif
};

void saveProfile(const fs::path& path, const json& profile)
{
    const std::string contents = profile.dump(2) + "\n";
    std::string errorMessage = "Could not write or replace profile.";
#ifdef _WIN32
    const auto pid = GetCurrentProcessId();
#else
    const auto pid = getpid();
#endif
    fs::path temp;
#ifdef _WIN32
    HANDLE file = INVALID_HANDLE_VALUE;
    for (int attempt = 0; attempt < 100; ++attempt) {
        temp = path;
        temp += ".tmp-" + std::to_string(pid) + "-" + std::to_string(tempCounter++);
        file = CreateFileW(temp.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW,
                           FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file != INVALID_HANDLE_VALUE || GetLastError() != ERROR_FILE_EXISTS)
            break;
    }
    if (file == INVALID_HANDLE_VALUE)
        throw Failure(ARCADE_IO_ERROR, "Could not create temporary profile: "
                      + std::system_category().message(static_cast<int>(GetLastError())));
    bool ok = true;
    size_t written = 0;
    while (written < contents.size()) {
        DWORD chunk = static_cast<DWORD>((std::min)(contents.size() - written, size_t{1024 * 1024}));
        DWORD count = 0;
        if (!WriteFile(file, contents.data() + written, chunk, &count, nullptr)) {
            errorMessage = "Could not write temporary profile: "
                + std::system_category().message(static_cast<int>(GetLastError()));
            ok = false;
            break;
        }
        if (count == 0) {
            errorMessage = "Could not write temporary profile: no bytes written.";
            ok = false;
            break;
        }
        written += count;
    }
    if (ok && !FlushFileBuffers(file)) {
        errorMessage = "Could not flush temporary profile: "
            + std::system_category().message(static_cast<int>(GetLastError()));
        ok = false;
    }
    if (!CloseHandle(file)) {
        if (ok)
            errorMessage = "Could not close temporary profile: "
                + std::system_category().message(static_cast<int>(GetLastError()));
        ok = false;
    }
    if (ok) {
        // Other Windows readers can temporarily prevent replacing the profile.
        for (int attempt = 0; attempt < 50; ++attempt) {
            if (MoveFileExW(temp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
                break;
            const DWORD error = GetLastError();
            if ((error == ERROR_SHARING_VIOLATION || error == ERROR_ACCESS_DENIED) && attempt < 49) {
                Sleep(20);
                continue;
            }
            errorMessage = "Could not replace profile: "
                + std::system_category().message(static_cast<int>(error));
            ok = false;
            break;
        }
    }
#else
    int file = -1;
    for (int attempt = 0; attempt < 100; ++attempt) {
        temp = path;
        temp += ".tmp-" + std::to_string(pid) + "-" + std::to_string(tempCounter++);
        file = open(temp.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);
        if (file >= 0 || errno != EEXIST)
            break;
    }
    if (file < 0)
        throw Failure(ARCADE_IO_ERROR, "Could not create temporary profile.");
    bool ok = true;
    size_t written = 0;
    while (written < contents.size()) {
        const ssize_t count = write(file, contents.data() + written, contents.size() - written);
        if (count < 0 && errno == EINTR)
            continue;
        if (count <= 0) {
            ok = false;
            break;
        }
        written += static_cast<size_t>(count);
    }
    if (ok && fsync(file) != 0)
        ok = false;
    if (close(file) != 0)
        ok = false;
    if (ok && rename(temp.c_str(), path.c_str()) != 0)
        ok = false;
#endif
    if (!ok) {
        std::error_code ignored;
        fs::remove(temp, ignored);
        throw Failure(ARCADE_IO_ERROR, errorMessage);
    }
}

json loadProfile(const fs::path& path)
{
    std::error_code ec;
    const bool exists = fs::exists(path, ec);
    if (ec)
        throw Failure(ARCADE_IO_ERROR, "Could not inspect profile: " + ec.message());
    if (!exists)
        return {{"schemaVersion", 1}, {"games", json::object()}};
    std::ifstream input(path, std::ios::binary);
    if (!input)
        throw Failure(ARCADE_IO_ERROR, "Could not open profile.");
    json profile;
    try {
        profile = json::parse(input);
    } catch (const json::exception& e) {
        throw Failure(ARCADE_INVALID_PROFILE, std::string("Invalid profile JSON: ") + e.what());
    }
    if (input.bad())
        throw Failure(ARCADE_IO_ERROR, "Could not read profile.");
    if (!profile.is_object() || !profile.contains("schemaVersion")
        || !profile["schemaVersion"].is_number_integer() || profile["schemaVersion"] != 1
        || !profile.contains("games") || !profile["games"].is_object())
        throw Failure(ARCADE_INVALID_PROFILE, "Invalid or unsupported profile schema.");
    for (auto it = profile["games"].begin(); it != profile["games"].end(); ++it) {
        const auto& game = it.value();
        if (!validId(it.key()) || !game.is_object()
            || !game.contains("playtimeSeconds") || !game["playtimeSeconds"].is_number_unsigned()
            || game["playtimeSeconds"].get<uint64_t>() > maxValue
            || !game.contains("highScore")
            || !(game["highScore"].is_null() || (game["highScore"].is_number_unsigned()
                && game["highScore"].get<uint64_t>() <= maxValue))
            || !game.contains("achievements") || !game["achievements"].is_array())
            throw Failure(ARCADE_INVALID_PROFILE, "Invalid game statistics in profile.");
        for (const auto& achievement : game["achievements"]) {
            if (!achievement.is_string() || !validId(achievement.get_ref<const std::string&>()))
                throw Failure(ARCADE_INVALID_PROFILE, "Invalid achievement in profile.");
        }
    }
    return profile;
}

void update(const std::function<void(json&)>& change)
{
    fs::create_directories(profileDirectory);
    ProfileLock lock(profileDirectory / "profile.lock");
    const fs::path path = profileDirectory / "profile.json";
    json profile = loadProfile(path);
    auto& games = profile["games"];
    if (!games.contains(gameId))
        games[gameId] = {{"playtimeSeconds", 0}, {"highScore", nullptr}, {"achievements", json::array()}};
    change(games[gameId]);
    saveProfile(path, profile);
}

template<typename F>
ArcadeResult call(F&& action)
{
    lastError.clear();
    try {
        std::lock_guard<std::mutex> guard(sessionMutex);
        action();
        return ARCADE_OK;
    } catch (const Failure& e) {
        lastError = e.what();
        return e.code;
    } catch (const fs::filesystem_error& e) {
        lastError = e.what();
        return ARCADE_IO_ERROR;
    } catch (const std::exception& e) {
        lastError = e.what();
        return ARCADE_INTERNAL_ERROR;
    } catch (...) {
        lastError = "Unknown SDK error.";
        return ARCADE_INTERNAL_ERROR;
    }
}

void requireSession()
{
    if (gameId.empty())
        throw Failure(ARCADE_INVALID_STATE, "SDK is not initialized.");
}
}

extern "C" {
ArcadeResult ArcadeInit(const char* id, const char* directory)
{
    return call([&] {
        if (!gameId.empty())
            throw Failure(ARCADE_INVALID_STATE, "SDK is already initialized.");
        if (!id || !validId(id) || (directory && !*directory))
            throw Failure(ARCADE_INVALID_ARGUMENT, "Invalid game ID or profile directory.");
        fs::path selected = directory ? fs::u8path(directory) : defaultDirectory();
        if (!selected.is_absolute())
            throw Failure(ARCADE_INVALID_ARGUMENT, "Profile directory must be absolute.");
        try {
            gameId = id;
            profileDirectory = selected;
            update([](json&) {});
        } catch (...) {
            gameId.clear();
            profileDirectory.clear();
            throw;
        }
    });
}

ArcadeResult ArcadeAddPlaytime(uint64_t seconds)
{
    return call([&] {
        requireSession();
        update([&](json& game) {
            const uint64_t current = game["playtimeSeconds"].get<uint64_t>();
            if (seconds > maxValue - current)
                throw Failure(ARCADE_INVALID_ARGUMENT, "Playtime exceeds the supported range.");
            game["playtimeSeconds"] = current + seconds;
        });
    });
}

ArcadeResult ArcadeUpdateHighScore(uint64_t score)
{
    return call([&] {
        requireSession();
        if (score > maxValue)
            throw Failure(ARCADE_INVALID_ARGUMENT, "Score exceeds the supported range.");
        update([&](json& game) {
            if (game["highScore"].is_null() || score > game["highScore"].get<uint64_t>())
                game["highScore"] = score;
        });
    });
}

ArcadeResult ArcadeUnlockAchievement(const char* id)
{
    return call([&] {
        requireSession();
        if (!id || !validId(id))
            throw Failure(ARCADE_INVALID_ARGUMENT, "Invalid achievement ID.");
        update([&](json& game) {
            auto& achievements = game["achievements"];
            for (const auto& existing : achievements)
                if (existing == id)
                    return;
            achievements.push_back(id);
        });
    });
}

ArcadeResult ArcadeShutdown(void)
{
    return call([] {
        requireSession();
        gameId.clear();
        profileDirectory.clear();
    });
}

const char* ArcadeLastError(void) { return lastError.c_str(); }
}
