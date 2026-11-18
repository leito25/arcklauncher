#include "ArcadeSDK.h"
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <chrono>

#ifdef _WIN32
#  define NOMINMAX
#  include <windows.h>
#else
#  include <sys/wait.h>
#  include <unistd.h>
#endif

namespace fs = std::filesystem;
using json = nlohmann::json;

void check(bool condition, const char* message)
{
    if (!condition)
        throw std::runtime_error(std::string(message) + ": " + ArcadeLastError());
}

json readProfile(const fs::path& directory)
{
    std::ifstream input(directory / "profile.json");
    return json::parse(input);
}

void worker(const fs::path& directory)
{
    const std::string path = directory.u8string();
    check(ArcadeInit("concurrent", path.c_str()) == ARCADE_OK, "worker init");
    for (int i = 0; i < 40; ++i)
        check(ArcadeAddPlaytime(1) == ARCADE_OK, "worker update");
    check(ArcadeShutdown() == ARCADE_OK, "worker shutdown");
}

void runWorkers(const fs::path& executable, const std::string& folder)
{
#ifdef _WIN32
    PROCESS_INFORMATION processes[2]{};
    for (auto& process : processes) {
        std::wstring command = L"\"" + executable.wstring() + L"\" --worker "
            + std::wstring(folder.begin(), folder.end());
        STARTUPINFOW startup{};
        startup.cb = sizeof(startup);
        check(CreateProcessW(executable.c_str(), command.data(), nullptr, nullptr, FALSE,
                             0, nullptr, nullptr, &startup, &process) != 0, "start worker");
    }
    bool workersOk = true;
    for (auto& process : processes) {
        const DWORD wait = WaitForSingleObject(process.hProcess, 30000);
        DWORD status = 1;
        if (wait != WAIT_OBJECT_0 || !GetExitCodeProcess(process.hProcess, &status) || status != 0)
            workersOk = false;
        CloseHandle(process.hThread);
        CloseHandle(process.hProcess);
    }
    check(workersOk, "worker failed");
#else
    pid_t processes[2]{};
    for (auto& process : processes) {
        process = fork();
        check(process >= 0, "start worker");
        if (process == 0) {
            execl(executable.c_str(), executable.c_str(), "--worker", folder.c_str(), nullptr);
            _exit(127);
        }
    }
    for (auto process : processes) {
        int status = 0;
        check(waitpid(process, &status, 0) == process && WIFEXITED(status) && WEXITSTATUS(status) == 0,
              "worker failed");
    }
#endif
}

int main(int argc, char** argv)
{
    fs::path directory;
    bool created = false;
    try {
        if (argc == 3 && std::string(argv[1]) == "--worker") {
            worker(fs::current_path() / argv[2]);
            return 0;
        }
#ifdef _WIN32
        const auto pid = GetCurrentProcessId();
        wchar_t module[32768];
        const DWORD length = GetModuleFileNameW(nullptr, module, 32768);
        check(length > 0 && length < 32768, "locate test executable");
        const fs::path executable(std::wstring(module, length));
#else
        const auto pid = getpid();
        const fs::path executable = fs::absolute(argv[0]);
#endif
        const std::string folder = "arcade-sdk-test-" + std::to_string(pid);
        directory = fs::current_path() / folder;
        created = fs::create_directory(directory);
        check(created, "create isolated test directory");
        const std::string path = directory.u8string();

        check(ArcadeAddPlaytime(1) == ARCADE_INVALID_STATE, "uninitialized update");
        check(ArcadeInit("Invalid ID", path.c_str()) == ARCADE_INVALID_ARGUMENT, "invalid ID");
        check(ArcadeInit("sample", "relative") == ARCADE_INVALID_ARGUMENT, "relative profile path");
        check(ArcadeInit("sample", path.c_str()) == ARCADE_OK, "init");
        check(ArcadeInit("sample", path.c_str()) == ARCADE_INVALID_STATE, "double init");
        check(ArcadeAddPlaytime(12) == ARCADE_OK, "playtime");
        check(ArcadeUpdateHighScore(100) == ARCADE_OK, "high score");
        check(ArcadeUpdateHighScore(20) == ARCADE_OK, "lower score");
        check(ArcadeUnlockAchievement("first-win") == ARCADE_OK, "achievement");
        check(ArcadeUnlockAchievement("first-win") == ARCADE_OK, "duplicate achievement");
        check(ArcadeUnlockAchievement("bad id") == ARCADE_INVALID_ARGUMENT, "invalid achievement");
        check(ArcadeAddPlaytime(9007199254740991ULL) == ARCADE_INVALID_ARGUMENT, "overflow");
#ifdef _WIN32
        HANDLE reader = CreateFileW((directory / "profile.json").c_str(), GENERIC_READ,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL, nullptr);
        check(reader != INVALID_HANDLE_VALUE, "open profile without delete sharing");
        std::thread releaseReader([reader] {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            CloseHandle(reader);
        });
        const ArcadeResult transientResult = ArcadeAddPlaytime(1);
        releaseReader.join();
        check(transientResult == ARCADE_OK, "retry transient profile sharing failure");
        reader = CreateFileW((directory / "profile.json").c_str(), GENERIC_READ,
                             FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL, nullptr);
        check(reader != INVALID_HANDLE_VALUE, "open locked profile");
        const ArcadeResult blockedResult = ArcadeAddPlaytime(1);
        CloseHandle(reader);
        check(blockedResult == ARCADE_IO_ERROR, "report persistent profile sharing failure");
#endif
        check(ArcadeShutdown() == ARCADE_OK, "shutdown");
        const json game = readProfile(directory)["games"]["sample"];
#ifdef _WIN32
        const int expectedPlaytime = 13;
#else
        const int expectedPlaytime = 12;
#endif
        check(game["playtimeSeconds"] == expectedPlaytime && game["highScore"] == 100
              && game["achievements"] == json::array({"first-win"}), "persisted stats");

        json extended = readProfile(directory);
        extended["futureField"] = "preserved";
        extended["games"]["sample"]["extra"] = true;
        { std::ofstream output(directory / "profile.json", std::ios::trunc); output << extended; }
        check(ArcadeInit("sample", path.c_str()) == ARCADE_OK, "reopen profile");
        check(ArcadeAddPlaytime(1) == ARCADE_OK, "update extended profile");
        check(ArcadeShutdown() == ARCADE_OK, "shutdown reopened profile");
        const json preserved = readProfile(directory);
        check(preserved["futureField"] == "preserved" && preserved["games"]["sample"]["extra"] == true,
              "preserve additive fields");

        runWorkers(executable, folder);
        check(readProfile(directory)["games"]["concurrent"]["playtimeSeconds"] == 80,
              "concurrent updates");

        const fs::path profile = directory / "profile.json";
        { std::ofstream output(profile, std::ios::trunc); output << "{\"broken\":"; }
        check(ArcadeInit("sample", path.c_str()) == ARCADE_INVALID_PROFILE, "corrupt profile");
        { std::ifstream input(profile); std::string content((std::istreambuf_iterator<char>(input)), {});
          check(content == "{\"broken\":", "corrupt profile must not be overwritten"); }
        { std::ofstream output(profile, std::ios::trunc); output << R"({"schemaVersion":2,"games":{}})"; }
        check(ArcadeInit("sample", path.c_str()) == ARCADE_INVALID_PROFILE, "future schema");

        fs::remove_all(directory);
        std::cout << "Arcade SDK tests passed\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        if (created) {
            std::error_code error;
            fs::remove_all(directory, error);
        }
        return 1;
    }
}
