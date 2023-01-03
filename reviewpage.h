//sgn
#ifndef REVIEWPAGE_H
#define REVIEWPAGE_H

#include <QObject>

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTextEdit>

#include "dbnotifier.h"
#include "dbinterface.h"
#include "statusupdater.h"

struct ReviewUiElements {
    QComboBox   *cmBxRevwNames      = nullptr,
                *cmBxRevwCmntStat   = nullptr;
    QLabel      *lblRevwStatus      = nullptr;
    QLineEdit   *lnEdtRevwComment   = nullptr;
    QPushButton *btnRevwAdd         = nullptr,
                *btnRevwDelete      = nullptr,
                *btnRevwUpdate      = nullptr;
    QTableWidget *tblWdgtCmnts      = nullptr;
    QTextEdit   *txtEdtRevwCmnt     = nullptr;
    StatusUpdater   *statusUpdater  = nullptr;
};

class ReviewPage : public QObject, virtual public DBNotifier
{
    Q_OBJECT

private slots:
    void    respInsertCmnt(QNetworkReply *pReply);
public:
    typedef std::shared_ptr<ReviewPage> Ptr;
    ReviewPage(ReviewUiElements *pUi, DBInterface::Ptr pDB);
    virtual ~ReviewPage() {}

    void onBtnRevwAddClicked();
    void onBtnRevwUpdateClicked();
    void onBtnRevwDeleteClicked();
    void onCmBxRevwNamesIdxChanged(int index);
    void onCmBxRevwCmntStatChanged(int index);
    void onTblWdgtCmntsClicked(int row, int col = 0);
    void onTblWdgtVHeaderClicked(int index);
    void onDBNotify();

private:
    void            populateCommentsTable(int32_t iRevwId);
    void            clearCommentsTable();
    void            populateReviewList();

    int32_t                 mCurRowIndex, mCurSchdlIndex;
    bool                    mbEyeOnDBUpdate, mIsReviewer, mIsAuditee;
    ReviewPage::Ptr         sanityCheck();
    ReviewUiElements        *ui;
    DBInterface::Ptr        mpDB;
    QNetworkAccessManager   *mpHttpMgr;
};

#endif // REVIEWPAGE_H
