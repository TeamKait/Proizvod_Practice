#include "SVNClient.h"

#include <QDir>
#include <QFileInfo>

SVNClient::SVNClient(QObject* parent) : QObject(parent) {}

bool SVNClient::connectToRepository(const QString& url,
                                    const QString& username,
                                    const QString& password) {
    // Сбрасываем предыдущее состояние
    m_url.clear();
    m_username.clear();
    m_password.clear();
    m_isConnected = false;

    // Сохраняем новые учетные данные
    m_url = url;
    m_username = username;
    m_password = password;

    // Формируем команду для проверки подключения
    QStringList args = {"list", url, "--non-interactive"};

    // Добавляем учетные данные, если они предоставлены
    if (!username.isEmpty()) {
        args << "--username" << username << "--password" << password;
    }

    // Выполняем команду проверки подключения
    QByteArray output;
    QString error;
    bool success = runSvnCommand(args, output, error);

    // Анализируем результат
    if (success) {
        m_isConnected = true;
        qDebug() << "Успешное подключение к репозиторию:" << url;
    } else {
        // Сбрасываем учетные данные при неудаче
        m_url.clear();
        m_username.clear();
        m_password.clear();

        // Анализ ошибок
        if (error.contains("E170001") ||
            error.contains("authorization failed") ||
            error.contains("E215004")) {
            qWarning() << "Ошибка аутентификации для пользователя:" << username;
        } else if (error.contains("E170000") ||
                   error.contains("Unable to connect") ||
                   error.contains("E175002") ||
                   error.contains("E180001")) {
            qWarning() << "Ошибка подключения к репозиторию:" << url;
        } else {
            qWarning() << "Неизвестная ошибка подключения:" << error;
        }
    }

    return m_isConnected;
}

bool SVNClient::runSvnCommand(const QStringList& args,
                              QByteArray& output,
                              QString& error) {
    QProcess svnProcess;
    svnProcess.setProcessChannelMode(QProcess::MergedChannels);
    svnProcess.start("svn", args);

    // Ожидание запуска процесса
    if (!svnProcess.waitForStarted(3000)) {
        error = "Не удалось запустить процесс SVN. Убедитесь, что SVN установлен и доступен в PATH.";
        return false;
    }

    // Ожидание завершения процесса
    if (!svnProcess.waitForFinished(15000)) {
        svnProcess.kill();
        error = "Таймаут выполнения команды SVN (15 секунд)";
        return false;
    }

    // Чтение вывода
    output = svnProcess.readAllStandardOutput();
    error = svnProcess.readAllStandardError();

    // Проверка статуса завершения
    if (svnProcess.exitStatus() != QProcess::NormalExit) {
        error = "Аварийное завершение SVN процесса";
        return false;
    }

    return (svnProcess.exitCode() == 0);
}

QStringList SVNClient::searchByFileName(const QString& path, const QString& fileName) {
    QStringList results;

    if (!m_isConnected) {
        qWarning() << "Попытка поиска без подключения к репозиторию";
        return results;
    }

    QString fullPath = m_url;
    if (!path.isEmpty()) {
        // Очищаем путь от лишних слэшей
        if (path.startsWith('/')) fullPath += path;
        else fullPath += "/" + path;
    }

    // Формируем команду для рекурсивного списка файлов
    QStringList args = {"list", "-R", fullPath, "--non-interactive"};
    if (!m_username.isEmpty()) {
        args << "--username" << m_username << "--password" << m_password;
    }

    QByteArray output;
    QString error;
    if (runSvnCommand(args, output, error)) {
        QStringList allFiles = QString::fromUtf8(output).split('\n', QString::SkipEmptyParts);

        for (const QString& file : allFiles) {
            QString trimmedFile = file.trimmed();
            QFileInfo fileInfo(trimmedFile);

            // Проверяем совпадение по имени файла
            if (fileInfo.fileName().contains(fileName, Qt::CaseInsensitive)) {
                // Формируем полный URL
                QString fullUrl = m_url;
                if (!path.isEmpty()) fullUrl += "/" + path;
                fullUrl += "/" + trimmedFile;
                results << fullUrl;
            }
        }
    } else {
        qWarning() << "Ошибка поиска по имени файла:" << error;
    }

    return results;
}

QStringList SVNClient::searchByMd5(const QString& path, const QString& md5) {
    Q_UNUSED(path);
    Q_UNUSED(md5);
    qWarning() << "Поиск по MD5 не реализован";
    return QStringList();
}

bool SVNClient::exportFile(const QString& url, const QString& localFilePath)
{
    if (!m_isConnected) {
        qWarning() << "Попытка экспорта без подключения";
        return false;
    }

    // Создаем папку для файла, если нужно
    QFileInfo fileInfo(localFilePath);
    QDir().mkpath(fileInfo.absolutePath());

    QStringList args = {"export", url, localFilePath, "--non-interactive"};
    if (!m_username.isEmpty()) {
        args << "--username" << m_username << "--password" << m_password;
    }

    QByteArray output;
    QString error;
    bool success = runSvnCommand(args, output, error);

    if (success) {
        qDebug() << "Файл успешно экспортирован в" << localFilePath;
        return true;
    } else {
        qWarning() << "Ошибка экспорта:" << error;
        return false;
    }
}
