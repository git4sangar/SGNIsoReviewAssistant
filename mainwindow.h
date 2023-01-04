#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>


#include "statusupdater.h"
#include "dbinterface.h"
#include "projectpage.h"
#include "schedulepage.h"
#include "reviewpage.h"

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
    void on_lnEdtLL6_textChanged(const QString &arg1);
    void on_tbWdgtMain_currentChanged(int index);

    void on_btnProjSubmit_clicked();
    void on_btnProjNew_clicked();
    void on_btnProjGetDetails_clicked();
    void on_tblWdgtProjects_cellClicked(int row, int column);
    void on_tblWdgtProjects_vHeaderClicked(int index);
    void on_btnProjDelete_clicked();
    void on_btnProjClear_clicked();

    void on_btnSchdlNew_clicked();
    void on_btnSchdlUpdate_clicked();
    void on_btnSchdlDelete_clicked();
    void on_btnSchdlRemind_clicked();
    void on_btnSchdlArchive_clicked();
    void on_tblWdgtSchdlLookup_cellClicked(int row, int column);
    void on_tblWdgtSchedules_cellClicked(int row, int column);
    void on_tblWdgtSchedules_vHeaderClicked(int index);

    void on_btnRevwAdd_clicked();
    void on_btnRevwUpdate_clicked();
    void on_btnRevwDelete_clicked();
    void on_cmBxRevwNames_currentIndexChanged(int index);
    void on_cmBxRevwCmntStat_currentIndexChanged(int index);
    void on_tblWdgtCmnts_cellClicked(int row, int column);
    void on_tblWdgtCmnts_vHeaderClicked(int index);

private:
    Ui::MainWindow          *ui;
    DBInterface::Ptr        mpDB;
    ProjectPage::Ptr        mpProjPage;
    SchedulePage::Ptr       mpSchdlPage;
    ReviewPage::Ptr         mpRevwPage;
    StatusUpdater           *mpStatusUpdater;


    ProjUiElements          *prepareProjUIElements();
    SchdlUiElements         *prepareSchdlUIElements();
    ReviewUiElements        *prepareRevwUIElements();
};
#endif // MAINWINDOW_H
