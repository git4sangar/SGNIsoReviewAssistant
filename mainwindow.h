#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>


#include "statusupdater.h"
#include "dbinterface.h"
#include "projectpage.h"
#include "schedulepage.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnProjSubmit_clicked();
    void on_btnProjNew_clicked();
    void on_btnProjGetDetails_clicked();
    void on_tblWdgtProjects_cellClicked(int row, int column);
    void on_tblWdgtProjects_vHeaderClicked(int index);
    void on_btnProjDelete_clicked();

    void on_btnSchdlNew_clicked();
    void on_btnSchdlUpdate_clicked();
    void on_btnSchdlDelete_clicked();
    void on_btnSchdlRemind_clicked();
    void on_tblWdgtSchdlLookup_cellClicked(int row, int column);
    void on_tblWdgtSchedules_cellClicked(int row, int column);

private:
    Ui::MainWindow          *ui;
    DBInterface::Ptr        mpDB;
    ProjectPage::Ptr        mpProjPage;
    SchedulePage::Ptr       mpSchdlPage;
    StatusUpdater           *mpStatusUpdater;


    ProjUiElements          *prepareProjUIElements();
    SchdlUiElements         *prepareSchdlUIElements();
};
#endif // MAINWINDOW_H
