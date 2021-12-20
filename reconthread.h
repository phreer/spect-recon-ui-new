#ifndef RECONTHREAD_H
#define RECONTHREAD_H

#include <QThread>
#include <QVector>
#include <vector>

#include "spect.h"
#include "recontaskparameter.h"

class ReconThread : public QThread
{
    Q_OBJECT
public:
    ReconThread(QObject *parent);
    ReconThread(ReconThread&& thread):
        spect_param_(std::move(thread.spect_param_)),
        progress_(thread.progress_),
        result_array_(thread.result_array_)
    {}
    int GetProgress() const { return progress_; }
    void SetParameter(ReconTaskParameter& recon_task_param);
    const std::vector<Tensor>& GetResultArray() const {
        return result_array_;
    }
    std::vector<Tensor>& GetResultArray() {
        return result_array_;
    }
    const std::vector<int>& GetResultIterIndexArray() const {
        return result_iter_index_array_;
    }
signals:
    void Progress();
protected:
    void run() override;

private:
    void Reconstruct();
    SPECTParam spect_param_;
    int progress_;
    std::vector<Tensor> result_array_;
    std::vector<int> result_iter_index_array_;
};

#endif // RECONTHREAD_H
