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
    , mbEyeOnDBUpdate(false)
    , mIsReviewer(false)
    , mIsAuditee(false)
    , ui(pUi)
    , mpDB(pDB)
    , mpHttpMgr(new QNetworkAccessManager()) {
    ui->cmBxRevwCmntStat->addItems(QStringList() << "Open" << "Closed");
}

//--------------------------------------------------------------------------------------------------------
//                                      Handle Ui {{{
//--------------------------------------------------------------------------------------------------------

void ReviewPage::populateReviewList() {
    ui->cmBxRevwNames->clear();
    const auto& allSchdls   = mpDB->getAllSchedules();

    QStringList revwNames;
    for(int32_t iLoop = 0; iLoop < allSchdls.size(); iLoop++) {
        Schedule::Ptr pSchdl = allSchdls[iLoop];
        revwNames << pSchdl->mReviewName;
    }

    if(revwNames.size() > 0) {
        if(mCurSchdlIndex == -1) mCurSchdlIndex = 0;
        ui->cmBxRevwNames->addItems(revwNames);
        ui->cmBxRevwNames->setCurrentIndex(mCurSchdlIndex);  //triggers onCmBxRevwNamesIdxChanged
    }
}

void ReviewPage::onCmBxRevwNamesIdxChanged(int index) {
    Schedule::Ptr pSchdl    = mpDB->getScheduleByIndex(index);
    if(!pSchdl) return;

    mCurSchdlIndex  = index;
    populateCommentsTable(pSchdl->mReviewId);
    updateReviewStatus(pSchdl->mReviewId);
}

void ReviewPage::populateCommentsTable(int32_t iRevwId) {
    clearCommentsTable();
    const QVector<Comment::Ptr>& comments = mpDB->getCommentsByRevwId(iRevwId);
    if(comments.size() == 0) return;

    QBrush  redColor        = QBrush(QColor(255, 0, 0));
    QStringList colHeaders  = Comment::getColNames();

    ui->tblWdgtCmnts->setRowCount(comments.size());
    ui->tblWdgtCmnts->setColumnCount(colHeaders.size());
    ui->tblWdgtCmnts->setHorizontalHeaderLabels(colHeaders);

    for(int32_t iRow = 0; iRow < comments.size(); iRow++) {
        Comment::Ptr pCmnt = comments[iRow];
        QVector<QTableWidgetItem*> pCols = pCmnt->getFieldsAsWidgetItems();
        for(int32_t iCol = 0; iCol < pCols.size(); iCol++) {
            if(pCmnt->mStatus == 0) pCols[iCol]->setForeground(redColor);
            ui->tblWdgtCmnts->setItem(iRow, iCol, pCols[iCol]);
        }
    }
}

void ReviewPage::onTblWdgtCmntsClicked(int row, int col) {
    mCurRowIndex            = row;
    QTableWidgetItem *pItem = ui->tblWdgtCmnts->item(row, 1);
    int32_t iCmntId         = pItem->text().toInt();
    Comment::Ptr pCmnt      = mpDB->getCommentById(iCmntId);

    if(pCmnt) {
        ui->txtEdtRevwCmnt->setText(pCmnt->mComment);
        ui->cmBxRevwCmntStat->setCurrentIndex(pCmnt->mStatus ? 1 : 0);
    }
}

void ReviewPage::onTblWdgtVHeaderClicked(int index) {
    onTblWdgtCmntsClicked(index);
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
    connect(mpHttpMgr, &QNetworkAccessManager::finished, this, &ReviewPage::respInsrtUpdtDel);
    mpHttpMgr->put(request, pCmnt->getInsertQuery().toUtf8());
}

void ReviewPage::onBtnRevwUpdateClicked() {
    Schedule::Ptr pSchdl = mpDB->getScheduleByIndex(mCurSchdlIndex);
    if(!pSchdl || mCurRowIndex < 0) { ui->statusUpdater->updateStatus("Pls choose a Review/Comment"); return; }

    if(pSchdl->mReviewer != mpDB->getCurUserCdsid()) {
        ui->statusUpdater->updateStatus("Only " + pSchdl->mReviewer + " can update");
        return;
    }

    Comment::Ptr pToBeChanged   = std::make_shared<Comment>();
    pToBeChanged->mComment      = ui->txtEdtRevwCmnt->toPlainText();
    pToBeChanged->mStatus       = ui->cmBxRevwCmntStat->currentIndex();

    QTableWidgetItem *pItem     = ui->tblWdgtCmnts->item(mCurRowIndex, 1);
    int32_t iCmntId             = pItem->text().toInt();
    Comment::Ptr pExistingCmnt  = mpDB->getCommentById(iCmntId);
    QString strUpdateQuery      = pExistingCmnt->getUpdateQuery(pToBeChanged);

    QNetworkRequest request = mpDB->makeUpdateRequest();
    connect(mpHttpMgr, &QNetworkAccessManager::finished, this, &ReviewPage::respInsrtUpdtDel);
    mpHttpMgr->put(request, strUpdateQuery.toUtf8());
}

void ReviewPage::onBtnRevwDeleteClicked() {
    Schedule::Ptr pSchdl = mpDB->getScheduleByIndex(mCurSchdlIndex);
    if(!pSchdl || mCurRowIndex < 0) { ui->statusUpdater->updateStatus("Pls choose a Review/Comment"); return; }

    if(pSchdl->mReviewer != mpDB->getCurUserCdsid()) {
        ui->statusUpdater->updateStatus("Only " + pSchdl->mReviewer + " can update");
        return;
    }

    QTableWidgetItem *pItem = ui->tblWdgtCmnts->item(mCurRowIndex, 1);
    int32_t iCmntId         = pItem->text().toInt();
    Comment::Ptr pCmnt      = mpDB->getCommentById(iCmntId);
    QString strDelQuery     = "DELETE FROM review WHERE id = " + QString::number(pCmnt->mId) + ";";

    QNetworkRequest request = mpDB->makeUpdateRequest();
    connect(mpHttpMgr, &QNetworkAccessManager::finished, this, &ReviewPage::respInsrtUpdtDel);
    mpHttpMgr->put(request, strDelQuery.toUtf8());
}

void ReviewPage::onCmBxRevwCmntStatChanged(int index) {}

//--------------------------------------------------------------------------------------------------------
//                                      Handle Ui }}}
//--------------------------------------------------------------------------------------------------------





//--------------------------------------------------------------------------------------------------------
//                                      Network Responses {{{
//--------------------------------------------------------------------------------------------------------
void ReviewPage::respInsrtUpdtDel(QNetworkReply *pReply) {
    disconnect(mpHttpMgr, &QNetworkAccessManager::finished, this, &ReviewPage::respInsrtUpdtDel);

    const auto& resp    = QString(pReply->readAll());
    json root           = json::parse(resp.toStdString(), nullptr, false);
    if(root.is_discarded() || root["isOk"] == false) { ui->statusUpdater->updateStatus("Error updating schedule"); return; }
    ui->statusUpdater->updateStatus("Successfully updated/deleted entries in DB");

    Schedule::Ptr pSchdl = mpDB->getScheduleByIndex(mCurSchdlIndex);
    if(pSchdl) setUserStr(QString(pSchdl->mReviewId));
    mpDB->triggerDBPull();
    mbEyeOnDBUpdate = true;
}

//--------------------------------------------------------------------------------------------------------
//                                      Network Responses }}}
//--------------------------------------------------------------------------------------------------------
void ReviewPage::onDBNotify(int32_t pUserInt, const QString& pUserStr) {
    if(!mbEyeOnDBUpdate) return;
    ui->txtEdtRevwCmnt->clear();

    // This sets cmBxRevwNames and triggers onCmBxRevwNamesIdxChanged
    // onCmBxRevwNamesIdxChanged triggers populateCommentsTable
    clearCommentsTable();
    populateReviewList();
    mbEyeOnDBUpdate = false;
}

void ReviewPage::updateReviewStatus(int32_t iRevwId) {
    int32_t openCmnts = 0, iStatus = -1;
    Comment::Ptr pCmnt;

    const auto& pCmnts = mpDB->getCommentsByRevwId(iRevwId);
    for(int32_t iLoop = 0; iLoop < pCmnts.size();  iLoop++) {
        pCmnt  = pCmnts[iLoop];
        if(pCmnt->mStatus == 0) openCmnts++;
    }
    if(!pCmnt) return;

    (openCmnts == 0) ?
        iStatus = Schedule::stringStatusToInt("Closed") :
        iStatus = Schedule::stringStatusToInt("In Progress");

    if(iStatus > 0) {
        std::stringstream ss;
        ss << "UPDATE schedule SET status = " << iStatus << " WHERE review_id = " << iRevwId << ";";
        QNetworkRequest request = mpDB->makeUpdateRequest();
        mpHttpMgr->put(request, QString(ss.str().c_str()).toUtf8());
    }
    ui->lblRevwStatus->setText("Comments Open: " + QString::number(openCmnts) + QString("/") + QString::number(pCmnts.size()) );
}
