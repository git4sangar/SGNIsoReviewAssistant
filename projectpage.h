    //sgn
#ifndef PROJECTPAGE_H
#define PROJECTPAGE_H

#include <iostream>
#include <memory>
#include <QObject>
#include <QVector>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QListWidget>
#include <QTextEdit>
#include <QTableWidget>

#include "dbnotifier.h"
#include "statusupdater.h"
#include "dbinterface.h"

struct ProjUiElements {
    QLineEdit       *lnEdtProjName = nullptr,
                    *lnEdtProjId = nullptr,
                    *lnEdtProjPM = nullptr,
                    *lnEdtProjLL6 = nullptr;
    QTextEdit       *txtEdtCdsids;
    QPushButton     *btnProjNew = nullptr,
                    *btnProjGetDetails = nullptr,
                    *btnProjSubmit = nullptr;
    QTableWidget    *tblWdgtProjects = nullptr;
    StatusUpdater   *statusUpdater = nullptr;
};

class ProjectPage : public QObject, public DBNotifier
{
    Q_OBJECT

private slots:
    void respInsertNewProj(QNetworkReply *pReply);
    void respUpdateProject(QNetworkReply *pReply);
    void respGetAllProjs(QNetworkReply *pReply);
    void respGetTeam(QNetworkReply *pReply);
    void respDeleteProj(QNetworkReply *pReply);

public:
    typedef std::shared_ptr<ProjectPage> Ptr;
    ProjectPage(): QObject(nullptr) {}
    ProjectPage(ProjUiElements *pUi, DBInterface::Ptr pDB);
    virtual ~ProjectPage() {}
    //std::shared_ptr<DBNotifier> getSharedPtr() {return shared_from_this();}

    void onBtnProjNewClicked();
    void onBtnProjGetDetailsClicked();
    void onBtnProjSubmitClicked();
    void onTblWdgtProjectsClicked(int row, int column = 0);
    void onTblWdgtVHeaderClicked(int index);
    void onBtnProjDeleteClicked();
    void onDBNotify();

private:
    void            getAllProjsByPM();
    void            insertNewProject();
    void            updateProject();
    void            deleteAllTeam();
    void            clearProjTable();
    void            clearProjDetails();
    void            getAllTeamsByProj(int32_t pProjId);
    void            populateProjTable(const QVector<Project::Ptr>& projs);
    void            populateTeamCdsids(const QVector<Team::Ptr>& teams);
    Project::Ptr    sanityCheck();
    QString         queryToInsertCdsids(int32_t iProjId, const QStringList& cdsids);

    int32_t                 mCurRowIndex;
    ProjUiElements          *ui;
    DBInterface::Ptr        mpDB;
    QNetworkAccessManager   *mpHttpMgr;
    QVector<Project::Ptr>   mProjs;
    QVector<Team::Ptr>      mTeams;
};

#endif // PROJECTPAGE_H
