#ifndef COMPUTATIONWORKER_H
#define COMPUTATIONWORKER_H

#include <QObject>
#include <QMutex>
#include "automaton_2d.h"

class ComputationWorker : public QObject
{
    Q_OBJECT

public:
    explicit ComputationWorker(QObject *parent = nullptr);
    void compute();

signals:
    void computationDone();

private:
    QMutex mutex;
    // other private members...
};

#endif
