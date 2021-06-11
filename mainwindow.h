#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QListWidgetItem>
#include <QDir>
#include <reconthread.h>

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

    void updateTaskStatus(int taskID, int process);
private:
    void ReleaseData();
    void UpdateParameterDisplay(const ReconTaskParameter &param);
    void SetEditable(bool editible);
    void ParamChanged();
    void CreateNewTask();


    Ui::MainWindow *ui;
    QVector<ReconTaskParameter> params_;
    // 0 for TO_BE_CONDUCTED, 1 for COMPLETED, 2 for RUNNING
    enum class TaskStatus {
        READ_TO_RUN,
        COMPLETED,
        RUNNING
    };

    QVector<TaskStatus> taskStatus_;

    static QString baseDir_;
    QString currentDir_;
    std::shared_ptr<ReconThread> thread_;
    int CurrentTaskIndex() const;
    void RunTask(int taskID);
    void RunAllTask();
};
#endif // MAINWINDOW_H
