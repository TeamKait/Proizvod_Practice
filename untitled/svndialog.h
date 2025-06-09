#ifndef SVNDIALOG_H
#define SVNDIALOG_H

#include <QDialog>
#include "svnclient.h"

namespace Ui {
class SVNDialog;
}

class SVNDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SVNDialog(QWidget *parent = nullptr);
    ~SVNDialog();

    SVNClient* connectedClient() const { return m_client; }
    void loadCredentials();
    void saveCredentials();

signals:
    void repositoryConnected(SVNClient* client);

private slots:
    void handleConnect();
    void clearFields();

private:
    Ui::SVNDialog *ui;
    SVNClient* m_client = nullptr;
};

#endif // SVNDIALOG_H
