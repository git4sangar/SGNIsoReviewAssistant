//sgn

#include <iostream>
#include <memory>
#include <sstream>
#include <QString>
#include <QStringList>

#include "nlohmann_json.hpp"
#include "datatypes.h"

using json = nlohmann::json;


QString Cdsid::getInsertQuery() {
    QStringList ss;
    ss << "INSERT INTO cdsids (cdsid, name) VALUES ('" << mCdsid << "', '" << mName << "');";
    return ss.join("");
}

QString Team::getInsertQuery() { return ""; }

QString getQString(const json& jsRoot, const std::string& colName)
{ return QString(jsRoot.value<std::string>(colName, "").c_str()); }

Cdsid::Ptr Cdsid::fromJson(const json& pJson) {
    if(pJson.is_discarded()) return nullptr;

    Cdsid::Ptr pCdsid   = std::make_shared<Cdsid>();
    pCdsid->mId         = pJson.value<int32_t>("id", 0);
    pCdsid->mCdsid      = getQString(pJson, "cdsid");
    pCdsid->mName       = getQString(pJson, "name");
    return pCdsid;
}

Team::Ptr Team::fromJson(const json& pJson) {
    if (pJson.is_discarded()) return nullptr;

    Team::Ptr pTeam = std::make_shared<Team>();
    pTeam->mId      = pJson.value<int32_t>("id", 0);
    pTeam->mProjId  = pJson.value<int32_t>("proj_id", 0);
    pTeam->mCdsid   = getQString(pJson, "cdsid");
    return pTeam;
}

//--------------------------------------------------------------------------------------------------------
//                                              Comment {{{
//--------------------------------------------------------------------------------------------------------

Comment::Ptr Comment::fromJson(const json& pJson) {
    if(pJson.is_discarded()) return nullptr;

    Comment::Ptr pComment= std::make_shared<Comment>();
    pComment->mId        = pJson.value<int32_t>("id", 0);
    pComment->mReviewId  = pJson.value<int32_t>("review_id", 0);
    pComment->mStatus    = pJson.value<int32_t>("status", 0);
    pComment->mComment   = getQString(pJson, "comment");
    pComment->mReviewer  = getQString(pJson, "reviewer");
    return pComment;
}

QString Comment::getInsertQuery() {
    std::stringstream ss;
    ss  << "INSERT INTO review (review_id, status, comment, reviewer) VALUES ("
        << mReviewId << ", " << mStatus << ", \"" << mComment.toStdString()
        << "\", \"" << mReviewer.toStdString() << "\");";
    return QString(ss.str().c_str());
}

//  The column names shall match cols in getFieldsAsWidgetItems
QStringList Comment::getColNames() {
    QStringList columnNames;
    columnNames << "RvwId" << "CmntId" << "Comment";
    return columnNames;
}

//  The vector size shall match column names in getColNames
QVector<QTableWidgetItem*> Comment::getFieldsAsWidgetItems() {
    QVector<QTableWidgetItem*> pItems;
    QTableWidgetItem* pItem = nullptr;

    pItem = new QTableWidgetItem(); pItem->setData(Qt::DisplayRole, QVariant(mReviewId));   pItems.push_back(pItem);
    pItem = new QTableWidgetItem(); pItem->setData(Qt::DisplayRole, QVariant(mId));         pItems.push_back(pItem);
    pItem = new QTableWidgetItem(); pItem->setText(mComment);                               pItems.push_back(pItem);

    return pItems;
}

//--------------------------------------------------------------------------------------------------------
//                                              Comment }}}
//--------------------------------------------------------------------------------------------------------




//--------------------------------------------------------------------------------------------------------
//                                              Schedule {{{
//--------------------------------------------------------------------------------------------------------

Schedule::Ptr Schedule::fromJson(const json& pJson) {
    if(pJson.is_discarded()) return nullptr;

    Schedule::Ptr pSchedule = std::make_shared<Schedule>();
    pSchedule->mId          = pJson.value<int32_t>("id", 0);
    pSchedule->mReviewId    = pJson.value<int32_t>("review_id", 0);
    pSchedule->mProjId      = pJson.value<int32_t>("proj_id", 0);
    pSchedule->mStatus      = pJson.value<int32_t>("status", 0);

    pSchedule->mStartDate   = pJson.value<int64_t>("start_date", 0);
    pSchedule->mEndDate     = pJson.value<int64_t>("end_date", 0);
    pSchedule->mActualDate  = pJson.value<int64_t>("actual_date", 0);
    pSchedule->mCloseDate   = pJson.value<int64_t>("close_date", 0);

    pSchedule->mReviewName  = getQString(pJson, "review_name");
    pSchedule->mReviewer    = getQString(pJson, "reviewer");
    pSchedule->mAuditee     = getQString(pJson, "auditee");
    pSchedule->mScheduler   = getQString(pJson, "scheduler");
    return pSchedule;
}

QString Schedule::getInsertQuery() {
    std::string strStartDate    = mStartDate.toStdString(),     strEndDate  = mEndDate.toStdString(),
                strActualDate   = mActualDate.toStdString(),    strCloseDate= mCloseDate.toStdString(),
                strReviewName   = mReviewName.toStdString(),    strAuditee  = mAuditee.toStdString(),
                strReviewer     = mReviewer.toStdString(),      strScheduler= mScheduler.toStdString();

    std::stringstream ss;
    ss  << "INSERT INTO schedule (review_id, proj_id, status, start_date, end_date, actual_date, close_date"
        << ", review_name, auditee, reviewer, scheduler) "
        << "SELECT COALESCE(MAX(id), 0) + 1, " << mProjId << ", 0, \"" << strStartDate << "\", \""
        << strEndDate << "\", \"" << strActualDate << "\", \"" << strCloseDate << "\", \"" << strReviewName
        << "\", \"" << strAuditee << "\", \"" << strReviewer << "\", \"" << strScheduler << "\" FROM schedule;";
    return QString(ss.str().c_str());
}

QString Schedule::getUpdateQuery(Schedule::Ptr pSchdl) {
    std::string strStartDate    = pSchdl->mStartDate.toStdString(), strEndDate  = pSchdl->mEndDate.toStdString(),
                strReviewName   = pSchdl->mReviewName.toStdString(),strAuditee  = pSchdl->mAuditee.toStdString(),
                strReviewer     = pSchdl->mReviewer.toStdString();
    std::stringstream ss;
    ss  << "UPDATE schedule SET review_name = \"" << strReviewName << "\", proj_id = " << mProjId
        << ", reviewer = \"" << strReviewer << "\", auditee = \"" << strAuditee << "\", start_date = \""
        << strStartDate << "\", end_date =  \"" << strEndDate << "\" WHERE review_id = " << mReviewId << ";";
    return QString(ss.str().c_str());
}

//  The column names shall match cols in getFieldsAsWidgetItems
QStringList Schedule::getColNames() {
    QStringList columnNames;
    columnNames << "Review Id" << "Review Name" << "Reviewer" << "Auditee" << "Status";
    return columnNames;
}

//  The vector size shall match column names in getColNames
QVector<QTableWidgetItem*> Schedule::getFieldsAsWidgetItems() {
    QVector<QTableWidgetItem*> pItems;
    QTableWidgetItem* pItem = nullptr;

    QString strStatus   = intStatusToString(mStatus);
    pItem = new QTableWidgetItem(); pItem->setData(Qt::DisplayRole, QVariant(mReviewId)); pItems.push_back(pItem);
    pItem = new QTableWidgetItem(); pItem->setText(mReviewName);pItems.push_back(pItem);
    pItem = new QTableWidgetItem(); pItem->setText(mReviewer);  pItems.push_back(pItem);
    pItem = new QTableWidgetItem(); pItem->setText(mAuditee);   pItems.push_back(pItem);
    pItem = new QTableWidgetItem(); pItem->setText(strStatus);  pItems.push_back(pItem);

    return pItems;
}

QString Schedule::intStatusToString(int32_t iStatus) {
    switch(iStatus) {
        case 0: return "Open";
        case 1: return "In Progress";
        case 2: return "Closed";
    }
    return QString();
}

int32_t Schedule::stringStatusToInt(const QString& strStatus) {
         if(strStatus == "Open")        return 0;
    else if(strStatus == "In Progress") return 1;
    else if(strStatus == "Closed")      return 2;
    return -1;
}

//--------------------------------------------------------------------------------------------------------
//                                              Schedule }}}
//--------------------------------------------------------------------------------------------------------






//--------------------------------------------------------------------------------------------------------
//                                              Project {{{
//--------------------------------------------------------------------------------------------------------
Project::Ptr Project::fromJson(const json& pJson) {
    if (pJson.is_discarded()) return nullptr;

    Project::Ptr pProject   = std::make_shared<Project>();
    pProject->mId           = pJson.value<int32_t>("id", 0);
    pProject->mProjId       = pJson.value<int32_t>("proj_id", 0);
    pProject->mName         = getQString(pJson, "name");
    pProject->mPM           = getQString(pJson, "pm");
    pProject->mLL6          = getQString(pJson, "ll6");
    pProject->mIsoSpoc      = getQString(pJson, "iso_spoc");
    return pProject;
}

//  The column names shall match cols in getFieldsAsWidgetItems
QStringList Project::getColNames() {
    QStringList columnNames;
    columnNames << "Proj Id" << "Proj Name" << "PM" << "LL6";
    return columnNames;
}

//  The vector size shall match column names in getColNames
QVector<QTableWidgetItem*> Project::getFieldsAsWidgetItems() {
    QVector<QTableWidgetItem*> pItems;
    QTableWidgetItem* pItem = nullptr;

    pItem = new QTableWidgetItem(); pItem->setData(Qt::DisplayRole, QVariant(mProjId)); pItems.push_back(pItem);
    pItem = new QTableWidgetItem(); pItem->setText(mName); pItems.push_back(pItem);
    pItem = new QTableWidgetItem(); pItem->setText(mPM);   pItems.push_back(pItem);
    pItem = new QTableWidgetItem(); pItem->setText(mLL6);  pItems.push_back(pItem);

    return pItems;
}

QString Project::getInsertQuery() {
    std::string delimiter = "$";
    std::stringstream ss;
    ss  << "SELECT * FROM project WHERE proj_id = " << mProjId << ";";
    ss  << delimiter;
    ss  << "INSERT INTO project (proj_id, name, pm, ll6, iso_spoc) VALUES ("
        << mProjId << ", \"" << mName.toStdString() << "\", \"" << mPM.toStdString()
        << "\", \"" << mLL6.toStdString() << "\", \"" << mIsoSpoc.toStdString() << "\");";
    return QString(ss.str().c_str());
}

QString Project::getUpdateQuery(Project::Ptr pProj) {
    if(pProj == nullptr) return QString();

    std::string strName = pProj->mName.toStdString();
    std::string strPM   = pProj->mPM.toStdString();
    std::string strLL6  = pProj->mLL6.toStdString();

    std::stringstream ss;
    ss  << "UPDATE project SET proj_id = " << pProj->mProjId << ", name = \"" << strName
        << "\", pm = \"" << strPM << "\", ll6 = \"" << strLL6 << "\" WHERE id = " << mId << ";";
    return QString(ss.str().c_str());
}

//--------------------------------------------------------------------------------------------------------
//                                              Project }}}
//--------------------------------------------------------------------------------------------------------
