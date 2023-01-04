//sgn
#ifndef SCHEDULEPAGE_H
#define SCHEDULEPAGE_H

#include <iostream>
#include <memory>
#include <QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

#include <QObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QListWidget>
#include <QDateEdit>
#include <QTableWidget>

#include "statusupdater.h"
#include "dbnotifier.h"
#include "dbinterface.h"

struct SchdlUiElements {
    QLabel          *lblSchdlReviewName = nullptr;
    QLineEdit       *lnEdtSchdlProjId   = nullptr,
                    *lnEdtSchdlRevwName = nullptr,
                    *lnEdtSchdlReviewer = nullptr,
                    *lnEdtSchdlAuditee  = nullptr;
    QDateEdit       *dtEdtSchdlStartDate= nullptr,
                    *dtEdtSchdlEndDate  = nullptr;
    QPushButton     *btnSchdlNew        = nullptr,
                    *btnSchdlUpdate     = nullptr,
                    *btnSchdlDelete     = nullptr,
                    *btnSchdlArchive    = nullptr,
                    *btnSchdlRemind     = nullptr;
    QTableWidget    *tblWdgtSchedules   = nullptr,
                    *tblWdgtSchdlLookup = nullptr;
    StatusUpdater   *statusUpdater      = nullptr;
};

class SchedulePage : public QObject, virtual public DBNotifier
{
    Q_OBJECT

private slots:
    void    respInsertUpdateSchdl(QNetworkReply *pReply);

public:
    typedef std::shared_ptr<SchedulePage> Ptr;
    SchedulePage(SchdlUiElements *pUi, DBInterface::Ptr pDB);
    virtual ~SchedulePage() {}
    //std::shared_ptr<SchedulePage> getSharedPtr() {return shared_from_this();}

    void onBtnSchdlNewClicked();
    void onBtnSchdlUpdateClicked();
    void onBtnSchdlDeleteClicked();
    void onBtnSchdlRemindClicked();
    void onBtnSchdlArchiveClicked();
    void onTblWdgtSchdlLookupClicked(int row, int column = 0);
    void onTblWdgtSchedulesClicked(int row, int column = 0);
    void onTblWdgtVHeaderClicked(int index);
    void populateSchdlTable();
    void onDBNotify(int32_t pUserInt = 0, const QString& pUserStr = QString());
    void listenToDBUpdate(bool bFlag) { mbEyeOnDBUpdate = bFlag; }

private:
    int32_t                 mCurRowIndex;
    bool                    mbEyeOnDBUpdate;
    SchdlUiElements         *ui;
    DBInterface::Ptr        mpDB;
    QNetworkAccessManager   *mpHttpMgr;

    Schedule::Ptr           sanityCheck();
    void                    clearScheduleDetails();
    void                    populateDORFLookupTable();
    void                    insertNewSchedule();
    void                    updateSchedule();
    void                    clearSchdlTable();
};

#endif // SCHEDULEPAGE_H
