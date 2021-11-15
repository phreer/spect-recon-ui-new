#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QListWidgetItem>
#include <QDir>
#include <QProgressBar>
#include <QTimer>

#include <recontask.h>

#include "recontaskparameter.h"

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

//    void updateTaskStatus(int taskID, int process);
    void on_horizontalScrollBarSinogram_valueChanged(int value);

    void on_horizontalScrollBarProjection_valueChanged(int value);

    void UpdateStatusBar() {
        UpdateStatusBar_();
    }
private:
    void ReleaseData_();
    void UpdateParameterDisplay_(const ReconTaskParameter &param);
    void SetEditable_(bool editable);
    void ParamChanged_();
    void CreateNewTask_();
    void UpdateProjection_(int index);
    void UpdateSinogram_(int index);
    void UpdateStatusBar_();
    void DrawProjectionLine_();
    int CurrentTaskIndex_() const;
    int GetTaskCount_() const {
        return task_array_.size();
    }
    ReconTask& CurrentTask_() {
        return task_array_[CurrentTaskIndex_()];
    }
    const ReconTaskParameter& GetParameter_(int index) const {
        assert (index < GetTaskCount_() && index >= 0);
        return task_array_[index].GetParameter();
    }
    ReconTaskParameter& GetParameter_(int index) {
        assert (index < GetTaskCount_() && index >= 0);
        return task_array_[index].GetParameter();
    }
    const ReconTaskParameter& GetCurrentParameter_() const {
        return task_array_[CurrentTaskIndex_()].GetParameter();
    }
    ReconTaskParameter& GetCurrentParameter_() {
        return task_array_[CurrentTaskIndex_()].GetParameter();
    }
    void RunTask_(int task_index);
    void RunAllTask_();

    Ui::MainWindow *ui;
    std::vector<ReconTask> task_array_;
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
