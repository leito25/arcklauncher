#include "ProfileStats.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include <limits>

namespace {
constexpr double MaxSafeInteger = 9007199254740991.0;
}

ProfileStats::ProfileStats(const QString &profilePath, QObject *parent)
    : QObject(parent),
      m_profilePath(QFileInfo(profilePath).absoluteFilePath()),
      m_profileDirectory(QFileInfo(profilePath).absolutePath())
{
    m_refreshTimer.setSingleShot(true);
    m_refreshTimer.setInterval(100);
    connect(&m_refreshTimer, &QTimer::timeout, this, &ProfileStats::refresh);
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &ProfileStats::scheduleRefresh);
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, &ProfileStats::scheduleRefresh);
    refreshWatchPaths();
    refresh();
}

QString ProfileStats::formattedPlaytime() const
{
    return m_formattedPlaytime;
}

QVariantMap ProfileStats::gamePlaytimes() const
{
    return m_gamePlaytimes;
}

QString ProfileStats::errorMessage() const
{
    return m_errorMessage;
}

QString ProfileStats::formatPlaytime(quint64 seconds) const
{
    const quint64 minutes = seconds / 60;
    const quint64 hours = minutes / 60;
    const quint64 remainingMinutes = minutes % 60;
    return hours > 0
        ? tr("%1h %2m").arg(hours).arg(remainingMinutes)
        : tr("%1m").arg(minutes);
}

void ProfileStats::scheduleRefresh()
{
    m_refreshTimer.start();
}

void ProfileStats::refreshWatchPaths()
{
    const QStringList watchedDirectories = m_watcher.directories();
    for (const QString &directory : watchedDirectories)
        m_watcher.removePath(directory);

    QDir directory(m_profileDirectory);
    while (!directory.exists() && directory.cdUp()) {
    }
    if (directory.exists())
        m_watcher.addPath(directory.absolutePath());

    if (QFileInfo::exists(m_profileDirectory) && !m_watcher.directories().contains(m_profileDirectory))
        m_watcher.addPath(m_profileDirectory);

    const QStringList watchedFiles = m_watcher.files();
    for (const QString &file : watchedFiles)
        m_watcher.removePath(file);
    if (QFileInfo(m_profilePath).isFile())
        m_watcher.addPath(m_profilePath);
}

void ProfileStats::setError(const QString &message)
{
    if (m_errorMessage == message)
        return;
    m_errorMessage = message;
    emit errorMessageChanged();
}

void ProfileStats::refresh()
{
    refreshWatchPaths();

    quint64 totalSeconds = 0;
    QVariantMap gamePlaytimes;
    if (QFileInfo::exists(m_profilePath)) {
        QFile profileFile(m_profilePath);
        if (!profileFile.open(QIODevice::ReadOnly)) {
            setError(tr("Cannot read profile.json: %1").arg(profileFile.errorString()));
            return;
        }

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(profileFile.readAll(), &parseError);
        const QJsonObject profile = document.object();
        if (parseError.error != QJsonParseError::NoError || !document.isObject() ||
            profile.value(QStringLiteral("schemaVersion")).toInt(-1) != 1 ||
            !profile.value(QStringLiteral("games")).isObject()) {
            setError(tr("Invalid or unsupported profile.json."));
            return;
        }

        const QJsonObject games = profile.value(QStringLiteral("games")).toObject();
        for (auto game = games.constBegin(); game != games.constEnd(); ++game) {
            if (!game.value().isObject()) {
                setError(tr("Invalid game statistics in profile.json."));
                return;
            }
            const QJsonValue playtime = game.value().toObject().value(QStringLiteral("playtimeSeconds"));
            const double seconds = playtime.toDouble(-1);
            if (!playtime.isDouble() || seconds < 0 || seconds > MaxSafeInteger ||
                seconds != static_cast<double>(static_cast<quint64>(seconds))) {
                setError(tr("Invalid game playtime in profile.json."));
                return;
            }
            const quint64 gameSeconds = static_cast<quint64>(seconds);
            if (gameSeconds > std::numeric_limits<quint64>::max() - totalSeconds)
                totalSeconds = std::numeric_limits<quint64>::max();
            else
                totalSeconds += gameSeconds;
            gamePlaytimes.insert(game.key(), formatPlaytime(gameSeconds));
        }
    }

    const QString formatted = formatPlaytime(totalSeconds);
    if (m_formattedPlaytime != formatted || m_gamePlaytimes != gamePlaytimes) {
        m_formattedPlaytime = formatted;
        m_gamePlaytimes = gamePlaytimes;
        emit statsChanged();
    }
    setError({});
}
