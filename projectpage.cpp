//sgn
#include <iostream>
#include <sstream>
#include <QString>

#include "projectpage.h"

ProjectPage::ProjectPage(ProjUiElements *pUi, DBInterface::Ptr pDB)
    : QObject(nullptr)
    , mbEyeOnDBUpdate(false)
    , mCurRowIndex(-1)
    , ui(pUi)
    , mpDB(pDB)
    , mpHttpMgr(new QNetworkAccessManager()){
    //  Validator needs ProjectPage to be inherited from QObject
    ui->lnEdtProjId->setValidator(new QIntValidator(0, 9999999, this));
}

//--------------------------------------------------------------------------------------------------------
//                          Handle Ui {{{
//--------------------------------------------------------------------------------------------------------

void ProjectPage::onBtnProjNewClicked() {
    clearProjDetails();
    clearProjTable();
    ui->btnProjSubmit->setText("Submit");
    ui->statusUpdater->updateStatus("Pls Enter all proj details & click submit");
}

void ProjectPage::onBtnProjGetDetailsClicked() {
    std::stringstream ss;
    int32_t iProjId = 0;

    QString strProjName     = ui->lnEdtProjName->text();
    QString strProjId       = ui->lnEdtProjId->text();
    QString strPM           = ui->lnEdtProjPM->text().toUpper();
    QString strLL6          = ui->lnEdtProjLL6->text().toUpper();
    if(!strProjId.isEmpty()) iProjId = strProjId.toInt();

         if(iProjId > 0)            { setUserInt(enumToInt(Cmds::CMD_PROJ_BY_ID));  setUserStr(strProjId); }
    else if(!strPM.isEmpty())       { setUserInt(enumToInt(Cmds::CMD_PROJ_BY_PM));  setUserStr(strPM); }
    else if(!strLL6.isEmpty())      { setUserInt(enumToInt(Cmds::CMD_PROJ_BY_LL6)); setUserStr(strLL6); }
    else if(!strProjName.isEmpty()) { setUserInt(enumToInt(Cmds::CMD_PROJ_LIKE));   setUserStr(strProjName); }
    else ui->statusUpdater->updateStatus("Pls Enter Proj Id or Name or PM or LL6 to get details.");

    mbEyeOnDBUpdate = true;
    mpDB->triggerDBPull();
}

void ProjectPage::onBtnProjSubmitClicked() {
         if(ui->btnProjSubmit->text() == "Submit") insertNewProject();
    else if(ui->btnProjSubmit->text() == "Update") updateProject();
}

void ProjectPage::insertNewProject() {
    Project::Ptr pProj      = sanityCheck();
    if(!pProj) return;

    std::stringstream ss;
    std::string delimiter   = "$";
    ss  << "SELECT * FROM project WHERE proj_id = " << pProj->mProjId << ";";
    ss  << delimiter;
    ss  << pProj->getInsertQuery().toStdString();
    ss  << delimiter;
    ss  << queryToInsertCdsids(pProj->mProjId, pProj->mTeamCdsids).toStdString();

    QNetworkRequest request = mpDB->makeSelectAndUpdateReq(false);
    connect(mpHttpMgr, &QNetworkAccessManager::finished, this, &ProjectPage::respSelectProjs);
    mpHttpMgr->put(request, QString(ss.str().c_str()).toUtf8());
}

void ProjectPage::updateProject() {
    if(mCurRowIndex == -1) {ui->statusUpdater->updateStatus("Pls choose a project"); return;}

    Project::Ptr pToChange  = sanityCheck();
    if(!pToChange) return;
    Project::Ptr pExisting  = mpDB->getAllProjects()[mCurRowIndex];
    if(pExisting->mPM != mpDB->getCurUserCdsid() && pExisting->mLL6 != mpDB->getCurUserCdsid())
        { ui->statusUpdater->updateStatus("Only PM or LL6 can update"); return; }

    std::stringstream ss;
    std::string delimiter   = "$";
    ss << pExisting->getUpdateQuery(pToChange).toStdString()
       << delimiter
       << "DELETE FROM team WHERE proj_id = " << pExisting->mProjId << ";"
       << delimiter
       << queryToInsertCdsids(pExisting->mProjId, pToChange->mTeamCdsids).toStdString();

    QNetworkRequest request = mpDB->makeUpdateRequest();
    connect(mpHttpMgr, &QNetworkAccessManager::finished, this, &ProjectPage::respSelectProjs);
    mpHttpMgr->put(request, QString(ss.str().c_str()).toUtf8());
}

void ProjectPage::populateProjTable(const QVector<Project::Ptr>& projs) {
    clearProjTable();
    if(projs.size() == 0) return;

    QStringList colHeaders  = Project::getColNames();
    ui->tblWdgtProjects->setRowCount(projs.size());
    ui->tblWdgtProjects->setColumnCount(colHeaders.size());
    ui->tblWdgtProjects->setHorizontalHeaderLabels(colHeaders);

    for(int32_t iRow = 0; iRow < projs.size(); iRow++) {
        Project::Ptr pProj = projs[iRow];
        QVector<QTableWidgetItem*> pCols = pProj->getFieldsAsWidgetItems();
        for(int32_t iCol = 0; iCol < pCols.size(); iCol++)
            ui->tblWdgtProjects->setItem(iRow, iCol, pCols[iCol]);
    }
}

void ProjectPage::onTblWdgtProjectsClicked(int row, int column) {
    mCurRowIndex            = row;
    QTableWidgetItem *pItem = ui->tblWdgtProjects->item(row, 0);
    QString strText         = pItem->text();
    int32_t iProjId         = strText.toInt();

    Project::Ptr pProj;
    const auto&  projs      = mpDB->getAllProjects();
    for(int32_t iLoop = 0; iLoop < projs.size(); iLoop++) {
        if(projs[iLoop]->mProjId == iProjId) { pProj = projs[iLoop]; break; }
    }

    if(pProj) {
        ui->lnEdtProjId->setText(QString::number(pProj->mProjId));
        ui->lnEdtProjName->setText(pProj->mName);
        ui->lnEdtProjPM->setText(pProj->mPM);
        ui->lnEdtProjLL6->setText(pProj->mLL6);
    }

    ui->btnProjSubmit->setText("Update");

    setUserInt(enumToInt(Cmds::CMD_TEAM_BY_PROJ_ID));
    setUserStr(QString::number(pProj->mProjId));
    mbEyeOnDBUpdate = true;
    mpDB->triggerDBPull();
}

void ProjectPage::onTblWdgtVHeaderClicked(int index) {
    onTblWdgtProjectsClicked(index);
}

void ProjectPage::populateTeamCdsids(const QVector<Team::Ptr>& teams) {
    if(teams.size() == 0) return;
    QStringList teamCdsids;
    for(int32_t iLoop = 0; iLoop < teams.size(); iLoop++)
        teamCdsids << teams[iLoop]->mCdsid.toUpper();
    ui->txtEdtCdsids->setText(teamCdsids.join(", "));
}

void ProjectPage::clearProjTable() {
    ui->tblWdgtProjects->setRowCount(0);
    ui->tblWdgtProjects->setColumnCount(0);
    ui->tblWdgtProjects->clear();
    mCurRowIndex = -1;
}

void ProjectPage::clearProjDetails() {
    ui->lnEdtProjName->clear();
    ui->lnEdtProjId->clear();
    ui->lnEdtProjPM->clear();
    ui->lnEdtProjLL6->setText(mpDB->getLL6Cdsid());
    ui->txtEdtCdsids->clear();
}

void ProjectPage::onBtnProjClear() {
    clearProjDetails();
}

Project::Ptr ProjectPage::sanityCheck() {
    Project::Ptr pProj;

    QString strProjName     = ui->lnEdtProjName->text();
    QString strProjId       = ui->lnEdtProjId->text();
    QString strPM           = ui->lnEdtProjPM->text().toUpper();
    QString strLL6          = ui->lnEdtProjLL6->text().toUpper();
    QString strTeamCdsids   = ui->txtEdtCdsids->toPlainText().toUpper();
    QStringList cdsids      = strTeamCdsids.split(",");

    if(!mpDB->isValidCdsid(strPM))  strPM.clear();
    if(!mpDB->isValidCdsid(strLL6)) strLL6.clear();

         if(strProjName.isEmpty())  { ui->statusUpdater->updateStatus("Pls Enter Project Name"); return nullptr; }
    else if(strProjId.isEmpty())    { ui->statusUpdater->updateStatus("Pls Enter Project DORF id"); return nullptr; }
    else if(strPM.isEmpty())        { ui->statusUpdater->updateStatus("Pls Enter Valid PM CDSID"); return nullptr; }
    else if(strLL6.isEmpty())       { ui->statusUpdater->updateStatus("Pls Enter Valid LL6 CDSID"); return nullptr; }
    else if(strTeamCdsids.isEmpty()){ ui->statusUpdater->updateStatus("Pls Enter team CDSIDs"); return nullptr; }
    else for(const auto& cdsid : cdsids) if(!mpDB->isValidCdsid(cdsid)) {
             ui->statusUpdater->updateStatus(cdsid + " : is not a valid CDSID");
             return nullptr;
         }

    pProj               = std::make_shared<Project>();
    pProj->mProjId      = strProjId.toInt();
    pProj->mName        = strProjName;
    pProj->mPM          = strPM;
    pProj->mLL6         = strLL6;
    pProj->mTeamCdsids  = cdsids;
    pProj->mIsoSpoc     = "-";
    return pProj;
}

void ProjectPage::onBtnProjDeleteClicked() {
    if(mCurRowIndex == -1) {
        ui->statusUpdater->updateStatus("Pls choose a proj to delete");
        return;
    }

    QString curUser     = mpDB->getCurUserCdsid();
    Project::Ptr pProj  = mpDB->getAllProjects()[mCurRowIndex];
    if(pProj->mPM != curUser && pProj->mLL6 != curUser) {
        ui->statusUpdater->updateStatus("Only PM or LL6 can delete");
        return;
    }

    std::string delimiter = "$";
    std::stringstream ss;
    ss  << "SELECT * FROM project WHERE proj_id = " << pProj->mProjId << ";"
        << delimiter
        << "DELETE FROM project WHERE proj_id = " << pProj->mProjId << ";"
        << delimiter
        << "DELETE FROM team WHERE proj_id = " << pProj->mProjId << ";";

    clearProjDetails();
    QNetworkRequest request = mpDB->makeSelectAndUpdateReq(true);
    connect(mpHttpMgr, &QNetworkAccessManager::finished, this, &ProjectPage::respSelectProjs);
    mpHttpMgr->put(request, QString(ss.str().c_str()).toUtf8());
}

//--------------------------------------------------------------------------------------------------------
//                                      Handle Ui }}}
//--------------------------------------------------------------------------------------------------------





//--------------------------------------------------------------------------------------------------------
//                                      Network Responses {{{
//--------------------------------------------------------------------------------------------------------

void ProjectPage::respSelectProjs(QNetworkReply *pReply) {
    disconnect(mpHttpMgr, &QNetworkAccessManager::finished, this, &ProjectPage::respSelectProjs);

    const auto& resp    = QString(pReply->readAll());
    json root           = json::parse(resp.toStdString(), nullptr, false);
    if(root.is_discarded() || root["isOk"] == false) {
        ui->statusUpdater->updateStatus("Could not update/delete. Record already exists?");
        return;
    }

    ui->statusUpdater->updateStatus("Successfully updated/deleted entries in DB");
    setUserInt(enumToInt(Cmds::CMD_PROJ_BY_PM));
    setUserStr(ui->lnEdtProjPM->text().toUpper());

    mbEyeOnDBUpdate = true;
    mpDB->triggerDBPull();
}

void ProjectPage::onDBNotify(int32_t pUserCmd, const QString& pUserParam) {
    if(!mbEyeOnDBUpdate)    return;
    QVector<Project::Ptr>   projs;
    QVector<Team::Ptr>      teams;
    int32_t                 iProjId = 0;

    mbEyeOnDBUpdate = false;
    switch(intToEnum(pUserCmd)) {
    case Cmds::CMD_PROJ_BY_ID:
        iProjId = pUserParam.toInt();
        for(const auto& proj : mpDB->getAllProjects())
            if(proj->mProjId == iProjId)
                projs.push_back(proj);
        populateProjTable(projs);
        break;

    case Cmds::CMD_PROJ_BY_PM:
        for(const auto& proj : mpDB->getAllProjects())
            if(proj->mPM == pUserParam)
                projs.push_back(proj);
        populateProjTable(projs);
        break;

    case Cmds::CMD_PROJ_BY_LL6:
        for(const auto& proj : mpDB->getAllProjects())
            if(proj->mLL6 == pUserParam)
                projs.push_back(proj);
        populateProjTable(projs);
        break;

    case Cmds::CMD_PROJ_LIKE:
        for(const auto& proj : mpDB->getAllProjects())
            if(proj->mName.contains(pUserParam))
                projs.push_back(proj);
        populateProjTable(projs);
        break;

    case Cmds::CMD_TEAM_BY_PROJ_ID:
        iProjId = pUserParam.toInt();
        for(const auto& team : mpDB->getAllTeams())
            if(team->mProjId == iProjId)
                teams.push_back(team);
        populateTeamCdsids(teams);
        break;

    case Cmds::CMD_NONE:
        break;
    }
    clearParams();
}
//--------------------------------------------------------------------------------------------------------
//                                      Network Responses }}}
//--------------------------------------------------------------------------------------------------------





//--------------------------------------------------------------------------------------------------------
//                                      Utils {{{
//--------------------------------------------------------------------------------------------------------

QString ProjectPage::queryToInsertCdsids(int32_t iProjId, const QStringList& cdsids) {
    if(cdsids.size() == 0 || iProjId <= 0) return QString();
    std::stringstream ss;
    ss << "INSERT INTO team (proj_id, cdsid) VALUES ";
    std::string prefix = "(";
    for(const auto& cdsid :cdsids) {
        QString trCdsid = cdsid.trimmed();
        ss << prefix << iProjId << ", \"" << trCdsid.toStdString() << "\")";
        prefix = ", (";
    }
    ss << ";";
    return QString(ss.str().c_str());
}

ProjectPage::Cmds ProjectPage::intToEnum(int32_t iVal) {
    switch(iVal) {
        case 101: return Cmds::CMD_PROJ_BY_ID;
        case 102: return Cmds::CMD_PROJ_BY_PM;
        case 103: return Cmds::CMD_PROJ_BY_LL6;
        case 104: return Cmds::CMD_PROJ_LIKE;
        case 105: return Cmds::CMD_TEAM_BY_PROJ_ID;
    }
    return Cmds::CMD_NONE;
}

int32_t ProjectPage::enumToInt(ProjectPage::Cmds pEnum) {
    switch(pEnum) {
        case Cmds::CMD_PROJ_BY_ID:  return 101;
        case Cmds::CMD_PROJ_BY_PM:  return 102;
        case Cmds::CMD_PROJ_BY_LL6: return 103;
        case Cmds::CMD_PROJ_LIKE:   return 104;
        case Cmds::CMD_TEAM_BY_PROJ_ID:  return 105;
        case Cmds::CMD_NONE:        return 0;
    }
    return -1;
}
//--------------------------------------------------------------------------------------------------------
//                                      Utils }}}
//--------------------------------------------------------------------------------------------------------


