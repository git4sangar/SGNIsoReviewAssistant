//sgn
#ifndef DATATYPES_H
#define DATATYPES_H

#include <iostream>
#include <memory>
#include <QString>
#include <QTableWidgetItem>

#include "nlohmann_json.hpp"

using json = nlohmann::json;

struct Cdsid {
    typedef std::shared_ptr<Cdsid> Ptr;
    int32_t     mId;
    QString     mCdsid, mName;

    static Ptr  fromJson(const json& pJson);
    QString     getInsertQuery();
};

struct Team {
    typedef     std::shared_ptr<Team> Ptr;
    int32_t     mId, mProjId;
    QString     mCdsid;

    static Ptr  fromJson(const json& pJson);
    QString     getInsertQuery();
};

struct Review {
    typedef     std::shared_ptr<Review> Ptr;
    int32_t     mId, mReviewId, mStatus;
    QString     mComment, mReviewer;

    static Ptr  fromJson(const QString& pJson);
    QString     getInsertQuery();
};

struct Schedule {
    typedef     std::shared_ptr<Schedule> Ptr;
    int32_t     mId, mReviewId, mProjId, mStatus;
    QString     mStartDate, mEndDate, mActualDate, mCloseDate;
    QString     mReviewName, mAuditee, mReviewer, mScheduler;

    static Ptr                  fromJson(const json& pJson);
    static QStringList          getColNames();
    static QString              intStatusToString(int32_t iStatus);
    static int32_t              stringStatusToInt(const QString& strStatus);
    QString                     getInsertQuery();
    QString                     getUpdateQuery(Schedule::Ptr pSchdl);
    QVector<QTableWidgetItem*>  getFieldsAsWidgetItems();
};

struct Project {
    typedef     std::shared_ptr<Project> Ptr;
    int32_t     mId, mProjId;
    QString     mName, mPM, mLL6, mIsoSpoc;
    QStringList mTeamCdsids;

    static Ptr                  fromJson(const json& pJson);
    static QStringList          getColNames();
    QString                     getInsertQuery();
    QString                     getUpdateQuery(Project::Ptr pProj);
    QVector<QTableWidgetItem*>  getFieldsAsWidgetItems();
};

#endif // DATATYPES_H

// Date Time SQLite Queries. Insert IST first and UTC next
// INSERT INTO EgDateTime(start_date) VALUES (strftime('%s', 'now', 'localtime')); - IST
// INSERT INTO EgDateTime(start_date) VALUES (strftime('%s', 'now')); - UTC
//
// SELECT datetime(start_date, 'unixepoch') FROM EgDateTime;
// 2022-12-24 10:21:53
// 2022-12-24 04:52:15
//
// SELECT date(start_date, 'unixepoch') FROM EgDateTime;
// 2022-12-24
// 2022-12-24
//
// SELECT time(start_date, 'unixepoch') FROM EgDateTime;
// 10:21:53
// 04:52:15
