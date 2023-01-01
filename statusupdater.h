//sgn
#ifndef STATUSUPDATER_H
#define STATUSUPDATER_H

#include <iostream>
#include <QString>
#include <QObject>
#include <QLabel>
#include <QTimer>

class StatusUpdater : public QObject {
    Q_OBJECT
    QLabel  *mpLblStatus;
    QTimer  *mpStatusTimer;

private slots:
    void clearStatus();

public:
    StatusUpdater(QLabel *pLabel);
    void updateStatus(const QString& pStatus, int delay = 5000);
};

#endif // STATUSUPDATER_H
