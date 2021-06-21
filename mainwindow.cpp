#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QRegExp>
#include <QMessageBox>
#include <QDir>
#include <QFileDialog>

#include "error_code.h"
#include "reconthread.h"

QString MainWindow::baseDir_ = QDir::home().filePath("spect-recon");

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QDir::home().mkdir("spect-recon");
    currentDir_ = baseDir_;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ReleaseData()
{
    qDebug() << __FUNCTION__ << " called.\n";
}

int MainWindow::CurrentTaskIndex() const
{
    return ui->listWidgetTask->currentRow();
}

void MainWindow::on_actionExit_triggered()
{
    ReleaseData();
    destroy();
    exit(0);
}

void MainWindow::ParamChanged()
{
    ui->pushButtonSave->setEnabled(true);
    ui->pushButtonRun->setEnabled(true);
    ui->actionRun_All_Tasks->setEnabled(true);
}

void MainWindow::SetEditable(bool editible)
{
    ui->lineEditTaskName->setEnabled(editible);
    ui->lineEditOutputDir->setEnabled(editible);
    ui->lineEditSysMat->setEnabled(editible);
    ui->lineEditSinogram->setEnabled(editible);
    ui->lineEditGamma->setEnabled(editible);
    ui->lineEditLambda->setEnabled(editible);
    ui->lineEditNumIters->setEnabled(editible);
    ui->lineEditNumDualIters->setEnabled(editible);

    ui->comboBoxIterator->setEnabled(editible);

    ui->checkBoxRestore->setEnabled(editible);
    ui->checkBoxScatterMap->setEnabled(editible);

    ui->pushButtonSelectSysMat->setEnabled(editible);
    ui->pushButtonSelectSinogram->setEnabled(editible);
    ui->pushButtonRun->setEnabled(editible);
    ui->pushButtonSave->setEnabled(editible);
    ui->pushButtonSelectOutputDir->setEnabled(editible);
}

void MainWindow::UpdateParameterDisplay(const ReconTaskParameter &param)
{
    SetEditable(true);
    ui->lineEditTaskName->setText(param.taskName);
    ui->lineEditSysMat->setText(param.pathSysMat);
    ui->lineEditSinogram->setText(param.pathSinogram);
    ui->lineEditGamma->setText(QString::number(param.gamma));
    ui->lineEditLambda->setText(QString::number(param.lambda));
    ui->lineEditNumIters->setText(QString::number(param.numIters));
    ui->lineEditNumDualIters->setText(QString::number(param.numDualIters));
    ui->lineEditScatterCoeff->setText(QString::number(param.paramScatter));
    ui->lineEditScatterMap->setText(param.pathScatterMap);

    if (param.useNN) ui->checkBoxRestore->setCheckState(Qt::Checked);
    else ui->checkBoxRestore->setCheckState(Qt::Unchecked);

    if (param.useScatterMap) ui->checkBoxScatterMap->setCheckState(Qt::Checked);
    else ui->checkBoxScatterMap->setCheckState(Qt::Unchecked);
    ui->lineEditScatterMap->setEnabled(param.useScatterMap);
    ui->pushButtonSelectScatterMap->setEnabled(param.useScatterMap);
    ui->lineEditOutputDir->setText(param.outputDir);

    ui->comboBoxIterator->setCurrentText(param.iteratorType);
    ParamChanged();
}

void MainWindow::CreateNewTask()
{
    ui->listWidgetTask->addItem("untitled task");
    ui->listWidgetTask->setCurrentRow(ui->listWidgetTask->count() - 1);
    ReconTaskParameter param;
    params_.push_back(std::move(param));
    taskStatus_.push_back(TaskStatus::READ_TO_RUN);
    UpdateParameterDisplay(params_[CurrentTaskIndex()]);
}

void MainWindow::on_actionNew_Task_triggered()
{
    CreateNewTask();
    ui->statusbar->showMessage("Press Run to run this task.");
}


void MainWindow::on_listWidgetTask_currentRowChanged(int currentRow)
{
    if (currentRow < params_.size()) UpdateParameterDisplay(params_[currentRow]);
}


void MainWindow::on_checkBoxScatterMap_stateChanged(int arg1)
{
    bool useScatterMap = false;
    if (arg1 == Qt::Checked) {
        useScatterMap = true;
    }
    ui->lineEditScatterMap->setEnabled(useScatterMap);
    ui->pushButtonSelectScatterMap->setEnabled(useScatterMap);
    ui->lineEditScatterCoeff->setEnabled(useScatterMap);
    params_[CurrentTaskIndex()].useScatterMap = useScatterMap;
    ParamChanged();
}

void MainWindow::on_checkBoxRestore_stateChanged(int arg1)
{
    bool useNN = false;
    if (arg1 == Qt::Checked) {
        useNN = true;
    }
    params_[CurrentTaskIndex()].useNN = useNN;
}

void MainWindow::on_lineEditTaskName_textChanged(const QString &arg1)
{
    ui->listWidgetTask->currentItem()->setText(arg1);
    ParamChanged();
}


void MainWindow::on_lineEditTaskName_editingFinished()
{
    QString taskName = ui->lineEditTaskName->text();
    if (taskName.contains(QRegExp("\\s"))) {
        auto box = new QMessageBox(this);
        box->setText("Whitespace in task name is not allowed.");
        box->exec();
        ui->lineEditTaskName->setFocus();
        return;
    }
    params_[CurrentTaskIndex()].taskName = taskName;
    ParamChanged();
}


void MainWindow::on_pushButtonSave_clicked()
{
    auto &param = params_[CurrentTaskIndex()];
    QString filename = param.taskName + ".task";
    QString outputPath = QDir(param.outputDir).filePath(filename);
    if (QDir(outputPath).isRelative()) {
        QString baseDir = QDir::home().filePath("spect-recon");
        QDir(baseDir).mkpath(param.outputDir);
        outputPath = QDir(baseDir).filePath(outputPath);
    }
    qDebug() << "Start saving to " << outputPath << "..." << endl;
    int r = params_[CurrentTaskIndex()].ToProtobufFilePath(outputPath);

    if (r == EINVALID_PATH) {
        auto box = new QMessageBox(this);
        box->setText(tr("Cannot open file ") + outputPath + ".");
        box->exec();
    } else if (r == 0) {
        ui->statusbar->showMessage(tr("Task saved successfully."));
        ui->pushButtonSave->setEnabled(false);
    } else {
        auto box = new QMessageBox(this);
        box->setText(tr("Failed to save the task."));
        box->exec();
    }
}


void MainWindow::on_pushButtonSelectOutputDir_clicked()
{
    QString old = ui->lineEditOutputDir->text();
    QString path = QFileDialog::getExistingDirectory(this,
                                                     tr("Select output directory."),
                                                     currentDir_);
    if (path.size()) ui->lineEditOutputDir->setText(path);
    if (old != path) ui->pushButtonSave->setEnabled(true);
}


void MainWindow::on_pushButtonSelectSysMat_clicked()
{
    QString old = ui->lineEditSysMat->text();
    QString path = QFileDialog::getOpenFileName(this,
                                                tr("Select system matrix."),
                                                currentDir_,
                                                tr("System Matrix File (*.aij *.sysmat)"));
    if (path.size()) {
        ui->lineEditSysMat->setText(path);
        if (old != path) ui->pushButtonSave->setEnabled(true);
        currentDir_ = QFileInfo(path).absoluteDir().path();
    }
}


void MainWindow::on_lineEditSysMat_textChanged(const QString &arg1)
{
    params_[CurrentTaskIndex()].pathSysMat = arg1;
    ParamChanged();
}


void MainWindow::on_lineEditOutputDir_textChanged(const QString &arg1)
{
    params_[CurrentTaskIndex()].outputDir = arg1;
    ParamChanged();
}

void MainWindow::on_lineEditSinogram_textChanged(const QString &arg1)
{
    params_[CurrentTaskIndex()].pathSinogram = arg1;
    ParamChanged();
}

void MainWindow::on_lineEditNumIters_editingFinished()
{
    bool isInteger = false;
    uint numIters = ui->lineEditNumIters->text().toUInt(&isInteger);
    if (!isInteger) {
        auto box = new QMessageBox(this);
        box->setText("Integer is required.");
        box->exec();
        ui->lineEditNumIters->setFocus();
        return;
    }
    params_[CurrentTaskIndex()].numIters = numIters;
    ui->pushButtonSave->setEnabled(true);
}


void MainWindow::on_lineEditNumDualIters_editingFinished()
{
    bool isInteger = false;
    uint numDualIters = ui->lineEditNumDualIters->text().toUInt(&isInteger);
    if (!isInteger) {
        auto box = new QMessageBox(this);
        box->setText("Integer is required.");
        box->exec();
        ui->lineEditNumDualIters->setFocus();
        return;
    }
    params_[CurrentTaskIndex()].numDualIters = numDualIters;
    qDebug() << __FUNCTION__ << "numDualIters: " << numDualIters << endl;
    ui->pushButtonSave->setEnabled(true);
}


void MainWindow::on_lineEditLambda_editingFinished()
{
    bool isNumber = false;
    double lambda = ui->lineEditLambda->text().toDouble(&isNumber);
    if (!isNumber) {
        auto box = new QMessageBox(this);
        box->setText("Number is required.");
        box->exec();
        ui->lineEditLambda->setFocus();
        return;
    }
    params_[CurrentTaskIndex()].lambda = lambda;
    ui->pushButtonSave->setEnabled(true);
}


void MainWindow::on_lineEditGamma_editingFinished()
{
    bool isNumber = false;
    double gamma = ui->lineEditGamma->text().toDouble(&isNumber);
    if (!isNumber) {
        auto box = new QMessageBox(this);
        box->setText("Number is required.");
        box->exec();
        ui->lineEditGamma->setFocus();
        return;
    }
    params_[CurrentTaskIndex()].gamma = gamma;
    ui->pushButtonSave->setEnabled(true);
}


void MainWindow::on_pushButtonSelectSinogram_clicked()
{
    QString old = ui->lineEditSinogram->text();
    QString path = QFileDialog::getOpenFileName(this,
                                                tr("Select a sinogram."),
                                                currentDir_,
                                                tr("Sinogram File (*.a00 *.sin)"));
    if (path.size()) {
        ui->lineEditSinogram->setText(path);
        if (old != path) ui->pushButtonSave->setEnabled(true);
        currentDir_ = QFileInfo(path).absoluteDir().path();
    }
}


void MainWindow::on_lineEditScatterMap_textChanged(const QString &arg1)
{
    params_[CurrentTaskIndex()].pathScatterMap = arg1;
    ui->pushButtonSave->setEnabled(true);
}


void MainWindow::on_pushButtonSelectScatterMap_clicked()
{
    QString old = ui->lineEditScatterMap->text();
    QString path = QFileDialog::getOpenFileName(this,
                                                tr("Select a scatter map."),
                                                currentDir_,
                                                tr("Scatter Map File (*.sca)"));
    if (path.size()) ui->lineEditScatterMap->setText(path);
    if (old != path) ui->pushButtonSave->setEnabled(true);
}


void MainWindow::on_lineEditScatterCoeff_editingFinished()
{
    bool isNumber = false;
    double scatterCoeff = ui->lineEditScatterCoeff->text().toDouble(&isNumber);
    if (!isNumber) {
        auto box = new QMessageBox(this);
        box->setText("Number is required.");
        box->exec();
        ui->lineEditScatterCoeff->setFocus();
        return;
    }
    params_[CurrentTaskIndex()].paramScatter = scatterCoeff;
    ui->pushButtonSave->setEnabled(true);
}


void MainWindow::on_pushButtonRun_clicked()
{
    ui->pushButtonRun->setEnabled(false);
    ui->actionRun_All_Tasks->setEnabled(false);
    RunTask(CurrentTaskIndex());
}


void MainWindow::on_actionImport_Tasks_triggered()
{
    if (ui->listWidgetTask->count() == 0) CreateNewTask();
    QString path = QFileDialog::getOpenFileName(this,
                                                tr("Select a scatter map."),
                                                currentDir_,
                                                tr("TASK (*.task)"));
    auto &param = params_[CurrentTaskIndex()];
    param.FromProtobufFilePath(path);
    UpdateParameterDisplay(param);
}

void MainWindow::RunTask(int taskID)
{
    QVector<ReconTaskParameter> paramList;
    paramList.append(params_[taskID]);
    thread_ = make_shared<ReconThread>(this, paramList, taskID);
    thread_->start();
    taskStatus_[taskID] = TaskStatus::RUNNING;
}

void MainWindow::on_comboBoxIterator_currentTextChanged(const QString &arg1)
{
    if (params_.count() == 0) CreateNewTask();
    params_[CurrentTaskIndex()].iteratorType = arg1;
}


void MainWindow::RunAllTask()
{
    thread_ = make_shared<ReconThread>(this, params_, 0);
    thread_->start();
}


void MainWindow::on_actionRun_All_Tasks_triggered()
{
    ui->pushButtonRun->setEnabled(false);
    ui->actionRun_All_Tasks->setEnabled(false);
    if (params_.count()) RunAllTask();
}

void MainWindow::updateTaskStatus(int taskID, int process)
{
    if (process == 100) taskStatus_[taskID] = TaskStatus::COMPLETED;
}
