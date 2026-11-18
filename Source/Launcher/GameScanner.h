#pragma once

#include <QAbstractListModel>
#include <QProcess>
#include <QStringList>

class GameScanner : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

public:
    explicit GameScanner(const QString &gamesDirectory, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    bool running() const;
    QString errorMessage() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void launch(int index);

signals:
    void runningChanged();
    void errorMessageChanged();

private:
    struct Game {
        QString id;
        QString title;
        QString executable;
        QString directory;
    };

    void setError(const QString &message);

    QString m_gamesDirectory;
    QList<Game> m_games;
    QProcess m_process;
    QString m_errorMessage;
};
