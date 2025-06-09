#include "svndialog.h"
#include "ui_svndialog.h"
#include <QMessageBox>
#include <QPushButton>
#include <qsettings.h>

// Загрузить данные о репозитории
void SVNDialog::loadCredentials()
{
    QSettings settings;
    ui->URLInput->setText(settings.value("svn/url").toString());
    ui->LoginInput->setText(settings.value("svn/username").toString());
    ui->PasswordInput->setText(settings.value("svn/password").toString());
}

// Сохранить данные о репозитории
void SVNDialog::saveCredentials()
{
    QSettings settings;
    settings.setValue("svn/url", ui->URLInput->text());
    settings.setValue("svn/username", ui->LoginInput->text());
    settings.setValue("svn/password", ui->PasswordInput->text());
}

SVNDialog::SVNDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SVNDialog)
{
    ui->setupUi(this);
    loadCredentials();

    ui->PasswordInput->setEchoMode(QLineEdit::Password);

    connect(ui->ConnectButton, &QPushButton::clicked, this, &SVNDialog::handleConnect);
    connect(ui->ClearButton, &QPushButton::clicked, this, &SVNDialog::clearFields);
}

SVNDialog::~SVNDialog()
{
    delete ui;
}

// Очистить поля ввода
void SVNDialog::clearFields(){
    ui->URLInput->setText("");
    ui->LoginInput->setText("");
    ui->PasswordInput->setText("");
}

// Подключиться к репозиторию
void SVNDialog::handleConnect()
{
    QString url = ui->URLInput->text().trimmed();
    QString login = ui->LoginInput->text().trimmed();
    QString password = ui->PasswordInput->text();

    if (url.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите URL репозитория");
        return;
    }

    // Удаляем предыдущий клиент, если был
    if (m_client) {
        delete m_client;
        m_client = nullptr;
    }

    m_client = new SVNClient(this);

    if (m_client->connectToRepository(url, login, password)) {
        QMessageBox::information(this, "Успех", "Подключение к репозиторию установлено");
//        ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
        saveCredentials();
        emit repositoryConnected(m_client);
        accept();
    } else {
        QMessageBox::critical(this, "Ошибка",
            "Не удалось подключиться к репозиторию.\n"
                                                       "Проверьте:\n"
                                                       "- Доступность репозитория\n"
                                                       "- Корректность URL\n"
                                                       "- Логин и пароль");

        delete m_client;
        m_client = nullptr;
    }
}
