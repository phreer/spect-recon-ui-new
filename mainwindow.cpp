#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QRegExp>
#include <QMessageBox>
#include <QDir>
#include <QFileDialog>

#include "global_defs.h"
#include "error_code.h"
#include "reconthread.h"
#include "sinogramfilereader.h"
#include "utils.h"

QString MainWindow::base_dir_ = kBaseDir;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QDir::home().mkdir(base_dir_);
    current_dir_ = base_dir_;
    progress_bar_ = new QProgressBar(this);
    progress_bar_->setRange(0, 100);
    progress_bar_->setValue(0);
    ui->statusbar->addPermanentWidget(progress_bar_, 0);

    timer_ = new QTimer(this);
    timer_->setInterval(1000);
    connect(timer_, SIGNAL(timeout()), this, SLOT(CheckStatus()));
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
    ui->lineEditNumAngles->setEnabled(editable);
}

QString GetLabelStringFromDataType(recontaskparameter_pb::ReconTaskParameterPB_FileDataType data_type) {
    using FileDataType = recontaskparameter_pb::ReconTaskParameterPB_FileDataType;
    QString result;
    switch (data_type) {
    case FileDataType::ReconTaskParameterPB_FileDataType_FLOAT32: {
        result = "float32";
        break;
    }
    case FileDataType::ReconTaskParameterPB_FileDataType_FLOAT64: {
        result = "float64";
        break;
    }
    default:
        std::cerr << "Use unimplemented data type." << std::endl;
        result = "unknown";
    }
    return result;
}

QString GetLabelStringFromFileFormat(recontaskparameter_pb::ReconTaskParameterPB_FileFormat format) {
    using FileFormat = recontaskparameter_pb::ReconTaskParameterPB_FileFormat;
    QString result;
    switch (format) {
    case FileFormat::ReconTaskParameterPB_FileFormat_DICOM: {
        result = "DICOM";
        break;
    }
    case FileFormat::ReconTaskParameterPB_FileFormat_RAW_PROJECTION: {
        result = "Raw (Projections)";
        break;
    }
    case FileFormat::ReconTaskParameterPB_FileFormat_RAW_SINOGRAM: {
        result = "Raw (Sinograms)";
        break;
    }
    }
    return result;
}
void MainWindow::UpdateParameterDisplay_()
{
    if (GetTaskCount_() == 0) return;

    auto& current_task = CurrentTask_();
    auto& param = current_task.GetParameter();

    SetEditable_(true);
    UpdateStatusBar_();
    ui->lineEditTaskName->setText(param.task_name);
    ui->lineEditSysMat->setText(param.path_sysmat);
    ui->lineEditSinogram->setText(param.path_sinogram);
    ui->lineEditGamma->setText(QString::number(param.gamma));
    ui->lineEditLambda->setText(QString::number(param.lambda));
    ui->lineEditNumIters->setText(QString::number(param.num_iters));
    ui->lineEditNumDualIters->setText(QString::number(param.num_dual_iters));
    ui->lineEditScatterCoeff->setText(QString::number(param.coeff_scatter));
    ui->lineEditScatterMap->setText(param.path_scatter_map);
    ui->lineEditNumAngles->setText(QString::number(param.num_angles));
    ui->lineEditNumSlices->setText((QString::number(param.num_slices)));

    if (param.use_nn) ui->checkBoxRestore->setCheckState(Qt::Checked);
    else ui->checkBoxRestore->setCheckState(Qt::Unchecked);

    if (param.use_scatter_map) ui->checkBoxScatterMap->setCheckState(Qt::Checked);
    else ui->checkBoxScatterMap->setCheckState(Qt::Unchecked);
    ui->lineEditScatterMap->setEnabled(param.use_scatter_map);
    ui->pushButtonSelectScatterMap->setEnabled(param.use_scatter_map);
    ui->lineEditOutputDir->setText(param.output_dir);
    ui->plainTextEditSinogramInfo->clear();
    ui->plainTextEditSinogramInfo->appendPlainText(param.sinogram_info);
    ui->comboBoxIterator->setCurrentText(param.iterator_type);
    if (current_task.GetLoadedFlag()) {
        UpdateSinogram_(param.index_sinogram);
        UpdateProjection_(param.index_projection);
        std::cout << "param.index_sinogram: " << param.index_sinogram << std::endl;
        ui->horizontalScrollBarProjection->setEnabled(true);
        ui->comboBoxProjectionIndex->setEnabled(true);

        ui->horizontalScrollBarProjection->setValue(param.index_projection);
        ui->horizontalScrollBarProjection->setMinimum(0);
        ui->horizontalScrollBarProjection->setMaximum(param.num_angles);
        ui->horizontalScrollBarSinogram->setEnabled(true);

        ui->comboBoxSinogramIndex->setEnabled(true);
        ui->horizontalScrollBarSinogram->setValue(param.index_sinogram);
        ui->horizontalScrollBarSinogram->setMinimum(0);
        ui->horizontalScrollBarSinogram->setMaximum(param.num_slices);
        DrawProjectionLine_();

    } else {
        ui->comboBoxProjectionIndex->setEnabled(false);
        ui->comboBoxSinogramIndex->setEnabled(false);
        ui->horizontalScrollBarProjection->setEnabled(false);
        ui->horizontalScrollBarSinogram->setEnabled(false);
        ui->labelProjectionImage->clear();
        ui->labelProjectionImage->setText("Projections Preview");
        ui->labelSinogramImage->clear();
        ui->labelSinogramImage->setText("Sinograms Preview");
    }

    if (current_task.GetStatus() == ReconTask::Status::kCompleted) {
        ui->pushButtonShowResult->setEnabled(true);
        SetLabelImage(*ui->labelResultImage, current_task.GetPixmapResult(0));
        const vector<int>& result_iter_index_array = current_task.GetResultIterIndexArray();
        ui->comboBoxResult->addItem(tr("Final Result"));
        for (auto index: result_iter_index_array) {
            ui->comboBoxResult->addItem(QString::number(index));
        }
        ui->comboBoxResult->setEnabled(true);
        ui->horizontalScrollBarResult->setMinimum(0);
        ui->horizontalScrollBarResult->setMaximum(static_cast<int>(result_iter_index_array.size()));
        ui->horizontalScrollBarResult->setEnabled(true);
    } else {
        ui->pushButtonShowResult->setEnabled(false);
        ui->labelResultImage->clear();
        ui->labelResultImage->setText("Result Preview");
        ui->horizontalScrollBarResult->setEnabled(false);
        ui->comboBoxResult->clear();
        ui->comboBoxResult->setEnabled(false);
    }
    ui->comboBoxDataType->setCurrentText(
            GetLabelStringFromDataType(param.file_data_type));
    ui->comboBoxSinogramFormat->setCurrentText(
            GetLabelStringFromFileFormat(param.file_format));
    ParamChanged_();
}

void MainWindow::CreateNewTask_()
{
    task_array_.emplace_back(new ReconTask(this));
    connect(task_array_.back(), &ReconTask::TaskCompleted, this, &MainWindow::TaskCompleted);
    ui->listWidgetTask->addItem("untitled task");
    ui->listWidgetTask->setCurrentRow(ui->listWidgetTask->count() - 1);
    UpdateParameterDisplay_();
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
            progress_bar_->setVisible(false);
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
            progress_bar_->setVisible(true);
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
        UpdateParameterDisplay_();
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
    QString output_path = filename;
    if (!param.output_dir.isEmpty()) {
        output_path = QDir(param.output_dir).filePath(filename);
    }
    if (QDir(output_path).isRelative()) {
        QString baseDir = QDir::home().filePath("spect-recon-data");
        QDir(baseDir).mkpath(param.output_dir);
        output_path = QDir(baseDir).filePath(output_path);
    }
    qDebug() << "Start saving to " << output_path << "..." << endl;
    int r = GetCurrentParameter_().ToProtobufFilePath(output_path);

    if (r == EINVALID_PATH) {
        auto box = new QMessageBox(this);
        box->setText(tr("Cannot open file ") + output_path + ".");
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
    int num_slices = 0;
    int num_angles = 0;
    int num_detectors = 128;
    ReconTask& current_task = CurrentTask_();
    auto& recon_param = current_task.GetParameter();

    SinogramFileReader::FileFormat file_format = SinogramFileReader::FileFormat::kDicom;
    auto file_format_s = ui->comboBoxSinogramFormat->currentText();
    QString allowed_format;
    if (file_format_s == "DICOM") {
        file_format = SinogramFileReader::FileFormat::kDicom;
        allowed_format = "DICOM files (*.dcm)";
    } else {
        if (file_format_s == "Raw (Sinograms)") {
            file_format = SinogramFileReader::FileFormat::kRawSinogram;
            allowed_format = "Raw Files (*.a00 *.sin)";
        } else if (file_format_s == "Raw (Projections)") {
            file_format = SinogramFileReader::FileFormat::kRawProjection;
            allowed_format = "Raw Files (*.a00 *.proj)";
        } else {
            std::cerr << "Invalid file_format: " << file_format_s.toStdString() << std::endl;
            exit(-1);
        }
        bool ok = false;
        num_slices = ui->lineEditNumSlices->text().toUInt(&ok);
        if (!ok) {
            auto box = new QMessageBox(this);
            box->setText("Positive value of # Slices is required.");
            box->exec();
            return;
        }
        num_angles = ui->lineEditNumAngles->text().toUInt(&ok);
        if (!ok) {
            auto box = new QMessageBox(this);
            box->setText("Positive value of # Angles is required.");
            box->exec();
            return;
        }
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

        SinogramFileReader reader(path.toStdString(), file_format,
                                  num_slices, num_angles, kNumDetectors, file_data_type);
        if (reader.GetStatus() == SinogramFileReader::Status::kFailToReadFile) {
            auto box = new QMessageBox(this);
            box->setText(QString("Cannot read file ") + path + ".");
            box->exec();
            return;
        } else if (reader.GetStatus() == SinogramFileReader::Status::kInvalidShape) {
            auto box = new QMessageBox(this);
            box->setText(QString("Invalid shape for file ") + path + ".");
            box->exec();
            return;
        } else if (reader.GetStatus() == SinogramFileReader::Status::kFailToParseFile) {
            auto box = new QMessageBox(this);
            box->setText(QString("Failed to parse file ") + path + ".");
            box->exec();
            return;
        }

        const auto& shape_sinogram = reader.GetSinogram().shape();
        QString sinogram_info = QString::fromStdString(reader.GetSinogramInfo().GetInfoString());
        ui->plainTextEditSinogramInfo->appendPlainText(sinogram_info);

        current_task.SetPixmapSinogramArray(reader.GetSinogramPixmap());
        current_task.SetPixmapProjectionArray(reader.GetProjectionPixmap());

        num_slices = shape_sinogram[0];
        num_angles = shape_sinogram[1];
        num_detectors = shape_sinogram[2];

        recon_param.sinogram = reader.GetSinogram();
        recon_param.projection = reader.GetProjection();
        recon_param.sinogram_info = sinogram_info;
        recon_param.num_slices = num_slices;
        recon_param.num_angles = num_angles;
        recon_param.num_detectors = num_detectors;
        assert (shape_sinogram.size() == 3);

        ui->horizontalScrollBarProjection->setEnabled(true);
        ui->horizontalScrollBarProjection->setMinimum(0);
        ui->horizontalScrollBarProjection->setMaximum(num_angles - 1);
        UpdateComboBoxProjectionIndex_();

        ui->horizontalScrollBarSinogram->setEnabled(true);
        ui->horizontalScrollBarSinogram->setMinimum(0);
        ui->horizontalScrollBarSinogram->setMaximum(num_slices - 1);
        UpdateComboBoxSinogramIndex_();

        UpdateStatusBar_();
        SetLabelImage(*ui->labelSinogramImage, CurrentTask_().GetCurrentPixmapSinogram());
        SetLabelImage(*ui->labelProjectionImage, CurrentTask_().GetCurrentPixmapProjection());

        DrawProjectionLine_();
        current_dir_ = QFileInfo(path).absoluteDir().path();
    }
}

void MainWindow::UpdateSinogram_(int value)
{
    ReconTask& current_task = CurrentTask_();
    CurrentTask_().GetParameter().index_sinogram = value;
    int w = ui->labelSinogramImage->width();
    int h = ui->labelSinogramImage->height();
    ui->labelSinogramImage->setPixmap(
            current_task.GetCurrentPixmapSinogram().scaled(w, h, Qt::KeepAspectRatio));
}

void MainWindow::UpdateProjection_(int index)
{
    ReconTask& current_task = CurrentTask_();
    CurrentTask_().GetParameter().index_projection = index;
    int w = ui->labelProjectionImage->width();
    int h = ui->labelProjectionImage->height();
    ui->labelProjectionImage->setPixmap(
            current_task.GetCurrentPixmapProjection().scaled(w, h, Qt::KeepAspectRatio));
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
    auto path_sysmat = ui->lineEditSysMat->text();
    if (path_sysmat.isEmpty()) {
        auto box = new QMessageBox(this);
        box->setText("A system matrix must be provided.");
        box->exec();
        return;
    }
    RunTask_(CurrentTaskIndex_());
}


void MainWindow::on_actionImport_Tasks_triggered()
{
    if (ui->listWidgetTask->count() == 0) CreateNewTask_();
    QString path = QFileDialog::getOpenFileName(this,
                                                tr("Select s task file."),
                                                current_dir_,
                                                tr("TASK (*.task)"));
    auto& current_task = CurrentTask_();
    auto& param = GetCurrentParameter_();
    param.FromProtobufFilePath(path);
    if (param.sinogram.shape().size()) {
        current_task.SetPixmapSinogramArray(GetPixmapArrayFromTensor3D(param.sinogram));
        current_task.SetPixmapProjectionArray(GetPixmapArrayFromTensor3D(param.projection));
    }
    UpdateParameterDisplay_();
    UpdateComboBoxProjectionIndex_();
    UpdateComboBoxSinogramIndex_();
}

void MainWindow::RunTask_(int task_index)
{
    assert (task_index < GetTaskCount_());
    task_array_[task_index]->Start();
    ui->pushButtonRun->setEnabled(false);
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
        task->Start();
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
    ui->comboBoxSinogramIndex->setCurrentIndex(value);
    ParamChanged_();
}


void MainWindow::on_horizontalScrollBarProjection_valueChanged(int value)
{
    UpdateProjection_(value);
    DrawProjectionLine_();
    ui->comboBoxProjectionIndex->setCurrentIndex(value);
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
    int w = ui->labelProjectionImage->width();
    int h = ui->labelProjectionImage->height();
    ui->labelProjectionImage->setPixmap(
            QPixmap::fromImage(projection_image_buffer_).scaled(w, h, Qt::KeepAspectRatio));
}

void MainWindow::on_pushButtonShowResult_clicked()
{
    ReconTask& current_task = CurrentTask_();
    ReconTaskParameter& param = current_task.GetParameter();
    QVector<QPixmap> pixmap_array;
    for (const auto& result: current_task.GetResultArray()) {
        pixmap_array.push_back(GetPixmapFromTensor2D(result));
    }
    ResultDialog *result_dialog = new ResultDialog(pixmap_array,
                                                   GetPixmapFromTensor2D(param.sinogram_used_to_reconstruct),
                                                   this);
    result_dialog->exec();
}

void MainWindow::TaskCompleted(ReconTask *recon_task)
{
    if (&CurrentTask_() == recon_task) {
        auto& result_iter_index_array = recon_task->GetResultIterIndexArray();
        if (result_iter_index_array.size() == 0) {
            std::cerr << "recon_task.GetResultArray() is empty.\n";
            return;
        }
        int w = ui->labelResultImage->width();
        int h = ui->labelResultImage->height();
        ui->labelResultImage->setPixmap(
                recon_task->GetPixmapResult(0).scaled(w, h, Qt::KeepAspectRatio));
        ui->horizontalScrollBarResult->setMaximum(static_cast<int>(result_iter_index_array.size()));
        ui->horizontalScrollBarResult->setMinimum(0);
        ui->horizontalScrollBarResult->setEnabled(true);
        ui->comboBoxResult->addItem("Final Result");
        for (auto index: result_iter_index_array) {
            ui->comboBoxResult->addItem(QString::number(index));
        }
        ui->comboBoxResult->setEnabled(true);
    }
}
void MainWindow::on_comboBoxProjectionIndex_currentIndexChanged(int index)
{
    if (GetTaskCount_() == 0) return;
    if (index < 0 || GetCurrentParameter_().projection.shape().empty()
            || index >= GetCurrentParameter_().projection.shape()[0]) return;
    UpdateProjection_(index);
    DrawProjectionLine_();
    ui->horizontalScrollBarProjection->setValue(index);
    ParamChanged_();
}


void MainWindow::on_comboBoxSinogramIndex_currentIndexChanged(int index)
{
    if (GetTaskCount_() == 0) return;
    if (index < 0 || GetCurrentParameter_().sinogram.shape().empty()
            || index >= GetCurrentParameter_().sinogram.shape()[0]) return;
    UpdateSinogram_(index);
    DrawProjectionLine_();
    ui->horizontalScrollBarSinogram->setValue(index);
    ParamChanged_();
}


void MainWindow::on_comboBoxDataType_currentTextChanged(const QString &data_type_s)
{
    using recontaskparameter_pb::ReconTaskParameterPB_FileDataType;
    ReconTaskParameterPB_FileDataType file_data_type;
    if (data_type_s == "float32") {
        file_data_type = ReconTaskParameterPB_FileDataType::ReconTaskParameterPB_FileDataType_FLOAT32;
    } else if (data_type_s == "float64") {
        file_data_type = ReconTaskParameterPB_FileDataType::ReconTaskParameterPB_FileDataType_FLOAT64;
    } else if (data_type_s == "int8") {
        file_data_type = ReconTaskParameterPB_FileDataType::ReconTaskParameterPB_FileDataType_INT8;
    } else if (data_type_s == "uint8") {
        file_data_type = ReconTaskParameterPB_FileDataType::ReconTaskParameterPB_FileDataType_UINT8;
    } else if (data_type_s == "int16") {
        file_data_type = ReconTaskParameterPB_FileDataType::ReconTaskParameterPB_FileDataType_INT16;
    } else if (data_type_s == "uint16") {
        file_data_type = ReconTaskParameterPB_FileDataType::ReconTaskParameterPB_FileDataType_UINT16;
    } else if (data_type_s == "int32") {
        file_data_type = ReconTaskParameterPB_FileDataType::ReconTaskParameterPB_FileDataType_INT32;
    } else if (data_type_s == "uint32") {
        file_data_type = ReconTaskParameterPB_FileDataType::ReconTaskParameterPB_FileDataType_UINT32;
    } else if (data_type_s == "int64") {
        file_data_type = ReconTaskParameterPB_FileDataType::ReconTaskParameterPB_FileDataType_INT64;
    } else if (data_type_s == "uint64") {
        file_data_type = ReconTaskParameterPB_FileDataType::ReconTaskParameterPB_FileDataType_UINT64;
    } else {
        std::cerr << "Invalid data_type: " << data_type_s.toStdString() << std::endl;
        exit(-1);
    }
    GetCurrentParameter_().file_data_type = file_data_type;
    ParamChanged_();
}


void MainWindow::on_comboBoxSinogramFormat_currentTextChanged(const QString &file_format_s)
{
    using recontaskparameter_pb::ReconTaskParameterPB_FileFormat;
    ReconTaskParameterPB_FileFormat file_format;
    QString allowed_format;
    if (file_format_s == "DICOM") {
        file_format = ReconTaskParameterPB_FileFormat::ReconTaskParameterPB_FileFormat_DICOM;
    } else {
        if (file_format_s == "Raw (Sinograms)") {
            file_format = ReconTaskParameterPB_FileFormat::ReconTaskParameterPB_FileFormat_RAW_SINOGRAM;
        } else if (file_format_s == "Raw (Projections)") {
            file_format = ReconTaskParameterPB_FileFormat::ReconTaskParameterPB_FileFormat_RAW_PROJECTION;
        } else {
            std::cerr << "Invalid file_format: " << file_format_s.toStdString() << std::endl;
            exit(-1);
        }
    }
    GetCurrentParameter_().file_format = file_format;
    ParamChanged_();
}


void MainWindow::on_lineEditNumSlices_textEdited(const QString &arg1)
{
    bool ok;
    int num = arg1.toUInt(&ok);
    if (!ok) {
        ShowMessageBox("Positive # Slices is required.");
        return;
    }
    GetCurrentParameter_().num_slices = num;
    ParamChanged_();
}


void MainWindow::on_lineEditNumAngles_textEdited(const QString &arg1)
{
    bool ok;
    int num = arg1.toUInt(&ok);
    if (!ok) {
        ShowMessageBox("Positive # Angles is required.");
        return;
    }
    GetCurrentParameter_().num_angles = num;
    ParamChanged_();
}


void MainWindow::on_lineEditScatterCoeff_textEdited(const QString &arg1)
{
    bool ok;
    double num = arg1.toDouble(&ok);
    if (!ok) {
        ShowMessageBox("Positive Scatter Coeefficient is required.");
        return;
    }
    GetCurrentParameter_().coeff_scatter = num;
    ParamChanged_();
}

void MainWindow::on_comboBoxResult_activated(int index)
{
    ui->horizontalScrollBarResult->setValue(index);
    SetLabelImage(*ui->labelResultImage, CurrentTask_().GetPixmapResult(index));
}

void MainWindow::on_horizontalScrollBarResult_valueChanged(int value)
{
    ui->comboBoxResult->setCurrentIndex(value);
    SetLabelImage(*ui->labelResultImage, CurrentTask_().GetPixmapResult(value));
}
