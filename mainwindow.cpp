#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QRegExp>
#include <QMessageBox>
#include <QDir>
#include <QFileDialog>

#include "error_code.h"
#include "reconthread.h"
#include "sinogramfilereader.h"

QString MainWindow::base_dir_ = QDir::home().filePath("spect-recon-data");

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QDir::home().mkdir(".spect-recon-data");
    current_dir_ = base_dir_;
    progress_bar_ = new QProgressBar(this);
    progress_bar_->setRange(0, 100);
    progress_bar_->setValue(0);
    ui->statusbar->addPermanentWidget(progress_bar_, 0);

    timer_ = new QTimer(this);
    timer_->setInterval(1000);
    connect(timer_, SIGNAL(timeout()), this, SLOT(UpdateStatusBar()));
    timer_->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ReleaseData_()
{
    qDebug() << __FUNCTION__ << " called.\n";
}

int MainWindow::CurrentTaskIndex_() const
{
    return ui->listWidgetTask->currentRow();
}

void MainWindow::on_actionExit_triggered()
{
    ReleaseData_();
    destroy();
    exit(0);
}

void MainWindow::ParamChanged_()
{
    ui->pushButtonSave->setEnabled(true);
    if (CurrentTask_().GetStatus() != ReconTask::Status::kRunning) {
        ui->pushButtonRun->setEnabled(true);
        ui->actionRun_All_Tasks->setEnabled(true);
    }
}

void MainWindow::SetEditable_(bool editable)
{
    ui->lineEditTaskName->setEnabled(editable);
    ui->lineEditOutputDir->setEnabled(editable);
    ui->lineEditSysMat->setEnabled(editable);
    ui->lineEditSinogram->setEnabled(editable);
    ui->lineEditGamma->setEnabled(editable);
    ui->lineEditLambda->setEnabled(editable);
    ui->lineEditNumIters->setEnabled(editable);
    ui->lineEditNumDualIters->setEnabled(editable);

    ui->comboBoxIterator->setEnabled(editable);

    ui->checkBoxRestore->setEnabled(editable);
    ui->checkBoxScatterMap->setEnabled(editable);

    ui->pushButtonSelectSysMat->setEnabled(editable);
    ui->pushButtonSelectSinogram->setEnabled(editable);
    ui->pushButtonRun->setEnabled(editable);
    ui->pushButtonSave->setEnabled(editable);
    ui->pushButtonSelectOutputDir->setEnabled(editable);

    ui->comboBoxDataType->setEnabled(editable);
    ui->comboBoxSinogramFormat->setEnabled(editable);
    ui->horizontalScrollBarProjection->setEnabled(editable);
    ui->horizontalScrollBarSinogram->setEnabled(editable);
    ui->lineEditNumSlices->setEnabled(editable);
}

void MainWindow::UpdateParameterDisplay_(const ReconTaskParameter &param)
{
    SetEditable_(true);
    ui->lineEditTaskName->setText(param.task_name);
    ui->lineEditSysMat->setText(param.path_sysmat);
    ui->lineEditSinogram->setText(param.path_sinogram);
    ui->lineEditGamma->setText(QString::number(param.gamma));
    ui->lineEditLambda->setText(QString::number(param.lambda));
    ui->lineEditNumIters->setText(QString::number(param.num_iters));
    ui->lineEditNumDualIters->setText(QString::number(param.num_dual_iters));
    ui->lineEditScatterCoeff->setText(QString::number(param.coeff_scatter));
    ui->lineEditScatterMap->setText(param.path_scatter_map);

    if (param.use_nn) ui->checkBoxRestore->setCheckState(Qt::Checked);
    else ui->checkBoxRestore->setCheckState(Qt::Unchecked);

    if (param.use_scatter_map) ui->checkBoxScatterMap->setCheckState(Qt::Checked);
    else ui->checkBoxScatterMap->setCheckState(Qt::Unchecked);
    ui->lineEditScatterMap->setEnabled(param.use_scatter_map);
    ui->pushButtonSelectScatterMap->setEnabled(param.use_scatter_map);
    ui->lineEditOutputDir->setText(param.output_dir);

    ui->comboBoxIterator->setCurrentText(param.iterator_type);
    if (CurrentTask_().GetLoadedFlag()) {
        UpdateSinogram_(CurrentTask_().GetParameter().index_sinogram);
        UpdateProjection_(CurrentTask_().GetParameter().index_projection);
    }
    ParamChanged_();
}

void MainWindow::CreateNewTask_()
{
    ui->listWidgetTask->addItem("untitled task");
    ui->listWidgetTask->setCurrentRow(ui->listWidgetTask->count() - 1);
    ReconTask task(this);
    task_array_.emplace_back(std::move(task));
    UpdateParameterDisplay_(CurrentTask_().GetParameter());
}

void MainWindow::UpdateStatusBar_() {
    using Status = ReconTask::Status;
    QString text;
    if (GetTaskCount_() == 0) {
        text = "Create or import a task first.";
    } else {
        switch (CurrentTask_().GetStatus()) {
        case Status::kInit: {
            text = "Please set parameters properly.";
            progress_bar_->setTextVisible(false);
            break;
        }
        case Status::kCompleted: {
            text = "Reconstruction done.";
            progress_bar_->setVisible(true);
            progress_bar_->setValue(100);
            break;
        }
        case Status::kRunning: {
            text = "Running Reconstruction...";
            progress_bar_->setVisible(true);
            progress_bar_->setValue(CurrentTask_().GetProgress());
            break;
        }
        case Status::kLoaded: {
            text = "Press Run button to run task.";
            progress_bar_->setTextVisible(true);
            progress_bar_->setValue(0);
            break;
        }
        case Status::kFailedToReconstruct: {
            text = "Error occurred during reconstruction!";
            break;
        }
        }
    }
    ui->statusbar->showMessage(text);
}

void MainWindow::on_actionNew_Task_triggered()
{
    CreateNewTask_();
    UpdateStatusBar_();
}


void MainWindow::on_listWidgetTask_currentRowChanged(int currentRow)
{
    if (currentRow < GetTaskCount_()) {
        UpdateParameterDisplay_(CurrentTask_().GetParameter());
    }
}


void MainWindow::on_checkBoxScatterMap_stateChanged(int arg1)
{
    bool use_scatter_map = false;
    if (arg1 == Qt::Checked) {
        use_scatter_map = true;
    }
    ui->lineEditScatterMap->setEnabled(use_scatter_map);
    ui->pushButtonSelectScatterMap->setEnabled(use_scatter_map);
    ui->lineEditScatterCoeff->setEnabled(use_scatter_map);
    auto &parameter = CurrentTask_().GetParameter();
    parameter.use_scatter_map = use_scatter_map;

    ParamChanged_();
}

void MainWindow::on_checkBoxRestore_stateChanged(int arg1)
{
    bool use_nn = false;
    if (arg1 == Qt::Checked) {
        use_nn = true;
    }
    auto& param = GetCurrentParameter_();
    param.use_nn = use_nn;
    ParamChanged_();
}

void MainWindow::on_lineEditTaskName_textChanged(const QString &arg1)
{
    ui->listWidgetTask->currentItem()->setText(arg1);
    ParamChanged_();
}


void MainWindow::on_lineEditTaskName_editingFinished()
{
    QString task_name = ui->lineEditTaskName->text();
    if (task_name.contains(QRegExp("\\s"))) {
        auto box = new QMessageBox(this);
        box->setText("Whitespace in task name is not allowed.");
        box->exec();
        ui->lineEditTaskName->setFocus();
        return;
    }
    auto& param = GetCurrentParameter_();
    param.task_name = task_name;
    ParamChanged_();
}


void MainWindow::on_pushButtonSave_clicked()
{
    auto &param = GetCurrentParameter_();
    QString filename = param.task_name + ".task";
    QString outputPath = QDir(param.output_dir).filePath(filename);
    if (QDir(outputPath).isRelative()) {
        QString baseDir = QDir::home().filePath("spect-recon");
        QDir(baseDir).mkpath(param.output_dir);
        outputPath = QDir(baseDir).filePath(outputPath);
    }
    qDebug() << "Start saving to " << outputPath << "..." << endl;
    int r = GetCurrentParameter_().ToProtobufFilePath(outputPath);

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
                                                     current_dir_);
    if (path.size()) ui->lineEditOutputDir->setText(path);
    if (old != path) ui->pushButtonSave->setEnabled(true);
    ParamChanged_();
}


void MainWindow::on_pushButtonSelectSysMat_clicked()
{
    QString old = ui->lineEditSysMat->text();
    QString path = QFileDialog::getOpenFileName(this,
                                                tr("Select system matrix."),
                                                current_dir_,
                                                tr("System Matrix File (*.aij *.sysmat)"));
    if (path.size()) {
        ui->lineEditSysMat->setText(path);
        if (old != path) ui->pushButtonSave->setEnabled(true);
        current_dir_ = QFileInfo(path).absoluteDir().path();
    }
    ParamChanged_();
}


void MainWindow::on_lineEditSysMat_textChanged(const QString &arg1)
{
    auto& param = GetCurrentParameter_();
    param.path_sysmat = arg1;
    ParamChanged_();
}


void MainWindow::on_lineEditOutputDir_textChanged(const QString &arg1)
{
    auto& param = GetCurrentParameter_();
    param.output_dir = arg1;
    ParamChanged_();
}

void MainWindow::on_lineEditSinogram_textChanged(const QString &arg1)
{
    auto& param = GetCurrentParameter_();
    param.path_sinogram = arg1;
    ParamChanged_();
}

void MainWindow::on_lineEditNumIters_editingFinished()
{
    bool is_integer = false;
    uint num_iters = ui->lineEditNumIters->text().toUInt(&is_integer);
    if (!is_integer || num_iters == 0) {
        auto box = new QMessageBox(this);
        box->setText("Positive Integer is required.");
        box->exec();
        ui->lineEditNumIters->setFocus();
        return;
    }
    auto& param = GetCurrentParameter_();
    param.num_iters = num_iters;
    ui->pushButtonSave->setEnabled(true);
    ParamChanged_();
}


void MainWindow::on_lineEditNumDualIters_editingFinished()
{
    bool is_integer = false;
    uint num_dual_iters = ui->lineEditNumDualIters->text().toUInt(&is_integer);
    if (!is_integer || num_dual_iters == 0) {
        auto box = new QMessageBox(this);
        box->setText("Positive Integer is required.");
        box->exec();
        ui->lineEditNumDualIters->setFocus();
        return;
    }
    auto& param = GetCurrentParameter_();
    param.num_dual_iters = num_dual_iters;
    qDebug() << __FUNCTION__ << "numDualIters: " << num_dual_iters << endl;
    ui->pushButtonSave->setEnabled(true);
    ParamChanged_();
}


void MainWindow::on_lineEditLambda_editingFinished()
{
    bool is_number = false;
    double lambda = ui->lineEditLambda->text().toDouble(&is_number);
    if (!is_number) {
        auto box = new QMessageBox(this);
        box->setText("Number is required.");
        box->exec();
        ui->lineEditLambda->setFocus();
        return;
    }
    auto& param = GetCurrentParameter_();
    param.lambda = lambda;
    ui->pushButtonSave->setEnabled(true);
    ParamChanged_();
}


void MainWindow::on_lineEditGamma_editingFinished()
{
    bool is_number = false;
    double gamma = ui->lineEditGamma->text().toDouble(&is_number);
    if (!is_number) {
        auto box = new QMessageBox(this);
        box->setText("Number is required.");
        box->exec();
        ui->lineEditGamma->setFocus();
        return;
    }
    auto& param = GetCurrentParameter_();
    param.gamma = gamma;
    ui->pushButtonSave->setEnabled(true);
    ParamChanged_();
}


void MainWindow::on_pushButtonSelectSinogram_clicked()
{
    SinogramFileReader::FileFormat file_format = SinogramFileReader::FileFormat::kDicom;
    auto file_format_s = ui->comboBoxSinogramFormat->currentText();
    QString allowed_format("Raw Files (*.a00 *.proj)");
    if (file_format_s == "DICOM") {
        file_format = SinogramFileReader::FileFormat::kDicom;
        allowed_format = "DICOM files (*.dcm)";
    } else if (file_format_s == "Raw (Sinograms)") {
        file_format = SinogramFileReader::FileFormat::kRawSinogram;
        allowed_format = "Raw Files (*.a00 *.sin)";
    } else if (file_format_s == "Raw (Projections)") {
        file_format = SinogramFileReader::FileFormat::kRawProjection;
    } else {
        std::cerr << "Invalid file_format: " << file_format_s.toStdString() << std::endl;
        exit(-1);
    }

    QString old = ui->lineEditSinogram->text();
    QString path = QFileDialog::getOpenFileName(this,
                                                tr("Select a input data to reconstruct."),
                                                current_dir_,
                                                tr(allowed_format.toStdString().c_str()));
    if (path.size()) {
        ui->lineEditSinogram->setText(path);
        ui->pushButtonSave->setEnabled(true);

        const QString& data_type_s = ui->comboBoxDataType->currentText();
        SinogramFileReader::FileDataType file_data_type = SinogramFileReader::FileDataType::kFloat32;
        if (data_type_s == "float32") {
            file_data_type = SinogramFileReader::FileDataType::kFloat32;
        } else if (data_type_s == "float64") {
            file_data_type = SinogramFileReader::FileDataType::kFloat64;
        } else if (data_type_s == "int8") {
            file_data_type = SinogramFileReader::FileDataType::kInt8;
        } else if (data_type_s == "uint8") {
            file_data_type = SinogramFileReader::FileDataType::kUInt8;
        } else if (data_type_s == "int16") {
            file_data_type = SinogramFileReader::FileDataType::kInt16;
        } else if (data_type_s == "uint16") {
            file_data_type = SinogramFileReader::FileDataType::kUInt16;
        } else if (data_type_s == "int32") {
            file_data_type = SinogramFileReader::FileDataType::kInt32;
        } else if (data_type_s == "uint32") {
            file_data_type = SinogramFileReader::FileDataType::kUInt32;
        } else if (data_type_s == "int64") {
            file_data_type = SinogramFileReader::FileDataType::kInt64;
        } else if (data_type_s == "uint64") {
            file_data_type = SinogramFileReader::FileDataType::kUInt64;
        } else {
            std::cerr << "Invalid data_type: " << data_type_s.toStdString() << std::endl;
            exit(-1);
        }

        bool is_integer = false;
        uint num_slices = ui->lineEditNumSlices->text().toUInt(&is_integer);
        if (!is_integer) {
            auto box = new QMessageBox(this);
            box->setText("Positive Integer is required.");
            box->exec();
            ui->lineEditNumDualIters->setFocus();
            return;
        }
        SinogramFileReader reader(path.toStdString(), file_format, num_slices, file_data_type);
        ui->plainTextEditSinogramInfo->appendPlainText(QString::fromStdString(reader.GetSinogramInfo().GetInfoString()));
        CurrentTask_().SetPixmapSinogramArray(reader.GetSinogramPixmap());
        CurrentTask_().SetPixmapProjectionArray(reader.GetProjectionPixmap());

        auto& recon_param = CurrentTask_().GetParameter();
        recon_param.sinogram = reader.GetSinogram();
        recon_param.projection = reader.GetProjection();

        const auto& shape_sinogram = recon_param.sinogram.shape();
        assert (shape_sinogram.size() == 3);
        ui->horizontalScrollBarSinogram->setMinimum(0);
        ui->horizontalScrollBarSinogram->setMaximum(shape_sinogram[0] - 1);

        const auto& shape_projection = recon_param.projection.shape();
        assert (shape_projection.size() == 3);
        ui->horizontalScrollBarProjection->setMinimum(0);
        ui->horizontalScrollBarProjection->setMaximum(shape_projection[0] - 1);

        CurrentTask_().SetLoadedFlag(true);
        UpdateStatusBar_();
        ui->labelSinogramImage->setPixmap(CurrentTask_().GetCurrentPixmapSinogram());
        ui->labelProjectionImage->setPixmap(CurrentTask_().GetCurrentPixmapProjection());
        DrawProjectionLine_();
        current_dir_ = QFileInfo(path).absoluteDir().path();
    }
}

void MainWindow::UpdateSinogram_(int value)
{
    CurrentTask_().GetParameter().index_sinogram = value;
    ui->labelSinogramImage->setPixmap(CurrentTask_().GetCurrentPixmapSinogram());
}

void MainWindow::UpdateProjection_(int index)
{
    ReconTask& current_task = CurrentTask_();
    CurrentTask_().GetParameter().index_projection = index;
    ui->labelProjectionImage->setPixmap(current_task.GetCurrentPixmapProjection());
}

void MainWindow::on_lineEditScatterMap_textChanged(const QString &arg1)
{
    auto& param = GetCurrentParameter_();
    param.path_scatter_map = arg1;
    ui->pushButtonSave->setEnabled(true);
}


void MainWindow::on_pushButtonSelectScatterMap_clicked()
{
    QString old = ui->lineEditScatterMap->text();
    QString path = QFileDialog::getOpenFileName(this,
                                                tr("Select a scatter map."),
                                                current_dir_,
                                                tr("Scatter Map File (*.sca)"));
    if (path.size()) ui->lineEditScatterMap->setText(path);
    if (old != path) ui->pushButtonSave->setEnabled(true);
}


void MainWindow::on_lineEditScatterCoeff_editingFinished()
{
    bool is_number = false;
    double scatter_coeff = ui->lineEditScatterCoeff->text().toDouble(&is_number);
    if (!is_number) {
        auto box = new QMessageBox(this);
        box->setText("Number is required.");
        box->exec();
        ui->lineEditScatterCoeff->setFocus();
        return;
    }
    auto& param = GetCurrentParameter_();
    param.coeff_scatter = scatter_coeff;
    ui->pushButtonSave->setEnabled(true);
}


void MainWindow::on_pushButtonRun_clicked()
{
    ui->pushButtonRun->setEnabled(false);
    ui->actionRun_All_Tasks->setEnabled(false);
    RunTask_(CurrentTaskIndex_());
}


void MainWindow::on_actionImport_Tasks_triggered()
{
    if (ui->listWidgetTask->count() == 0) CreateNewTask_();
    QString path = QFileDialog::getOpenFileName(this,
                                                tr("Select s task file."),
                                                current_dir_,
                                                tr("TASK (*.task)"));
    auto &param = GetCurrentParameter_();
    param.FromProtobufFilePath(path);
    UpdateParameterDisplay_(param);
}

void MainWindow::RunTask_(int task_index)
{
    QVector<ReconTaskParameter> paramList;
    assert (task_index < GetTaskCount_());
    task_array_[task_index].Start();
    UpdateStatusBar_();
}

void MainWindow::on_comboBoxIterator_currentTextChanged(const QString &arg1)
{
    auto& param = GetCurrentParameter_();
    param.iterator_type = arg1;
    ParamChanged_();
}


void MainWindow::RunAllTask_()
{
    for (auto& task: task_array_) {
        task.Start();
    }
}


void MainWindow::on_actionRun_All_Tasks_triggered()
{
    ui->pushButtonRun->setEnabled(false);
    ui->actionRun_All_Tasks->setEnabled(false);
    RunAllTask_();
}

void MainWindow::on_horizontalScrollBarSinogram_valueChanged(int value)
{
    UpdateSinogram_(value);
    DrawProjectionLine_();
    ParamChanged_();
}


void MainWindow::on_horizontalScrollBarProjection_valueChanged(int value)
{
    UpdateProjection_(value);
    DrawProjectionLine_();
    ParamChanged_();
}

void MainWindow::DrawProjectionLine_() {
    ReconTask& current_task = CurrentTask_();
    int num_detectors = current_task.GetParameter().sinogram.shape()[2];
    QImage projection_image_buffer_ = current_task.GetCurrentPixmapProjection().toImage();
    int index_sinogram = current_task.GetParameter().index_sinogram;
    for (int i = 0; i < num_detectors; ++i) {
        projection_image_buffer_.setPixel(i, index_sinogram, qRgb(255, 0, 0));
    }
    ui->labelProjectionImage->setPixmap(QPixmap::fromImage(projection_image_buffer_));
}
