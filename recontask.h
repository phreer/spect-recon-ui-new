#ifndef RECONTASK_H
#define RECONTASK_H

#include <QObject>
#include <QPixmap>

#include "utils.h"
#include "recontaskparameter.h"
#include "reconthread.h"

class ReconTask: public QObject
{
    Q_OBJECT
public:
    ReconTask(QObject *parent):
        QObject(parent),
        parameter_(),
        thread_(new ReconThread(this))
    {
        connect(thread_, SIGNAL(finished()), this, SLOT(OnThreadFinished()));
    }
    enum class Status {
        kInit, // Sinograms and projections have not been loaded yet.
        kLoaded, // Sinograms and projections have been loaded successfully.
        kRunning, // Task is running.
        kCompleted, // Reconstruction completed successfully.
        kFailedToReconstruct, // Error occurred in reconstruction.
    };

    const ReconTaskParameter& GetParameter() const {
        return parameter_;
    }
    ReconTaskParameter& GetParameter() {
        return parameter_;
    }
    void Start() {
        if (thread_ && !thread_->isRunning()) {
            pixmap_result_array_.clear();
            result_iter_index_array_.clear();

            thread_->SetParameter(parameter_);
            thread_->start();
        }
    }
    Status GetStatus() const {
        if (thread_ == nullptr) return Status::kFailedToReconstruct;
        if (thread_->isFinished()) {
            if (thread_->GetProgress() == 100) {
                return Status::kCompleted;
            } else {
                return Status::kFailedToReconstruct;
            }
        } else if (thread_->isRunning()){
           return Status::kRunning;
        } else if (pixmap_sinogram_array_.size() > 0) {
            return Status::kLoaded;
        } else {
            return Status::kInit;
        }
    }
    int GetProgress() const {
        return thread_->GetProgress();
    }
    const QString& GetTaskName() const {
        return parameter_.task_name;
    }
    const QPixmap& GetPixmapResult(int index) const {
        if (GetStatus() != Status::kCompleted) {
            std::cerr << "Trying to get result pixmap from a non-completed reconstruction task!\n";
            exit(-1);
        }
        return pixmap_result_array_[index];
    }
    const std::vector<int> GetResultIterIndexArray() const {
        return result_iter_index_array_;
    }

    void SetPixmapSinogramArray(std::vector<QPixmap>&& pixmap_sinogram_array) {
        pixmap_sinogram_array_ = pixmap_sinogram_array;
    }
    const QPixmap& GetPixmapSinogram(int index) const {
        return pixmap_sinogram_array_[index];
    }
    const QPixmap& GetCurrentPixmapSinogram() const {
        return GetPixmapSinogram(parameter_.index_sinogram);
    }

    void SetPixmapProjectionArray(std::vector<QPixmap>&& pixmap_projection_array) {
        pixmap_projection_array_ = pixmap_projection_array;
    }
    const QPixmap& GetPixmapProjection(int index) const {
        return pixmap_projection_array_[index];
    }
    const QPixmap& GetCurrentPixmapProjection() const {
        return GetPixmapProjection(parameter_.index_projection);
    }
    bool GetLoadedFlag() {
        return parameter_.sinogram.shape().size() > 0;
    }
    const std::vector<Tensor>& GetResultArray() const {
        return result_array_;
    }
    std::vector<Tensor>& GetResultArray() {
        return result_array_;
    }
protected:
signals:
    void TaskCompleted(ReconTask *recon_task);

private slots:
    void OnThreadFinished()
    {
        if (thread_ == nullptr) return;
        parameter_.reconstructed_tomographs = thread_->GetResultArray();
        result_iter_index_array_ = thread_->GetResultIterIndexArray();
        result_array_ = thread_->GetResultArray();
        for (auto& tensor: result_array_) {
            pixmap_result_array_.push_back(GetPixmapFromTensor2D(tensor));
        }
        emit(TaskCompleted(this));
    }
private:
    ReconTaskParameter parameter_;
    ReconThread *thread_;

    // For displaying
    std::vector<Tensor> result_array_;
    std::vector<QPixmap> pixmap_result_array_;
    std::vector<int> result_iter_index_array_;

    std::vector<QPixmap> pixmap_sinogram_array_;
    std::vector<QPixmap> pixmap_projection_array_;
    std::vector<QPixmap> restored_sinogram_array_;
};

#endif // RECONTASK_H
