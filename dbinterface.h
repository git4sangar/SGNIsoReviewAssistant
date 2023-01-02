//sgn
#ifndef DBINTERFACE_H
#define DBINTERFACE_H

#include <memory>
#include <QObject>
#include <QVector>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#include "dbnotifier.h"
#include "datatypes.h"

#define ONE_SEC     (1000)
#define ONE_MIN     (ONE_SEC * 60)

class DBInterface : public QObject
{
    Q_OBJECT

    QTimer                  *mpDBPuller;
    QVector<Cdsid::Ptr>     mAllCdsids;
    QVector<Project::Ptr>   mAllProjects;
    QVector<Team::Ptr>      mAllTeams;
    QVector<Schedule::Ptr>  mAllSchedules;
    QVector<Comment::Ptr>   mAllComments;
    QString                 mCurUserCdsid, mLL6Cdsid;
    QNetworkAccessManager   *mpHttpMgr;
    QVector<DBNotifier::Ptr> mpDBSubscribers;

private slots:
    void onAllCdsids(QNetworkReply *pReply);
    void onAllProjects(QNetworkReply *pReply);
    void onAllTeams(QNetworkReply *pReply);
    void onAllSchedules(QNetworkReply *pReply);
    void onAllRevwCmnts(QNetworkReply *pReply);
    void pullFromDB();

public:
    typedef std::shared_ptr<DBInterface> Ptr;
    DBInterface(const QString& pCurUserCdsid, const QString& pLL6Cdsid);
    virtual ~DBInterface() {}

    const QVector<Cdsid::Ptr>&      getAllCdsids();
    const QVector<Project::Ptr>&    getAllProjects();
    const QVector<Team::Ptr>&       getAllTeams();
    const QVector<Schedule::Ptr>&   getAllSchedules();
    const QVector<Comment::Ptr>&    getAllComments();
    Schedule::Ptr                   getScheduleByIndex(int32_t idx);

    const QString&                  getCurUserCdsid();
    void                            triggerDBPull();
    void                            subscribeToDBNotification(DBNotifier::Ptr pSubscriber);
    bool                            isValidCdsid(const QString& pCdsid);
    bool                            isValidProjId(int32_t pProjId);
    bool                            isValidProjId(const QString& strProjId);
    bool                            isValidAuditee(const QString& pAuditee, const QString& strProjId);
    QString                         getProjName(const QString& strProjId);
    QString                         getLL6Cdsid();
    void                            setLL6Cdsid(const QString& pLL6Cdsid);
    QNetworkRequest                 makeSelectRequest();
    QNetworkRequest                 makeUpdateRequest();
    QNetworkRequest                 makeSelectAndUpdateReq(bool bFlag);
};

#endif // DBINTERFACE_H
