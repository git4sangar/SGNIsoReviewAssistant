//sgn
#include <iostream>
#include <sstream>
#include <QString>

#include "projectpage.h"

ProjectPage::ProjectPage(ProjUiElements *pUi, DBInterface::Ptr pDB)
    : QObject(nullptr)
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

         if(iProjId > 0)            ss << "SELECT * FROM project WHERE proj_id = " << iProjId << " ORDER BY proj_id ASC;";
    else if(!strProjName.isEmpty()) ss << "SELECT * FROM project WHERE name LIKE \"%" << strProjName.toStdString() << "%\" ORDER BY proj_id ASC;";
    else if(!strPM.isEmpty())       ss << "SELECT * FROM project WHERE pm = \"" << strPM.toStdString() << "\" ORDER BY proj_id ASC;";
    else if(!strLL6.isEmpty())      ss << "SELECT * FROM project WHERE ll6 = \"" << strLL6.toStdString() << "\" ORDER BY proj_id ASC;";
    else ui->statusUpdater->updateStatus("Pls Enter Proj Id or Name or PM or LL6 to get details.");

     QNetworkRequest request = mpDB->makeSelectRequest();
     connect(mpHttpMgr, &QNetworkAccessManager::finished, this, &ProjectPage::respGetAllProjs);
     mpHttpMgr->put(request, QString(ss.str().c_str()).toUtf8());
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
    ss << pProj->getInsertQuery().toStdString();
    ss << delimiter << queryToInsertCdsids(pProj->mProjId, pProj->mTeamCdsids).toStdString();

    QNetworkRequest request = mpDB->makeSelectAndUpdateReq(false);
    connect(mpHttpMgr, &QNetworkAccessManager::finished, this, &ProjectPage::respInsertNewProj);
    mpHttpMgr->put(request, QString(ss.str().c_str()).toUtf8());
}

void ProjectPage::updateProject() {
    if(mCurRowIndex == -1) {ui->statusUpdater->updateStatus("Pls choose a project"); return;}

    Project::Ptr pToChange  = sanityCheck();
    if(!pToChange) return;
    Project::Ptr pExisting  = mProjs[mCurRowIndex];

    std::stringstream ss;
    std::string delimiter   = "$";
    ss << pExisting->getUpdateQuery(pToChange).toStdString()
       << delimiter
       << "DELETE FROM team WHERE proj_id = " << pExisting->mProjId << ";"
       << delimiter
       << queryToInsertCdsids(pExisting->mProjId, pToChange->mTeamCdsids).toStdString();

    QNetworkRequest request = mpDB->makeUpdateRequest();
    connect(mpHttpMgr, &QNetworkAccessManager::finished, this, &ProjectPage::respUpdateProject);
    mpHttpMgr->put(request, QString(ss.str().c_str()).toUtf8());
}

void ProjectPage::populateProjTable(const QVector<Project::Ptr>& projs) {
    if(projs.size() == 0) return;
    clearProjTable();

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
    for(int32_t iLoop = 0; iLoop < mProjs.size(); iLoop++) {
        if(mProjs[iLoop]->mProjId == iProjId) { pProj = mProjs[iLoop]; break; }
    }

    if(pProj) {
        ui->lnEdtProjId->setText(QString::number(pProj->mProjId));
        ui->lnEdtProjName->setText(pProj->mName);
        ui->lnEdtProjPM->setText(pProj->mPM);
        ui->lnEdtProjLL6->setText(pProj->mLL6);
    }

    ui->btnProjSubmit->setText("Update");
    getAllTeamsByProj(pProj->mProjId);
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
    ui->lnEdtProjLL6->setText(ui->lnEdtProjLL6->text());
    ui->txtEdtCdsids->clear();
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
    Project::Ptr pProj  = mProjs[mCurRowIndex];
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
    QNetworkRequest request = mpDB->makeSelectAndUpdateReq(true);
    connect(mpHttpMgr, &QNetworkAccessManager::finished, this, &ProjectPage::respDeleteProj);
    mpHttpMgr->put(request, QString(ss.str().c_str()).toUtf8());
}

//--------------------------------------------------------------------------------------------------------
//                                      Handle Ui }}}
//--------------------------------------------------------------------------------------------------------





//--------------------------------------------------------------------------------------------------------
//                                      Network Responses {{{
//--------------------------------------------------------------------------------------------------------

void ProjectPage::respInsertNewProj(QNetworkReply *pReply) {
    disconnect(mpHttpMgr, &QNetworkAccessManager::finished, this, &ProjectPage::respInsertNewProj);

    const auto& resp    = QString(pReply->readAll());
    json root           = json::parse(resp.toStdString(), nullptr, false);
    if(root.is_discarded() || root["isOk"] == false) {
        ui->statusUpdater->updateStatus("Could not create record. Record already exists?");
        return;
    }
    ui->statusUpdater->updateStatus("Updated New Project");
    getAllProjsByPM();
}

void ProjectPage::respUpdateProject(QNetworkReply *pReply) {
    disconnect(mpHttpMgr, &QNetworkAccessManager::finished, this, &ProjectPage::respUpdateProject);

    const auto& resp    = QString(pReply->readAll());
    json root           = json::parse(resp.toStdString(), nullptr, false);
    if(root.is_discarded() || root["isOk"] == false) {
        ui->statusUpdater->updateStatus("Error updating project");
        return;
    }
    ui->statusUpdater->updateStatus("Updated project");
    mpDB->triggerDBPull();
}

void ProjectPage::respGetAllProjs(QNetworkReply *pReply) {
    disconnect(mpHttpMgr, &QNetworkAccessManager::finished, this, &ProjectPage::respGetAllProjs);

    const auto& resp    = QString(pReply->readAll());
    json root           = json::parse(resp.toStdString(), nullptr, false);
    if(root.is_discarded() || !root["isOk"]) { ui->statusUpdater->updateStatus("No projects"); return; }

    QVector<Project::Ptr> allProjs;
    if(root["rows"].is_array()) for(const auto& row :root["rows"]) {
        Project::Ptr pProj  = Project::fromJson(row);
        allProjs.push_back(pProj);
    }
    if(allProjs.size() > 0) {
        mProjs.clear();
        mProjs = allProjs;
        populateProjTable(allProjs);
    }
    mpDB->triggerDBPull();
}

void ProjectPage::respGetTeam(QNetworkReply *pReply) {
    disconnect(mpHttpMgr, &QNetworkAccessManager::finished, this, &ProjectPage::respGetTeam);

    const auto& resp    = QString(pReply->readAll());
    json root           = json::parse(resp.toStdString(), nullptr, false);
    if(root.is_discarded() || !root["isOk"]) return;

    QVector<Team::Ptr> teams;
    if(root["rows"].is_array()) for(const auto& row :root["rows"]) {
        Team::Ptr pTeam  = Team::fromJson(row);
        teams.push_back(pTeam);
    }
    if(teams.size() > 0) { mTeams.clear(); mTeams = teams; }
    populateTeamCdsids(teams);
}

void ProjectPage::respDeleteProj(QNetworkReply *pReply) {
    disconnect(mpHttpMgr, &QNetworkAccessManager::finished, this, &ProjectPage::respDeleteProj);

    const auto& resp    = QString(pReply->readAll());
    json root           = json::parse(resp.toStdString(), nullptr, false);
    if(root.is_discarded() || !root["isOk"]) {
        ui->statusUpdater->updateStatus("Could not delete project");
        return;
    }
    ui->statusUpdater->updateStatus("Project deleted successfully");
    clearProjTable();
    getAllProjsByPM();
    mpDB->triggerDBPull();
}

void ProjectPage::onDBNotify() {}
//--------------------------------------------------------------------------------------------------------
//                                      Network Responses }}}
//--------------------------------------------------------------------------------------------------------





//--------------------------------------------------------------------------------------------------------
//                                      Utils {{{
//--------------------------------------------------------------------------------------------------------
void ProjectPage::getAllTeamsByProj(int32_t pProjId) {
    std::stringstream ss;

    ss << "SELECT * FROM team WHERE proj_id = " << pProjId << ";";
    QNetworkRequest request = mpDB->makeSelectRequest();
    connect(mpHttpMgr, &QNetworkAccessManager::finished, this, &ProjectPage::respGetTeam);
    mpHttpMgr->put(request, QString(ss.str().c_str()).toUtf8());
}

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

void ProjectPage::getAllProjsByPM() {
    std::stringstream ss;
    std::string strPM   = ui->lnEdtProjPM->text().toUpper().toStdString();

    ss << "SELECT * FROM project WHERE pm = \"" << strPM << "\" ORDER BY proj_id ASC;";
    QNetworkRequest request = mpDB->makeSelectRequest();
    connect(mpHttpMgr, &QNetworkAccessManager::finished, this, &ProjectPage::respGetAllProjs);
    mpHttpMgr->put(request, QString(ss.str().c_str()).toUtf8());
}
//--------------------------------------------------------------------------------------------------------
//                                      Utils }}}
//--------------------------------------------------------------------------------------------------------


