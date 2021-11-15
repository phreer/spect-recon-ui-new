#ifndef RECONTASK_H
#define RECONTASK_H

#include <QObject>
#include <QVector>
#include <QPixmap>

#include "recontaskparameter.h"
#include "reconthread.h"

class ReconTask: public QObject
{
    Q_OBJECT
public:
    ReconTask(QObject *parent): parameter_(), thread_(parent) {}
    ReconTask(ReconTask&& task): parameter_(std::move(task.parameter_)), thread_(std::move(task.thread_)), recon_result_array_(std::move(task.recon_result_array_)) {}
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
        if (!thread_.isRunning()) {
            thread_.SetParameter(parameter_);
            thread_.start();
        }
    }
    Status GetStatus() const {
        if (thread_.isFinished()) {
            if (thread_.GetProgress() == 100) {
                return Status::kCompleted;
            } else {
                return Status::kFailedToReconstruct;
            }
        } else if (thread_.isRunning()){
           return Status::kRunning;
        } else if (pixmap_sinogram_array_.size() > 0) {
            return Status::kLoaded;
        } else {
            return Status::kInit;
        }
    }
    int GetProgress() const {
        return thread_.GetProgress();
    }
    const QString& GetTaskName() const {
        return parameter_.task_name;
    }
    void SetPixmapSinogramArray(QVector<QPixmap>&& pixmap_sinogram_array) {
        pixmap_sinogram_array_ = pixmap_sinogram_array;
    }
    const QPixmap& GetPixmapSinogram(int index) const {
        return pixmap_sinogram_array_[index];
    }
    const QPixmap& GetCurrentPixmapSinogram() const {
        return GetPixmapSinogram(parameter_.index_sinogram);
    }
    void SetPixmapProjectionArray(QVector<QPixmap>&& pixmap_projection_array) {
        pixmap_projection_array_ = pixmap_projection_array;
    }
    const QPixmap& GetPixmapProjection(int index) const {
        return pixmap_projection_array_[index];
    }
    const QPixmap& GetCurrentPixmapProjection() const {
        return GetPixmapProjection(parameter_.index_projection);
    }
    void SetLoadedFlag(bool loaded_flag) {
        loaded_flag_ = loaded_flag;
    }
    bool GetLoadedFlag() {
        return loaded_flag_;
    }
private:
    ReconTaskParameter parameter_;
    ReconThread thread_;

    std::vector<std::vector<Tensor> > recon_result_array_;
    // For displaying
    QVector<QPixmap> pixmap_sinogram_array_;
    QVector<QPixmap> pixmap_projection_array_;
    bool loaded_flag_ = false;
};

#endif // RECONTASK_H
