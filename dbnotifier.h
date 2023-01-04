//sgn
#ifndef DBNOTIFIER_H
#define DBNOTIFIER_H

#include <memory>
#include <QString>
#include <QObject>

class DBNotifier {
    int32_t mUserInt;
    QString mUserStr;

public:
    typedef std::shared_ptr<DBNotifier> Ptr;
    DBNotifier() : mUserInt(0) {}
    virtual ~DBNotifier() {}

    virtual void    onDBNotify(int32_t pUserInt = 0, const QString& pUserStr = QString()) = 0;

    int32_t         getUserInt() { return mUserInt; }
    void            setUserInt(int32_t pInt) { mUserInt = pInt; }

    QString         getUserStr() { return mUserStr; }
    void            setUserStr(const QString& pStr) { mUserStr = pStr; }

    void            clearParams() { mUserInt = 0; mUserStr.clear(); }
};

#endif // DBNOTIFIER_H
