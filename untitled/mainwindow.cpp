#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "svnclient.h"
#include "svndialog.h"
#include "resultsdialog.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QProcess>
#include <QDesktopServices>
#include <QDir>
#include <QUrl>
#include <QSettings>

SVNClient* MainWindow::m_svnClient = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Инициализация SVN клиента
    m_svnClient = new SVNClient(this);

    // Загрузка сохраненных данных
    QSettings settings;
    QString savedUrl = settings.value("svn/url").toString();
    QString savedUsername = settings.value("svn/username").toString();
    QString savedPassword = settings.value("svn/password").toString();

    // Автоматическое подключение при наличии данных
    if (!savedUrl.isEmpty()) {
        if (m_svnClient->connectToRepository(savedUrl, savedUsername, savedPassword)) {
            qDebug() << "Автоматическое подключение к репозиторию успешно";
        } else {
            qWarning() << "Не удалось автоматически подключиться к репозиторию";
        }
    }

    // Включить/Выключить кнопки в зависимости от подключения
    toggleUI();
    connect(ui->ConnectSVNButton, &QPushButton::clicked, this, &MainWindow::openSVNDialog);
    connect(ui->searchButton, &QPushButton::clicked, this, &MainWindow::searchFile);

    // Проверка наличия TortoiseSVN
    m_tortoiseInstalled = checkTortoiseSVN();
}

MainWindow::~MainWindow()
{
    delete ui;
    if (m_svnClient) delete m_svnClient;

    // Очистка временной папки
    QDir tempDir(QDir::tempPath() + "/SVNExplorer/");
    if (tempDir.exists()) {
        tempDir.removeRecursively();
    }
}

// Проверить наличие TortoiseSVN
bool MainWindow::checkTortoiseSVN()
{
#ifdef Q_OS_WIN
    // Проверка возможных путей установки TortoiseSVN
    QStringList possiblePaths = {
        "C:\\Program Files\\TortoiseSVN\\bin\\TortoiseProc.exe",
        "C:\\Program Files (x86)\\TortoiseSVN\\bin\\TortoiseProc.exe"
    };

    for (const QString& path : possiblePaths) {
        if (QFileInfo::exists(path)) {
            return true;
        }
    }
#endif
    return false;
}

// Включить/Выключить кнопки в зависимости от подключения
void MainWindow::toggleUI()
{
    bool isConnected = m_svnClient && m_svnClient->isConnected();

    ui->searchButton->setEnabled(isConnected);
    ui->FileNameInput->setEnabled(isConnected);

    ui->FileNameInput->setPlaceholderText(isConnected ? "Файл" : "SVN не подключён");
    ui->MainLabel->setText(QString::asprintf("%s SVN Поисковик", isConnected ? "🟢" : "🔴"));
}

// Диалог подключения к SVN репозиторию
void MainWindow::openSVNDialog()
{
    SVNDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        if (dlg.connectedClient()) {
            if (m_svnClient) {
                delete m_svnClient;
            }
            m_svnClient = dlg.connectedClient();
            m_svnClient->setParent(this);
            toggleUI();
        }
    }
}

// Поиск файла по репозиторию
void MainWindow::searchFile()
{
    if (!m_svnClient || !m_svnClient->isConnected()) {
        QMessageBox::warning(this, "Ошибка", "Сначала подключитесь к SVN репозиторию");
        return;
    }

    QString fileName = ui->FileNameInput->text().trimmed();

    QString searchPattern = fileName;

    if (searchPattern.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите имя файла или расширение");
        return;
    }

    QStringList results = m_svnClient->searchByFileName("", searchPattern);

    ResultsDialog resultsDialog(this);
    resultsDialog.setResults(results);

    // Передаеча информации о наличии TortoiseSVN
    resultsDialog.setTortoiseInstalled(m_tortoiseInstalled);

    resultsDialog.exec();
}
