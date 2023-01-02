//sgn

#include <iostream>
#include <sstream>
#include <QString>
#include <QDate>
#include <QTableWidgetItem>

#include "reviewpage.h"

ReviewPage::ReviewPage(ReviewUiElements *pUi, DBInterface::Ptr pDB)
    : QObject(nullptr)
    , mCurRowIndex(-1)
    , mCurSchdlIndex(-1)
    , mbEyeOnDBUpdate(true)
    , mIsReviewer(false)
    , mIsAuditee(false)
    , ui(pUi)
    , mpDB(pDB)
    , mpHttpMgr(new QNetworkAccessManager()) {
    populateReviewList();
    ui->cmBxRevwCmntStat->addItems(QStringList() << "Open" << "Closed");
}

//--------------------------------------------------------------------------------------------------------
//                                      Handle Ui {{{
//--------------------------------------------------------------------------------------------------------
void ReviewPage::onCmBxRevwNamesIdxChanged(int index) {
    Schedule::Ptr pSchdl    = mpDB->getScheduleByIndex(index);
    if(!pSchdl) return;

    mCurSchdlIndex = index;
    QVector<Comment::Ptr> comments;
    const auto& allCmnts= mpDB->getAllComments();
    for(int32_t iLoop = 0; iLoop < allCmnts.size(); iLoop++) {
        comments.push_back(allCmnts[iLoop]);
    }
    populateCommentsTable(comments);
}

void ReviewPage::populateCommentsTable(const QVector<Comment::Ptr>& comments) {
    if(comments.size() == 0) return;
    clearCommentsTable();

    QStringList colHeaders  = Comment::getColNames();
    ui->tblWdgtCmnts->setRowCount(comments.size());
    ui->tblWdgtCmnts->setColumnCount(colHeaders.size());
    ui->tblWdgtCmnts->setHorizontalHeaderLabels(colHeaders);

    for(int32_t iRow = 0; iRow < comments.size(); iRow++) {
        Comment::Ptr pCmnt = comments[iRow];
        QVector<QTableWidgetItem*> pCols = pCmnt->getFieldsAsWidgetItems();
        for(int32_t iCol = 0; iCol < pCols.size(); iCol++)
            ui->tblWdgtCmnts->setItem(iRow, iCol, pCols[iCol]);
    }
}

void ReviewPage::onTblWdgtCmntsClicked(int row, int col) {
    mCurRowIndex            = row;
    QTableWidgetItem *pItem = ui->tblWdgtCmnts->item(row, 2);
    QString strCmnt         = pItem->text();
    ui->txtEdtRevwCmnt->setText(strCmnt);
}

void ReviewPage::clearCommentsTable() {
    ui->tblWdgtCmnts->setRowCount(0);
    ui->tblWdgtCmnts->setColumnCount(0);
    ui->tblWdgtCmnts->clear();
    mCurRowIndex = -1;
}

void ReviewPage::onBtnRevwAddClicked() {
    if(mCurSchdlIndex < 0) {
        ui->statusUpdater->updateStatus("Pls choose a Review");
        return;
    }

    Schedule::Ptr pSchdl    = mpDB->getAllSchedules()[mCurSchdlIndex];
    if(pSchdl->mReviewer != mpDB->getCurUserCdsid()) {
        ui->statusUpdater->updateStatus("Only " + pSchdl->mReviewer + " can add/edit");
        return;
    }

    QString strCmnt = ui->lnEdtRevwComment->text();
    if(strCmnt.isEmpty()) { ui->statusUpdater->updateStatus("Pls enter a comment"); return; }

    Comment::Ptr pCmnt  = std::make_shared<Comment>();
    pCmnt->mReviewId    = pSchdl->mReviewId;
    pCmnt->mComment     = strCmnt;
    pCmnt->mReviewer    = pSchdl->mReviewer;
    pCmnt->mStatus      = Schedule::stringStatusToInt("Open");

    QNetworkRequest request = mpDB->makeUpdateRequest();
    connect(mpHttpMgr, &QNetworkAccessManager::finished, this, &ReviewPage::respInsertCmnt);
    mpHttpMgr->put(request, pCmnt->getInsertQuery().toUtf8());
}

void ReviewPage::populateReviewList() {
    ui->cmBxRevwNames->clear();
    const auto& allSchdls   = mpDB->getAllSchedules();

    QStringList revwNames;
    for(int32_t iLoop = 0; iLoop < allSchdls.size(); iLoop++) {
        Schedule::Ptr pSchdl = allSchdls[iLoop];
        revwNames << pSchdl->mReviewName;
    }
    if(revwNames.size() > 0) {
        mCurSchdlIndex = 0;
        ui->cmBxRevwNames->addItems(revwNames);
        ui->cmBxRevwNames->setCurrentIndex(0);
    }
}

void ReviewPage::onBtnRevwUpdateClicked() {}
void ReviewPage::onBtnRevwDeleteClicked() {}
void ReviewPage::onCmBxRevwCmntStatChanged(int index) {}

//--------------------------------------------------------------------------------------------------------
//                                      Handle Ui }}}
//--------------------------------------------------------------------------------------------------------





//--------------------------------------------------------------------------------------------------------
//                                      Network Responses {{{
//--------------------------------------------------------------------------------------------------------
void ReviewPage::respInsertCmnt(QNetworkReply *pReply) {
    disconnect(mpHttpMgr, &QNetworkAccessManager::finished, this, &ReviewPage::respInsertCmnt);

    const auto& resp    = QString(pReply->readAll());
    json root           = json::parse(resp.toStdString(), nullptr, false);
    if(root.is_discarded() || root["isOk"] == false) { ui->statusUpdater->updateStatus("Error updating schedule"); return; }
    ui->statusUpdater->updateStatus("Successfully updated/deleted entries in DB");

    mpDB->triggerDBPull();
    mbEyeOnDBUpdate = true;
}

//--------------------------------------------------------------------------------------------------------
//                                      Network Responses }}}
//--------------------------------------------------------------------------------------------------------
void ReviewPage::onDBNotify() {
    if(!mbEyeOnDBUpdate) return;

    populateReviewList();
}
