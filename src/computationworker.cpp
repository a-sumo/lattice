#include "computationworker.h"

ComputationWorker::ComputationWorker(QObject *parent) : QObject(parent)
{
    // Initialization code...
}

void ComputationWorker::compute()
{
    // Computation code here...
    // After computation:
    emit computationDone();
}
