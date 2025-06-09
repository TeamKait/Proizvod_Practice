#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "svnclient.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class SVNDialog;
class ResultsDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    static SVNClient* m_svnClient;
private slots:
    void searchFile();
    void openSVNDialog();

private:
    Ui::MainWindow *ui;
    bool m_tortoiseInstalled = false;
    
    void toggleUI();
    bool checkTortoiseSVN();
    void openLocalFolder(const QString& path);
};

#endif // MAINWINDOW_H
