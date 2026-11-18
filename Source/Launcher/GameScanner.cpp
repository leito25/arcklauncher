#include "GameScanner.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>

namespace {
constexpr int TitleRole = Qt::UserRole + 1;
constexpr int GameIdRole = Qt::UserRole + 2;

bool validGameId(const QString &id)
{
    if (id.isEmpty())
        return false;
    for (const QChar character : id) {
        if (!((character >= QLatin1Char('a') && character <= QLatin1Char('z')) ||
              (character >= QLatin1Char('0') && character <= QLatin1Char('9')) ||
              character == QLatin1Char('.') || character == QLatin1Char('_') ||
              character == QLatin1Char('-')))
            return false;
    }
    return true;
}
}

GameScanner::GameScanner(const QString &gamesDirectory, QObject *parent)
    : QAbstractListModel(parent), m_gamesDirectory(gamesDirectory)
{
    connect(&m_process, &QProcess::started, this, &GameScanner::runningChanged);
    connect(&m_process, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus status) {
        if (status == QProcess::CrashExit)
            setError(tr("The game stopped unexpectedly."));
        else if (exitCode != 0) {
            QString message = tr("The game exited with code %1.").arg(exitCode);
            const QString details = QString::fromUtf8(m_process.readAllStandardError()).trimmed().left(1024);
            if (!details.isEmpty())
                message += "\n" + details;
            setError(message);
        }
        emit runningChanged();
    });
    connect(&m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart) {
            setError(tr("Could not start the game: %1").arg(m_process.errorString()));
            emit runningChanged();
        }
    });
    refresh();
}

int GameScanner::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_games.size();
}

QVariant GameScanner::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_games.size())
        return {};
    if (role == TitleRole)
        return m_games.at(index.row()).title;
    if (role == GameIdRole)
        return m_games.at(index.row()).id;
    return {};
}

QHash<int, QByteArray> GameScanner::roleNames() const
{
    return {{TitleRole, "gameTitle"}, {GameIdRole, "gameId"}};
}

bool GameScanner::running() const
{
    return m_process.state() != QProcess::NotRunning;
}

QString GameScanner::errorMessage() const
{
    return m_errorMessage;
}

void GameScanner::setError(const QString &message)
{
    if (m_errorMessage == message)
        return;
    m_errorMessage = message;
    emit errorMessageChanged();
}

void GameScanner::refresh()
{
    if (running())
        return;

    QList<Game> games;
    QStringList errors;
    const QDir root(m_gamesDirectory);
    if (!root.exists()) {
        setError(tr("Games folder not found: %1").arg(m_gamesDirectory));
    } else {
        const auto folders = root.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        QSet<QString> gameIds;
        for (const QFileInfo &folder : folders) {
            QFile metadata(QDir(folder.absoluteFilePath()).filePath("meta.json"));
            if (!metadata.open(QIODevice::ReadOnly)) {
                errors.append(tr("Cannot read %1: %2").arg(metadata.fileName(), metadata.errorString()));
                continue;
            }
            QJsonParseError parseError;
            const QJsonDocument document = QJsonDocument::fromJson(metadata.readAll(), &parseError);
            const QJsonObject object = document.object();
            const QString id = object.value("id").toString();
            const QString title = object.value("title").toString().trimmed();
            const QString executable = object.value("executable").toString();
            if (parseError.error != QJsonParseError::NoError || !document.isObject() ||
                !validGameId(id) || title.isEmpty() || executable.isEmpty() ||
                executable == "." || executable == ".." ||
                executable.contains('/') || executable.contains('\\') || QDir::isAbsolutePath(executable)) {
                errors.append(tr("Invalid game metadata: %1").arg(metadata.fileName()));
                continue;
            }
            if (gameIds.contains(id)) {
                errors.append(tr("Duplicate game ID in metadata: %1").arg(id));
                continue;
            }
            const QString path = QDir(folder.absoluteFilePath()).filePath(executable);
            if (!QFileInfo(path).isFile()) {
                errors.append(tr("Game executable not found: %1").arg(path));
                continue;
            }
            games.append({id, title, path, folder.absoluteFilePath()});
            gameIds.insert(id);
        }
        setError(errors.join('\n'));
    }

    beginResetModel();
    m_games = std::move(games);
    endResetModel();
}

void GameScanner::launch(int index)
{
    if (running())
        return;
    if (index < 0 || index >= m_games.size()) {
        setError(tr("Select an available game."));
        return;
    }

    setError({});
    const Game &game = m_games.at(index);
    m_process.setWorkingDirectory(game.directory);
    m_process.start(game.executable, {});
}
