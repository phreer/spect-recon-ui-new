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
    ReconThread(QObject *parent);
    ReconThread(ReconThread&& thread): spect_param_(std::move(thread.spect_param_)), progress_(thread.progress_) {}
    int GetProgress() const { return progress_; }
    void SetParameter(const ReconTaskParameter& recon_task_param);
signals:
    void Progress();
protected:
    void run() override;

private:
    void Reconstruct();
    SPECTParam spect_param_;
    int progress_;
};

#endif // RECONTHREAD_H
