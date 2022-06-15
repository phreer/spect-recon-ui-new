#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QListWidgetItem>
#include <QDir>
#include <QProgressBar>
#include <QTimer>
#include <QMessageBox>

#include <ui_mainwindow.h>
#include <recontask.h>
#include "recontaskparameter.h"
#include "resultdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void CheckStatus()
    {
        UpdateStatusBar_();
        if (GetTaskCount_()) {
            if (CurrentTask_().GetStatus() == ReconTask::Status::kCompleted) {
                ui->pushButtonShowResult->setEnabled(true);
            }
            if (CurrentTask_().GetStatus() == ReconTask::Status::kRunning) {
                ui->pushButtonRun->setEnabled(false);
            } else {
                ui->pushButtonRun->setEnabled(true);
            }
        }
    }
    void TaskCompleted(ReconTask *recon_task);

    void on_actionExit_triggered();

    void on_actionNew_Task_triggered();

    void on_listWidgetTask_currentRowChanged(int currentRow);

    void on_checkBoxScatterMap_stateChanged(int arg1);

    void on_lineEditTaskName_textChanged(const QString &arg1);

    void on_lineEditTaskName_editingFinished();

    void on_pushButtonSave_clicked();

    void on_pushButtonSelectOutputDir_clicked();

    void on_pushButtonSelectSysMat_clicked();

    void on_lineEditSysMat_textChanged(const QString &arg1);

    void on_lineEditSinogram_textChanged(const QString &arg1);

    void on_lineEditOutputDir_textChanged(const QString &arg1);

    void on_lineEditNumIters_editingFinished();

    void on_lineEditNumDualIters_editingFinished();

    void on_lineEditLambda_editingFinished();

    void on_lineEditGamma_editingFinished();

    void on_pushButtonSelectSinogram_clicked();

    void on_lineEditScatterMap_textChanged(const QString &arg1);

    void on_pushButtonSelectScatterMap_clicked();

    void on_lineEditScatterCoeff_editingFinished();

    void on_pushButtonRun_clicked();

    void on_actionImport_Tasks_triggered();

    void on_comboBoxIterator_currentTextChanged(const QString &arg1);

    void on_checkBoxRestore_stateChanged(int arg1);

    void on_actionRun_All_Tasks_triggered();

    void on_horizontalScrollBarSinogram_valueChanged(int value);

    void on_horizontalScrollBarProjection_valueChanged(int value);

    void on_pushButtonShowResult_clicked();

    void on_comboBoxProjectionIndex_currentIndexChanged(int index);

    void on_comboBoxSinogramIndex_currentIndexChanged(int index);

    void on_comboBoxDataType_currentTextChanged(const QString &arg1);

    void on_comboBoxSinogramFormat_currentTextChanged(const QString &arg1);

    void on_lineEditNumSlices_textEdited(const QString &arg1);

    void on_lineEditNumAngles_textEdited(const QString &arg1);

    void on_lineEditScatterCoeff_textEdited(const QString &arg1);

    void on_comboBoxResult_activated(int index);

    void on_horizontalScrollBarResult_valueChanged(int value);

    void on_comboBoxFilter_currentTextChanged(const QString &arg1);

    void on_lineEditNumDetectors_textEdited(const QString &arg1);

private:
    void ReleaseData_();
    void UpdateParameterDisplay_();
    void SetEditable_(bool editable);
    void ParamChanged_();
    void CreateNewTask_();
    void UpdateProjection_(int index);
    void UpdateSinogram_(int index);
    void UpdateStatusBar_();
    void DrawProjectionLine_();
    int CurrentTaskIndex_() const;
    int GetTaskCount_() const {
        return static_cast<int>(task_array_.size());
    }
    void ShowMessageBox(const QString& message) {
        auto box = new QMessageBox(this);
        box->setText(message);
        box->exec();
    }
    void UpdateComboBoxProjectionIndex_() {
        if (GetTaskCount_() == 0) return;
        auto comboBox = ui->comboBoxProjectionIndex;
        ReconTask& current_task = CurrentTask_();
        ReconTaskParameter& param = current_task.GetParameter();
        int num_angles = param.num_angles;
        if (num_angles == 0 || comboBox->count() == num_angles) return;
        int old_index = param.index_projection;
        comboBox->clear();
        for (int i = 0; i < num_angles; ++i) {
            comboBox->addItem(QString::number(i));
        }
        std::cout << "param.index_projection: " << param.index_projection << std::endl;
        if (old_index < comboBox->count()) {
            comboBox->setCurrentIndex(param.index_projection);
        }
        comboBox->setEnabled(true);
    }
    void UpdateComboBoxSinogramIndex_() {
        if (GetTaskCount_() == 0) return;
        auto comboBox = ui->comboBoxSinogramIndex;

        ReconTask& current_task = CurrentTask_();
        ReconTaskParameter& param = current_task.GetParameter();
        int num_slices = param.num_slices;
        if (num_slices == 0 || comboBox->count() == num_slices) return;
        int old_index = param.index_sinogram;
        comboBox->clear();
        for (int i = 0; i < num_slices; ++i) {
            comboBox->addItem(QString::number(i));
        }
        std::cout << "param.index_sinogram: " << param.index_sinogram << std::endl;
        if (old_index < comboBox->count()) comboBox->setCurrentIndex(old_index);
        comboBox->setEnabled(true);
    }
    ReconTask& CurrentTask_() {
        return *task_array_[CurrentTaskIndex_()];
    }
    const ReconTaskParameter& GetParameter_(int index) const {
        assert (index < GetTaskCount_() && index >= 0);
        return task_array_[index]->GetParameter();
    }
    ReconTaskParameter& GetParameter_(int index) {
        assert (index < GetTaskCount_() && index >= 0);
        return task_array_[index]->GetParameter();
    }
    const ReconTaskParameter& GetCurrentParameter_() const {
        return task_array_[CurrentTaskIndex_()]->GetParameter();
    }
    ReconTaskParameter& GetCurrentParameter_() {
        return task_array_[CurrentTaskIndex_()]->GetParameter();
    }
    void RunTask_(int task_index);
    void RunAllTask_();

    Ui::MainWindow *ui;
    std::vector<ReconTask* > task_array_;
    // 0 for TO_BE_CONDUCTED, 1 for COMPLETED, 2 for RUNNING
    enum class TaskStatus {
        READ_TO_RUN,
        COMPLETED,
        RUNNING
    };

    static QString base_dir_;
    QString current_dir_;
    QProgressBar *progress_bar_;
    QTimer *timer_;
};
#endif // MAINWINDOW_H
