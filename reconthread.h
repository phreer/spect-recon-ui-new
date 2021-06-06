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
                const QVector<ReconTaskParameter> &reconTaskParamList);
    int getTaskID() const {
        return _currentTask;
    }
    int getProcess() const {
        return _process;
    }
signals:
    void milestone(uint task_id, uint process);

protected:
    void run() override;

private:
    void reconstruct(SPECTProject &spectProject, const ReconTaskParameter &param);
    QVector<ReconTaskParameter> _reconTaskParamList;
    int _process;
    int _currentTask;
    static QString model_path;
};

#endif // RECONTHREAD_H
