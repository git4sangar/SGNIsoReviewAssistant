//sgn
#include <iostream>
#include <QLabel>
#include "statusupdater.h"

StatusUpdater::StatusUpdater(QLabel *lbl)
    : mpLblStatus(lbl)
    , mpStatusTimer(nullptr)
{
    mpStatusTimer   = new QTimer();
    connect(mpStatusTimer, SIGNAL(timeout()), this, SLOT(clearStatus()));
}

void StatusUpdater::updateStatus(const QString& pStatus, int delay) {
    mpStatusTimer->stop();
    mpLblStatus->setText(pStatus);
    mpStatusTimer->start(delay);
}

void StatusUpdater::clearStatus() {
    mpStatusTimer->stop();
    mpLblStatus->clear();
}
