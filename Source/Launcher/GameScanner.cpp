#include "GameScanner.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>

namespace {
constexpr int TitleRole = Qt::UserRole + 1;
}

GameScanner::GameScanner(const QString &gamesDirectory, QObject *parent)
    : QAbstractListModel(parent), m_gamesDirectory(gamesDirectory)
{
    connect(&m_process, &QProcess::started, this, &GameScanner::runningChanged);
    connect(&m_process, &QProcess::finished, this, [this](int exitCode, QProcess::ExitStatus status) {
        if (status == QProcess::CrashExit)
            setError(tr("The game stopped unexpectedly."));
        else if (exitCode != 0)
            setError(tr("The game exited with code %1.").arg(exitCode));
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
    return {};
}

QHash<int, QByteArray> GameScanner::roleNames() const
{
    return {{TitleRole, "gameTitle"}};
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
        for (const QFileInfo &folder : folders) {
            QFile metadata(QDir(folder.absoluteFilePath()).filePath("meta.json"));
            if (!metadata.open(QIODevice::ReadOnly)) {
                errors.append(tr("Cannot read %1: %2").arg(metadata.fileName(), metadata.errorString()));
                continue;
            }
            QJsonParseError parseError;
            const QJsonDocument document = QJsonDocument::fromJson(metadata.readAll(), &parseError);
            const QJsonObject object = document.object();
            const QString title = object.value("title").toString().trimmed();
            const QString executable = object.value("executable").toString();
            if (parseError.error != QJsonParseError::NoError || !document.isObject() ||
                title.isEmpty() || executable.isEmpty() || executable == "." || executable == ".." ||
                executable.contains('/') || executable.contains('\\') || QDir::isAbsolutePath(executable)) {
                errors.append(tr("Invalid game metadata: %1").arg(metadata.fileName()));
                continue;
            }
            const QString path = QDir(folder.absoluteFilePath()).filePath(executable);
            if (!QFileInfo(path).isFile()) {
                errors.append(tr("Game executable not found: %1").arg(path));
                continue;
            }
            games.append({title, path, folder.absoluteFilePath()});
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
