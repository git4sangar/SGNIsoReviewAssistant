//sgn
#include <iostream>
#include <sstream>
#include <QDebug>

#include "nlohmann_json.hpp"
#include "dbinterface.h"

using json = nlohmann::json;

DBInterface::DBInterface(const QString& pCurUserCdsid, const QString& pLL6Cdsid)
    : QObject(nullptr)
    , mCurUserCdsid(pCurUserCdsid)
    , mLL6Cdsid(pLL6Cdsid)
    , mpHttpMgr(new QNetworkAccessManager()) {

    if(mLL6Cdsid.isEmpty()) mLL6Cdsid = "DBASKAR1";

    mpDBPuller  = new QTimer();
    connect(mpDBPuller, SIGNAL(timeout()), this, SLOT(pullFromDB()));
}

void DBInterface::subscribeToDBNotification(DBNotifier::Ptr pSubscriber) {
    if(pSubscriber) mpDBSubscribers.push_back(pSubscriber);
}

void DBInterface::pullFromDB() {
    mpDBPuller->stop();
    mAllProjects.clear(); mAllTeams.clear(); mAllSchedules.clear();
    QNetworkRequest request = makeSelectRequest();
    connect(mpHttpMgr, &QNetworkAccessManager::finished, this, &DBInterface::onAllCdsids);
    mpHttpMgr->put(request, QString("SELECT * FROM cdsids;").toUtf8());
}

void DBInterface::onAllCdsids(QNetworkReply *pReply) {
    disconnect(mpHttpMgr, &QNetworkAccessManager::finished, this, &DBInterface::onAllCdsids);

    const auto& resp    = QString(pReply->readAll());
    json root           = json::parse(resp.toStdString(), nullptr, false);
    if(root.is_discarded() || root["isOk"] == false) return;

    if(root["rows"].is_array() && mAllCdsids.size() != root.value<int32_t>("count", 0)) {
        mAllCdsids.clear();
        for(const auto& row :root["rows"]) {
            Cdsid::Ptr pCdsid   = Cdsid::fromJson(row);
            if(pCdsid) mAllCdsids.push_back(pCdsid);
        }
    }

    std::stringstream ss;
    ss << "SELECT * FROM project WHERE ll6 = \"" << mLL6Cdsid.toStdString() << "\" ORDER BY name ASC;";
    QNetworkRequest request = makeSelectRequest();
    connect(mpHttpMgr, &QNetworkAccessManager::finished, this, &DBInterface::onAllProjects);
    mpHttpMgr->put(request, QString(ss.str().c_str()).toUtf8());
}

void DBInterface::onAllProjects(QNetworkReply *pReply) {
    disconnect(mpHttpMgr, &QNetworkAccessManager::finished, this, &DBInterface::onAllProjects);

    const auto& resp    = QString(pReply->readAll());
    json root           = json::parse(resp.toStdString(), nullptr, false);
    if(root.is_discarded() || root["isOk"] == false) return;

    if(root["rows"].is_array()) for(const auto& row :root["rows"]) {
        Project::Ptr pProj  = Project::fromJson(row);
        if(pProj) mAllProjects.push_back(pProj);
    }

    std::stringstream ss;
    ss << "SELECT * FROM team WHERE proj_id IN (SELECT DISTINCT proj_id FROM project WHERE ll6 = \""
       << mLL6Cdsid.toStdString() << "\");";
    QNetworkRequest request = makeSelectRequest();
    connect(mpHttpMgr, &QNetworkAccessManager::finished, this, &DBInterface::onAllTeams);
    mpHttpMgr->put(request, QString(ss.str().c_str()).toUtf8());
}

void DBInterface::onAllTeams(QNetworkReply *pReply) {
    disconnect(mpHttpMgr, &QNetworkAccessManager::finished, this, &DBInterface::onAllTeams);

    const auto& resp    = QString(pReply->readAll());
    json root           = json::parse(resp.toStdString(), nullptr, false);
    if(root.is_discarded() || root["isOk"] == false) return;

    if(root["rows"].is_array()) for(const auto& row :root["rows"]) {
        Team::Ptr pTeam  = Team::fromJson(row);
        if(pTeam) mAllTeams.push_back(pTeam);
    }

    std::stringstream ss;
    ss << "SELECT * FROM schedule WHERE proj_id IN (SELECT DISTINCT proj_id FROM project WHERE ll6 = \""
       << mLL6Cdsid.toStdString() << "\");";
    QNetworkRequest request = makeSelectRequest();
    connect(mpHttpMgr, &QNetworkAccessManager::finished, this, &DBInterface::onAllSchedules);
    mpHttpMgr->put(request, QString(ss.str().c_str()).toUtf8());
}

void DBInterface::onAllSchedules(QNetworkReply *pReply) {
    disconnect(mpHttpMgr, &QNetworkAccessManager::finished, this, &DBInterface::onAllSchedules);

    const auto& resp    = QString(pReply->readAll());
    json root           = json::parse(resp.toStdString(), nullptr, false);
    if(root.is_discarded() || root["isOk"] == false) return;

    if(root["rows"].is_array()) for(const auto& row :root["rows"]) {
        Schedule::Ptr pSchdl  = Schedule::fromJson(row);
        if(pSchdl) mAllSchedules.push_back(pSchdl);
    }

    //mpDBPuller->start(ONE_MIN);
    for(int32_t iLoop = 0; iLoop < mpDBSubscribers.size(); iLoop++)
        mpDBSubscribers[iLoop]->onDBNotify();
}

QNetworkRequest DBInterface::makeSelectRequest() {
    QNetworkRequest request;
    QString strUrl      = QString("http://68.183.84.172:8080/selectquery");
    request.setUrl(QUrl(strUrl));
    return request;
}

//  This is a multi-query statment delimited by '$'
//  The first one is a select-query followed by 1 or more update-queries
//  Only when the select-query yields the value in bFlag, the subsequent updates get executed
//  Eg: If bFlag is false, all the update-queries execute only when first select-query fails
//  If bFlag is true, all the update-queries execute only when first select-query succeeds
QNetworkRequest DBInterface::makeSelectAndUpdateReq(bool bFlag) {
    QNetworkRequest request;
    QString         strUrl;

    strUrl  = (bFlag) ?
        QString("http://68.183.84.172:8080/selectsucceedupdate") :
        QString("http://68.183.84.172:8080/selectnotupdate");
    request.setUrl(QUrl(strUrl));

    return request;
}

//  This is a multi-update queries delimited by '$'
QNetworkRequest DBInterface::makeUpdateRequest() {
    QNetworkRequest request;
    QString strUrl      = QString("http://68.183.84.172:8080/sqlupdate");
    request.setUrl(QUrl(strUrl));
    return request;
}

const QVector<Cdsid::Ptr>&      DBInterface::getAllCdsids()     { return mAllCdsids;   }
const QVector<Project::Ptr>&    DBInterface::getAllProjects()   { return mAllProjects; }
const QVector<Team::Ptr>&       DBInterface::getAllTeams()      { return mAllTeams; }
const QVector<Schedule::Ptr>&   DBInterface::getAllSchedules()  { return mAllSchedules; }
const QString&                  DBInterface::getCurUserCdsid()  { return mCurUserCdsid;}
void                            DBInterface::triggerDBPull()    { pullFromDB(); }
QString                         DBInterface::getLL6Cdsid()      { return mLL6Cdsid; }

bool DBInterface::isValidCdsid(const QString& cdsid) {
    if(cdsid.isEmpty()) return false;
    int32_t iLoop = 0, iNoOfCdsids = mAllCdsids.size();
    for(iLoop = 0; iLoop < iNoOfCdsids; iLoop++)
        if(mAllCdsids[iLoop]->mCdsid == cdsid.trimmed()) return true;
    return false;
}

bool DBInterface::isValidProjId(int32_t pProjId) {
    int32_t iLoop = 0, iNoOfProjs = mAllProjects.size();
    for(iLoop = 0; iLoop < iNoOfProjs; iLoop++)
        if(mAllProjects[iLoop]->mProjId == pProjId) return true;
    return false;
}

bool DBInterface::isValidProjId(const QString& strProjId) {
    if(strProjId.isEmpty()) return false;
    return isValidProjId(strProjId.toInt());
}

bool DBInterface::isValidAuditee(const QString& pAuditee, const QString& strProjId) {
    int32_t iLoop = 0, iNoOfTeamMembs = mAllTeams.size(), iProjId = strProjId.toInt();
    for(iLoop = 0; iLoop < iNoOfTeamMembs; iLoop++) {
        if(mAllTeams[iLoop]->mCdsid == pAuditee && mAllTeams[iLoop]->mProjId == iProjId)
            return true;
    }
    return false;
}

QString DBInterface::getProjName(const QString& strProjId){
    if(strProjId.isEmpty()) return QString();

    int32_t iLoop       = 0,
            iProjId     = strProjId.toInt();
    for(iLoop = 0; iLoop < mAllProjects.size(); iLoop++)
        if(mAllProjects[iLoop]->mProjId == iProjId)
            return mAllProjects[iLoop]->mName;
    return QString();
}
