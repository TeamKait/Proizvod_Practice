#ifndef RESULTSDIALOG_H
#define RESULTSDIALOG_H

#include <QDialog>
#include <QStringList>
#include <QListWidgetItem>

namespace Ui {
class ResultsDialog;
}

class ResultsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ResultsDialog(QWidget *parent = nullptr);
    ~ResultsDialog();

    void setResults(const QStringList &results);
    void setTortoiseInstalled(bool installed);

signals:
    void downloadRequested(const QString& url);
    void openFolderRequested(const QString& path);

private slots:
    void downloadFile();
    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);
    void on_openFolderButton_clicked();
    void handleOpenFolderRequested(const QString& url);
    void openWithTortoiseSVN(const QString& url);

private:
    Ui::ResultsDialog *ui;
    QStringList m_results;
    bool m_tortoiseInstalled = false;

    // Новый метод для обработки выбранного элемента
    void processSelectedItem(bool openFolder);
};

#endif // RESULTSDIALOG_H
