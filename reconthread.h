#ifndef RECONTHREAD_H
#define RECONTHREAD_H

#include <QThread>
#include <QVector>

#include "spect.h"
#include "recontaskparameter.h"

class ReconThread : public QThread
{
    Q_OBJECT
public:
    ReconThread(QObject *parent,
                const QVector<ReconTaskParameter> &reconTaskParamList,
                int startTaskIndex);
    int getTaskID() const {
        return _startTaskIndex + _taskIndexOffset;
    }
    int getProcess() const {
        return _process;
    }
signals:
    void milestone(int taskId, int process);

protected:
    void run() override;

private:
    void reconstruct(SPECTProject &spectProject, const ReconTaskParameter &param);
    QVector<ReconTaskParameter> _reconTaskParamList;
    int _startTaskIndex;
    int _process;
    int _taskIndexOffset;
    QString _modelPath;
};

#endif // RECONTHREAD_H
