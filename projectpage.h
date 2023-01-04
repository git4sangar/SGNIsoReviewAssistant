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

    enum class Cmds { CMD_NONE = 0, CMD_PROJ_BY_ID = 101, CMD_PROJ_BY_PM, CMD_PROJ_BY_LL6, CMD_PROJ_LIKE, CMD_TEAM_BY_PROJ_ID };
    Cmds    intToEnum(int32_t iVal);
    int32_t enumToInt(Cmds pEnum);

private slots:
    void respSelectProjs(QNetworkReply *pReply);

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
    void onBtnProjClear();
    void onDBNotify(int32_t pUserCmd = 0, const QString& pUserParam = QString());
    void listenToDBUpdate(bool bFlag) { mbEyeOnDBUpdate = bFlag; }

private:
    void            insertNewProject();
    void            updateProject();
    void            deleteAllTeam();
    void            clearProjTable();
    void            clearProjDetails();
    void            populateProjTable(const QVector<Project::Ptr>& projs);
    void            populateTeamCdsids(const QVector<Team::Ptr>& teams);
    Project::Ptr    sanityCheck();
    QString         queryToInsertCdsids(int32_t iProjId, const QStringList& cdsids);

    bool                    mbEyeOnDBUpdate;
    int32_t                 mCurRowIndex;
    ProjUiElements          *ui;
    DBInterface::Ptr        mpDB;
    QNetworkAccessManager   *mpHttpMgr;
    QVector<Team::Ptr>      mTeams;
};

#endif // PROJECTPAGE_H
