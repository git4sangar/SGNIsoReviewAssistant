//sgn
#include <QDir>
#include <QStringList>
#include <QDebug>
#include <QHeaderView>

#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "projectpage.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QString curUserCdsid= qgetenv("USERNAME");
    ui->lblUser->setText(curUserCdsid);

    QString strImgDir   = QDir::currentPath() + QString("\\..\\Images\\");
    QString pLogoFile   = strImgDir + "ford_logo_320x180.png";
    QPixmap pixMap(pLogoFile);
    ui->lblLogo->setPixmap(pixMap.scaled(320,180));
    ui->lblLogo->setAlignment(Qt::AlignmentFlag::AlignCenter);

    setWindowIcon(QIcon(strImgDir + "iso_logo.png"));

    mpDB            = std::make_shared<DBInterface>(curUserCdsid, ui->lnEdtLL6->text());
    mpStatusUpdater = new StatusUpdater(ui->lblStatus);
    mpProjPage      = std::make_shared<ProjectPage>(prepareProjUIElements(), mpDB);
    mpDB->subscribeToDBNotification(mpProjPage);
    mpSchdlPage     = std::make_shared<SchedulePage>(prepareSchdlUIElements(), mpDB);
    mpDB->subscribeToDBNotification(mpSchdlPage);
    mpDB->triggerDBPull();

    const auto tblVHeader = ui->tblWdgtProjects->verticalHeader();
    connect(tblVHeader, &QHeaderView::sectionClicked, this, &MainWindow::on_tblWdgtProjects_vHeaderClicked);
}

MainWindow::~MainWindow() {
    delete ui;
}

//--------------------------------------------------------------------------------------------------------
//                          Project and Team Page {{{
//--------------------------------------------------------------------------------------------------------

ProjUiElements* MainWindow::prepareProjUIElements() {
    ProjUiElements *pUi     = new ProjUiElements();
    pUi->lnEdtProjName      = ui->lnEdtProjName;
    pUi->lnEdtProjId        = ui->lnEdtProjId;
    pUi->lnEdtProjPM        = ui->lnEdtProjPM;
    pUi->lnEdtProjLL6       = ui->lnEdtProjLL6;
    pUi->txtEdtCdsids       = ui->txtEdtCdsids;
    //pUi->lnEdtProjSpoc    = ui->lnEdtProjSpoc;
    pUi->btnProjNew         = ui->btnProjNew;
    pUi->btnProjGetDetails  = ui->btnProjGetDetails;
    pUi->btnProjSubmit      = ui->btnProjSubmit;
    pUi->tblWdgtProjects    = ui->tblWdgtProjects;
    pUi->statusUpdater      = mpStatusUpdater;
    return pUi;
}

void MainWindow::on_btnProjSubmit_clicked() { mpProjPage->onBtnProjSubmitClicked(); }
void MainWindow::on_btnProjNew_clicked() { mpProjPage->onBtnProjNewClicked(); }
void MainWindow::on_btnProjGetDetails_clicked() { mpProjPage->onBtnProjGetDetailsClicked(); }
void MainWindow::on_tblWdgtProjects_cellClicked(int row, int column) { mpProjPage->onTblWdgtProjectsClicked(row, column); }
void MainWindow::on_tblWdgtProjects_vHeaderClicked(int index) { mpProjPage->onTblWdgtVHeaderClicked(index); }
void MainWindow::on_btnProjDelete_clicked() { mpProjPage->onBtnProjDeleteClicked(); }

//--------------------------------------------------------------------------------------------------------
//                          Project and Team Page }}}
//--------------------------------------------------------------------------------------------------------





//--------------------------------------------------------------------------------------------------------
//                          Schedule Page {{{
//--------------------------------------------------------------------------------------------------------

SchdlUiElements* MainWindow::prepareSchdlUIElements() {
    SchdlUiElements *pUi    = new SchdlUiElements();
    pUi->lnEdtSchdlRevwName = ui->lnEdtSchdlRevwName;
    pUi->lnEdtSchdlProjId   = ui->lnEdtSchdlProjId;
    pUi->lnEdtSchdlReviewer = ui->lnEdtSchdlReviewer;
    pUi->lnEdtSchdlAuditee  = ui->lnEdtSchdlAuditee;
    pUi->dtEdtSchdlStartDate= ui->dtEdtSchdlStartDate;
    pUi->dtEdtSchdlEndDate  = ui->dtEdtSchdlEndDate;
    pUi->btnSchdlNew        = ui->btnSchdlNew;
    pUi->btnSchdlUpdate     = ui->btnSchdlUpdate;
    pUi->btnSchdlDelete     = ui->btnSchdlDelete;
    pUi->btnSchdlRemind     = ui->btnSchdlRemind;
    pUi->tblWdgtSchedules   = ui->tblWdgtSchedules;
    pUi->tblWdgtSchdlLookup = ui->tblWdgtSchdlLookup;
    pUi->statusUpdater      = mpStatusUpdater;
    return pUi;
}

void MainWindow::on_btnSchdlNew_clicked() { mpSchdlPage->onBtnSchdlNewClicked(); }
void MainWindow::on_btnSchdlUpdate_clicked() { mpSchdlPage->onBtnSchdlUpdateClicked(); }
void MainWindow::on_btnSchdlDelete_clicked() { mpSchdlPage->onBtnSchdlDeleteClicked(); }
void MainWindow::on_btnSchdlRemind_clicked() { mpSchdlPage->onBtnSchdlRemindClicked(); }
void MainWindow::on_tblWdgtSchdlLookup_cellClicked(int row, int column) { mpSchdlPage->onTblWdgtSchdlLookupClicked(row, column); }
void MainWindow::on_tblWdgtSchedules_cellClicked(int row, int column) { mpSchdlPage->onTblWdgtSchedulesClicked(row, column); }
//--------------------------------------------------------------------------------------------------------
//                          Schedule Page }}}
//--------------------------------------------------------------------------------------------------------




