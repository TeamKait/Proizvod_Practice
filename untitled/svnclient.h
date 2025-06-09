#ifndef SVNCLIENT_H
#define SVNCLIENT_H

#include <QObject>
#include <QProcess>
#include <QByteArray>
#include <QStringList>
#include <QDebug>
#include <QFileInfo>

class SVNClient : public QObject {
    Q_OBJECT
public:
    explicit SVNClient(QObject* parent = nullptr);

    bool connectToRepository(const QString& url,
                             const QString& username,
                             const QString& password);

    bool isConnected() const { return m_isConnected; }
    QStringList searchByFileName(const QString& path, const QString& fileName);
    QStringList searchByMd5(const QString& path, const QString& md5);

    // Новый метод для экспорта файла
    bool exportFile(const QString& url, const QString& localFilePath);

private:
    QString m_url;
    QString m_username;
    QString m_password;
    bool m_isConnected = false;

    bool runSvnCommand(const QStringList& args, QByteArray& output, QString& error);
};

#endif // SVNCLIENT_H
