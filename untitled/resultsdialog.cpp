#include "resultsdialog.h"
#include "qurl.h"
#include "ui_resultsdialog.h"
#include "mainwindow.h"
#include <QMessageBox>
#include <QFileInfo>
#include <qdir.h>
#include <QDesktopServices>
#include <QProcess>

ResultsDialog::ResultsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ResultsDialog)
{
    ui->setupUi(this);
    setWindowTitle("Результаты поиска");
    ui->listWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(ui->downloadButton, &QPushButton::clicked, this, &ResultsDialog::downloadFile);
}

ResultsDialog::~ResultsDialog()
{
    delete ui;
}

void ResultsDialog::setResults(const QStringList &results)
{
    m_results = results;
    ui->listWidget->clear();

    // Обработка пустых результатов
    if (results.isEmpty()) {
        QListWidgetItem* item = new QListWidgetItem("Файлы не найдены");
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        ui->listWidget->addItem(item);
        ui->downloadButton->setEnabled(false);
        ui->openFolderButton->setEnabled(false);
    } else {
        // Формирование отображаемых имен файлов
        QStringList displayNames;
        for (const QString& path : results) {
            if (path.startsWith("file://")) {
                displayNames << QFileInfo(QUrl(path).toLocalFile()).fileName();
            } else {
                displayNames << QFileInfo(path).fileName();
            }
        }
        ui->listWidget->addItems(displayNames);
        ui->downloadButton->setEnabled(true);
        ui->openFolderButton->setEnabled(m_tortoiseInstalled);
    }
}

void ResultsDialog::setTortoiseInstalled(bool installed)
{
    m_tortoiseInstalled = installed;
    // Активация кнопки при наличии результатов
    if (!m_results.isEmpty()) {
        ui->openFolderButton->setEnabled(m_tortoiseInstalled);
    }
}

void ResultsDialog::downloadFile()
{
    QListWidgetItem *item = ui->listWidget->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Ошибка", "Выберите файл для скачивания");
        return;
    }

    QString url = m_results[ui->listWidget->row(item)];
    QString tempDir = QDir::tempPath() + "/SVNExplorer/";
    QDir().mkpath(tempDir);

    // Формирование имени файла
    QString fileName = QUrl(url).fileName();
    if (fileName.isEmpty()) fileName = "downloaded_file";
    QString savePath = tempDir + fileName;

    // Скачивание файла
    if (MainWindow::m_svnClient->exportFile(url, savePath)) {
        // Открытие папки с файлом после скачивания
        QDesktopServices::openUrl(QUrl::fromLocalFile(tempDir));
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось скачать файл");
    }
}

// Обработка двойного клика
void ResultsDialog::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    if (item->flags() & Qt::ItemIsEnabled) {
        processSelectedItem(true);
    }
}

// Обработка нажатия кнопки открытия папки
void ResultsDialog::on_openFolderButton_clicked()
{
    processSelectedItem(true);
}

// Универсальная обработка выбранного элемента
void ResultsDialog::processSelectedItem(bool openFolder)
{
    QListWidgetItem *item = ui->listWidget->currentItem();
    if (!item) {
        QMessageBox::warning(this, "Ошибка", "Выберите файл");
        return;
    }

    QString path = m_results[ui->listWidget->row(item)];
    QString folderPath;

    // Определение типа пути (локальный/удаленный)
    if (path.startsWith("file://")) {
        QUrl fileUrl(path);
        folderPath = QFileInfo(fileUrl.toLocalFile()).absolutePath();
    } else {
        QUrl fullUrl(path);
        if (!fullUrl.isValid()) {
            QMessageBox::warning(this, "Ошибка", "Некорректный URL: " + path);
            return;
        }

        // Извлечение пути к папке из URL
        QString urlPath = fullUrl.path();
        int lastSlash = urlPath.lastIndexOf('/');
        if (lastSlash <= 0) {
            QMessageBox::warning(this, "Ошибка", "Невозможно определить путь к папке");
            return;
        }
        folderPath = urlPath.left(lastSlash);
    }

    if (openFolder) {
        handleOpenFolderRequested(folderPath);
    }
}

// Обработка открытия папки
void ResultsDialog::handleOpenFolderRequested(const QString& path)
{
    // Для локальных путей
    if (path.startsWith('/') || path.contains(':') || QDir(path).exists()) {
#ifdef Q_OS_WIN
        QString command = "explorer \"" + QDir::toNativeSeparators(path) + "\"";
        QProcess::startDetached(command);
#else
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
#endif
    }
    // Для удаленных URL
    else {
        openWithTortoiseSVN(path);
    }
}

// Открытие через TortoiseSVN (Windows)
void ResultsDialog::openWithTortoiseSVN(const QString& url)
{
#ifdef Q_OS_WIN
    if (!m_tortoiseInstalled) {
        QMessageBox::warning(this, "Ошибка", "TortoiseSVN не установлен");
        return;
    }

    // Поиск TortoiseProc.exe
    QString tortoiseProc;
    QStringList searchPaths = {
        "C:\\Program Files\\TortoiseSVN\\bin\\TortoiseProc.exe",
        "C:\\Program Files (x86)\\TortoiseSVN\\bin\\TortoiseProc.exe"
    };

    for (const QString& path : searchPaths) {
        if (QFileInfo::exists(path)) {
            tortoiseProc = path;
            break;
        }
    }

    if (tortoiseProc.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "TortoiseSVN не найден");
        return;
    }

    // Формирование команды
    QString command = QString("\"%1\" /command:repobrowser /path:\"%2\"")
                          .arg(tortoiseProc)
                          .arg(url);
    QProcess::startDetached(command);
#else
    Q_UNUSED(url)
    QMessageBox::warning(this, "Ошибка", "TortoiseSVN поддерживается только в Windows");
#endif
}
