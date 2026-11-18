#pragma once

#include <QObject>
#include <QFileSystemWatcher>
#include <QStringList>
#include <QTimer>
#include <QVariantMap>

class ProfileStats : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString formattedPlaytime READ formattedPlaytime NOTIFY statsChanged)
    Q_PROPERTY(QVariantMap gamePlaytimes READ gamePlaytimes NOTIFY statsChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

public:
    explicit ProfileStats(const QString &profilePath, QObject *parent = nullptr);

    QString formattedPlaytime() const;
    QVariantMap gamePlaytimes() const;
    QString errorMessage() const;

    Q_INVOKABLE void refresh();

signals:
    void statsChanged();
    void errorMessageChanged();

private:
    void scheduleRefresh();
    void refreshWatchPaths();
    void setError(const QString &message);
    QString formatPlaytime(quint64 seconds) const;

    QString m_profilePath;
    QString m_profileDirectory;
    QString m_formattedPlaytime = QStringLiteral("0m");
    QVariantMap m_gamePlaytimes;
    QString m_errorMessage;
    QFileSystemWatcher m_watcher;
    QTimer m_refreshTimer;
};
