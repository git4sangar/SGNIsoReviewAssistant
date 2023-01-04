//sgn

#include <iostream>
#include <sstream>
#include <QString>
#include <QTextStream>
#include <QProcess>
#include <QDate>
#include <QDir>
#include <QFile>
#include <QTableWidgetItem>

#include "schedulepage.h"

SchedulePage::SchedulePage(SchdlUiElements *pUi, DBInterface::Ptr pDB)
    : QObject(nullptr)
    , mCurRowIndex(-1)
    , mbEyeOnDBUpdate(false)
    , ui(pUi)
    , mpDB(pDB)
    , mpHttpMgr(new QNetworkAccessManager()) {
    //  Validator needs ProjectPage to be inherited from QObject
    ui->lnEdtSchdlProjId->setValidator(new QIntValidator(0, 9999999, this));
}

//--------------------------------------------------------------------------------------------------------
//                                      Handle Ui {{{
//--------------------------------------------------------------------------------------------------------

void SchedulePage::populateDORFLookupTable() {
    const QVector<Project::Ptr>& pAllProjs  = mpDB->getAllProjects();
    QTableWidgetItem* pItem                 = nullptr;
    Project::Ptr pProj                      = nullptr;

    ui->tblWdgtSchdlLookup->setRowCount(pAllProjs.size());
    ui->tblWdgtSchdlLookup->setColumnCount(2);
    ui->tblWdgtSchdlLookup->setHorizontalHeaderLabels(QStringList() << "DORF id" << "Proj Name");

    for(int32_t iRow = 0; iRow < pAllProjs.size(); iRow++) {
        pProj = pAllProjs[iRow];
        pItem = new QTableWidgetItem(); pItem->setData(Qt::DisplayRole, QVariant(pProj->mProjId));
        ui->tblWdgtSchdlLookup->setItem(iRow, 0, pItem);
        pItem = new QTableWidgetItem(); pItem->setText(pProj->mName);
        ui->tblWdgtSchdlLookup->setItem(iRow, 1, pItem);
    }
}

void SchedulePage::populateSchdlTable() {
    clearSchdlTable();
    const auto& pSchdls = mpDB->getAllSchedules();
    if(pSchdls.size() == 0) return;

    QStringList colHeaders  = Schedule::getColNames();
    ui->tblWdgtSchedules->setRowCount(pSchdls.size());
    ui->tblWdgtSchedules->setColumnCount(colHeaders.size());
    ui->tblWdgtSchedules->setHorizontalHeaderLabels(colHeaders);

    for(int32_t iRow = 0; iRow < pSchdls.size(); iRow++) {
        Schedule::Ptr pSchdl            = pSchdls[iRow];
        QVector<QTableWidgetItem*> pCols= pSchdl->getFieldsAsWidgetItems();
        for(int32_t iCol = 0; iCol < pCols.size(); iCol++)
            ui->tblWdgtSchedules->setItem(iRow, iCol, pCols[iCol]);
    }
}

void SchedulePage::onBtnSchdlNewClicked() {
    if(ui->tblWdgtSchdlLookup->rowCount() == 0) populateDORFLookupTable();
    clearScheduleDetails();
    ui->btnSchdlUpdate->setText("Submit");
    ui->lnEdtSchdlRevwName->setText(QDate().currentDate().toString("yyyy-MM")+"_ReviewName");
    ui->statusUpdater->updateStatus("Pls Enter all schedule details & click submit");
}

void SchedulePage::onBtnSchdlUpdateClicked() {
         if(ui->btnSchdlUpdate->text() == "Submit") insertNewSchedule();
    else if(ui->btnSchdlUpdate->text() == "Update") updateSchedule();
}

void SchedulePage::insertNewSchedule() {
    Schedule::Ptr pSchdl    = sanityCheck();
    if(!pSchdl) return;

    QNetworkRequest request = mpDB->makeUpdateRequest();
    connect(mpHttpMgr, &QNetworkAccessManager::finished, this, &SchedulePage::respInsertUpdateSchdl);
    mpHttpMgr->put(request, pSchdl->getInsertQuery().toUtf8());
}

void SchedulePage::updateSchedule() {
    if(mCurRowIndex == -1) {ui->statusUpdater->updateStatus("Pls choose a schedule"); return;}
    Schedule::Ptr pToChange = sanityCheck();
    if(!pToChange) return;
    Schedule::Ptr pExisting = mpDB->getAllSchedules()[mCurRowIndex];

    QNetworkRequest request = mpDB->makeUpdateRequest();
    connect(mpHttpMgr, &QNetworkAccessManager::finished, this, &SchedulePage::respInsertUpdateSchdl);
    mpHttpMgr->put(request, pExisting->getUpdateQuery(pToChange).toUtf8());
}

void SchedulePage::onBtnSchdlArchiveClicked() {
    if(mCurRowIndex == -1) {
        ui->statusUpdater->updateStatus("Pls choose a Schedule to archive");
        return;
    }

    QString curUser     = mpDB->getCurUserCdsid();
    Schedule::Ptr pSchdl= mpDB->getAllSchedules()[mCurRowIndex];
    if(pSchdl->mScheduler != curUser || pSchdl->mStatus != Schedule::stringStatusToInt("Closed")) {
        ui->statusUpdater->updateStatus(pSchdl->mScheduler + " alone can archive this that too after CLOSED");
        return;
    }

    std::string delimiter = "$";
    std::stringstream ss;
    ss  << "SELECT * FROM schedule WHERE review_id = " << pSchdl->mReviewId << ";"
        << delimiter
        << "UPDATE schedule SET is_archived = 1 WHERE review_id = " << pSchdl->mReviewId << ";";
    QNetworkRequest request = mpDB->makeSelectAndUpdateReq(true);
    connect(mpHttpMgr, &QNetworkAccessManager::finished, this, &SchedulePage::respInsertUpdateSchdl);
    mpHttpMgr->put(request, QString(ss.str().c_str()).toUtf8());
}

void SchedulePage::onBtnSchdlDeleteClicked() {
    if(mCurRowIndex == -1) {
        ui->statusUpdater->updateStatus("Pls choose a Schedule to delete");
        return;
    }

    QString curUser     = mpDB->getCurUserCdsid();
    Schedule::Ptr pSchdl= mpDB->getAllSchedules()[mCurRowIndex];
    if(pSchdl->mScheduler != curUser) {
        ui->statusUpdater->updateStatus("Only Scheduler can delete");
        return;
    }

    std::string delimiter = "$";
    std::stringstream ss;
    ss  << "SELECT * FROM schedule WHERE review_id = " << pSchdl->mReviewId << ";"
        << delimiter
        << "DELETE FROM schedule WHERE review_id = " << pSchdl->mReviewId << ";";
    QNetworkRequest request = mpDB->makeSelectAndUpdateReq(true);
    connect(mpHttpMgr, &QNetworkAccessManager::finished, this, &SchedulePage::respInsertUpdateSchdl);
    mpHttpMgr->put(request, QString(ss.str().c_str()).toUtf8());
}

void SchedulePage::onTblWdgtSchdlLookupClicked(int row, int column) {
    QTableWidgetItem *pItem = ui->tblWdgtSchdlLookup->item(row, 0);
    QString strText         = pItem->text();
    ui->lnEdtSchdlProjId->setText(strText);
}

void SchedulePage::onTblWdgtSchedulesClicked(int row, int column) {
    mCurRowIndex            = row;
    QTableWidgetItem *pItem = ui->tblWdgtSchedules->item(row, 0);
    QString strText         = pItem->text();
    int32_t iReviewId       = strText.toInt();
    const auto& pSchdls     = mpDB->getAllSchedules();

    Schedule::Ptr pSchdl;
    for(int32_t iLoop = 0; iLoop < pSchdls.size(); iLoop++) {
        if(pSchdls[iLoop]->mReviewId == iReviewId) pSchdl = pSchdls[iLoop];
    }

    if(pSchdl) {
        ui->lnEdtSchdlRevwName->setText(pSchdl->mReviewName);
        ui->lnEdtSchdlProjId->setText(QString::number(pSchdl->mProjId));
        ui->lnEdtSchdlReviewer->setText(pSchdl->mReviewer);
        ui->lnEdtSchdlAuditee->setText(pSchdl->mAuditee);
        ui->btnSchdlUpdate->setText("Update");
    }
}

Schedule::Ptr SchedulePage::sanityCheck() {
    Schedule::Ptr pSchdl;

    QString strProjId   = ui->lnEdtSchdlProjId->text();
    QString strRvwName  = ui->lnEdtSchdlRevwName->text();
    QString strReviewer = ui->lnEdtSchdlReviewer->text().toUpper();
    QString strAuditee  = ui->lnEdtSchdlAuditee->text().toUpper();
    QDate dtStartDate   = ui->dtEdtSchdlStartDate->date();
    QDate dtEndDate     = ui->dtEdtSchdlEndDate->date();

    if(!mpDB->isValidCdsid(strReviewer))    strReviewer.clear();
    if(!mpDB->isValidCdsid(strAuditee))     strAuditee.clear();
    if(!mpDB->isValidProjId(strProjId))     strProjId.clear();

         if(strProjId.isEmpty())    { ui->statusUpdater->updateStatus("Pls Enter Valid Project DORF id");     return nullptr; }
    else if(strRvwName.isEmpty())   { ui->statusUpdater->updateStatus("Pls Enter a Review Name");       return nullptr; }
    else if(strReviewer.isEmpty())  { ui->statusUpdater->updateStatus("Pls Enter Valid Reviewer CDSID");return nullptr; }
    else if(strAuditee.isEmpty())   { ui->statusUpdater->updateStatus("Pls Enter Valid Auditee CDSID"); return nullptr; }
    else if(!mpDB->isValidAuditee(strAuditee, strProjId)) {
            ui->statusUpdater->updateStatus(strAuditee + " does not belong to " + mpDB->getProjName(strProjId));
            return nullptr;
        }

    pSchdl              = std::make_shared<Schedule>();
    pSchdl->mProjId     = strProjId.toInt();
    pSchdl->mStatus     = 0;

    pSchdl->mStartDate  = dtStartDate.toString("yyyy-MM-dd");
    pSchdl->mEndDate    = dtEndDate.toString("yyyy-MM-dd");
    pSchdl->mActualDate = "1970-01-01";
    pSchdl->mCloseDate  = "1970-01-01";

//  pSchdl->mReviewName = dtStartDate.toString("yyyy-MM") + QString(" ") + strRvwName;
    pSchdl->mReviewName = strRvwName;
    pSchdl->mReviewer   = strReviewer;
    pSchdl->mAuditee    = strAuditee;
    pSchdl->mScheduler  = mpDB->getCurUserCdsid();

    return pSchdl;
}

void SchedulePage::clearScheduleDetails() {
    ui->lnEdtSchdlRevwName->clear();
    ui->lnEdtSchdlProjId->clear();
    ui->lnEdtSchdlReviewer->clear();
    ui->lnEdtSchdlAuditee->clear();
}

void SchedulePage::clearSchdlTable() {
    ui->tblWdgtSchedules->setRowCount(0);
    ui->tblWdgtSchedules->setColumnCount(0);
    ui->tblWdgtSchedules->clear();
    mCurRowIndex = -1;
}

void SchedulePage::onTblWdgtVHeaderClicked(int index) {
    onTblWdgtSchedulesClicked(index);
}

void SchedulePage::onBtnSchdlRemindClicked() {
    std::stringstream ssTo, ssCnt;
    const auto& allSchdls   = mpDB->getAllSchedules();

    ssTo << "To: ";
    std::string strPrefix;
    for(int32_t iLoop = 0; iLoop < allSchdls.size(); iLoop++) {
        Schedule::Ptr pSchdl= allSchdls[iLoop];

        if(pSchdl->mStatus != Schedule::stringStatusToInt("Closed")) {
            ssTo    << strPrefix << pSchdl->mReviewer.toStdString() << "; " << pSchdl->mAuditee.toStdString();
            ssCnt   << "\n\nHi "
                    << mpDB->getNameForCdsid(pSchdl->mReviewer) << ", "
                    << mpDB->getNameForCdsid(pSchdl->mAuditee) << "\n"
                    << "\tReview Name : " << pSchdl->mReviewName.toStdString() << " is not closed yet." << std::endl
                    << "\tLast date for closing is " << pSchdl->mEndDate.toStdString() << "." << std::endl
                    << "\tPls close ASAP.\n-"
                    << mpDB->getNameForCdsid(pSchdl->mScheduler);
        }
        strPrefix   = "; ";
    }

    QFile file("../Images/mail.txt");
    file.open(QFile::WriteOnly | QFile::Text);
    QTextStream outF(&file);

    outF << QString(ssTo.str().c_str())
         << "\n\n" << "Subj: ISO Review Reminder - " << QDate::currentDate().toString("dd-MMM-yyyy")
         << QString(ssCnt.str().c_str());
    file.flush();
    file.close();

    QProcess *myProcess  = new QProcess();
    myProcess->start("C:\\Windows\\System32\\notepad.exe", QStringList() << "..\\Images\\mail.txt");
}
//--------------------------------------------------------------------------------------------------------
//                                      Handle Ui }}}
//--------------------------------------------------------------------------------------------------------






//--------------------------------------------------------------------------------------------------------
//                                      Network Responses {{{
//--------------------------------------------------------------------------------------------------------

void SchedulePage::respInsertUpdateSchdl(QNetworkReply *pReply) {
    disconnect(mpHttpMgr, &QNetworkAccessManager::finished, this, &SchedulePage::respInsertUpdateSchdl);

    const auto& resp    = QString(pReply->readAll());
    json root           = json::parse(resp.toStdString(), nullptr, false);
    if(root.is_discarded() || root["isOk"] == false) { ui->statusUpdater->updateStatus("Error updating schedule"); return; }
    ui->statusUpdater->updateStatus("Successfully updated/deleted entries in DB");

    mpDB->triggerDBPull();
    mbEyeOnDBUpdate = true;
}

void SchedulePage::onDBNotify(int32_t pUserInt, const QString& pUserStr) {
    if(!mbEyeOnDBUpdate) return;

    clearSchdlTable();
    populateSchdlTable();
    populateDORFLookupTable();
    mbEyeOnDBUpdate = false;
}

//--------------------------------------------------------------------------------------------------------
//                                      Network Responses }}}
//--------------------------------------------------------------------------------------------------------
