//sgn
#ifndef DBNOTIFIER_H
#define DBNOTIFIER_H

#include <memory>
#include <QDebug>
#include <QObject>

class DBNotifier {
public:
    typedef std::shared_ptr<DBNotifier> Ptr;
    DBNotifier() {}
    virtual ~DBNotifier() {}
    virtual void onDBNotify() = 0;
};

#endif // DBNOTIFIER_H
