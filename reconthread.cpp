#include "reconthread.h"

#include <stdlib.h>
#include <string.h>
#include <algorithm>

#include <QDir>
#include <QDebug>
#include <QElapsedTimer>

#include "spect.h"
#include "scascnet.h"
#include "sinogram.h"

#define OS_LINUX

ReconThread::ReconThread(QObject *parent)
    : QThread(parent)
{
    // 初始化内存全局变量
    SPECTSetLogLevel(LOG_INFO, true);
    int rv = SPECTInitialize();
    if (0 != rv) {
        qDebug() << __FUNCTION__ << ": ";
        qDebug() << "fail to init planning..., error:%s\n";
        qDebug() << SPECTGetErrString((ERR_CODE)rv) << endl;
    }
}

void ReconThread::SetParameter(const ReconTaskParameter &param) {
    QString base_dir = QDir::home().filePath("spect-recon");

    spect_param_.io_param.sino_path = param.path_sinogram.toStdString();
    spect_param_.io_param.sysmat_path = param.path_sysmat.toStdString();
    spect_param_.io_param.sca_path = param.path_scatter_map.toStdString();

    spect_param_.io_param.mumap_path = "./attenuation.mmap";

    spect_param_.num_iters = param.num_iters;
    spect_param_.num_dual_iters = param.num_dual_iters;
    spect_param_.sca_param = param.coeff_scatter;

    QString basename = param.task_name;
    spect_param_.io_param.recon_filename = basename.toStdString();
    spect_param_.io_param.recon_hr_filename = basename.toStdString();

    // Setup output directory.
    QString outputDir = param.output_dir.trimmed();
    if (QDir(outputDir).isRelative()) {
        QDir d = QDir(base_dir);
        outputDir = d.filePath(outputDir);
        d = QDir(outputDir);
        d.mkpath(basename);
        outputDir = d.filePath(basename);
    }
    spect_param_.io_param.outputdir = outputDir.toStdString();


    const int sinogram_start_index = std::max<int>(param.index_sinogram - kNumSlices / 2 + 1, 0);
    const int sinogram_inner_index = param.index_sinogram - sinogram_start_index;
    const int sinogram_end_index = std::min<int>(sinogram_start_index + kNumSlices, param.sinogram.shape()[0]);
    std::vector<double> buff(kNumSlices * kNumAngles * kNumDetectors);
    const int element_start_index = sinogram_start_index * kNumDetectors * kNumAngles;
    for (int i = 0; i < (sinogram_end_index - sinogram_start_index) * kNumAngles * kNumDetectors; ++i) {
        buff[i] = param.sinogram.data()[element_start_index + i];
    }
    Sinogram<double> input_sinogram(buff, kNumSlices, kNumAngles, kNumDetectors);
    Sinogram<double> restored_sinogram;
    if (param.use_nn) {
        Scascnet net(param.path_model.toStdString().c_str());

        qDebug() << "Performing restoration..." << endl;
        QElapsedTimer timer;
        timer.start();
        restored_sinogram = net.Run(input_sinogram.TransformType<float>()).TransformType<double>();
        qDebug() << "Time consumed for restoration: " << timer.elapsed() << " (ms)." << endl;
        qDebug() << "Restoration completed." << endl;

        QFileInfo info_input_sinogram(param.path_sinogram);
        std::string restoredSinogramOutputPath = QDir(outputDir).filePath(
                    info_input_sinogram.fileName() + ".resf").toStdString();

        restoredSinogramOutputPath = QDir(outputDir).filePath(
                    info_input_sinogram.fileName() + ".resd").toStdString();
        restored_sinogram.WriteToFilePath(restoredSinogramOutputPath);

        spect_param_.io_param.sino_path = restoredSinogramOutputPath;
        qDebug() << "Restored sinogram saved to "
            << QString::fromStdString(restoredSinogramOutputPath) << endl;
    } else {
        restored_sinogram = input_sinogram;
    }
    spect_param_.io_param.sinogram_data.resize(kNumAngles * kNumDetectors);
    for (int i = 0, j = sinogram_inner_index * kNumAngles * kNumDetectors; i < kNumAngles * kNumDetectors; ++i, ++j) {
        spect_param_.io_param.sinogram_data[i] = restored_sinogram.GetData()[j];
    }
    spect_param_.io_param.asum_filename = (basename + ".asum").toStdString();
    spect_param_.Print();

    if (param.iterator_type == "EM-Tikhonov") spect_param_.iterator_type = EM_TIKHONOV;
    else if (param.iterator_type == "PAPA-2DWavelet") spect_param_.iterator_type = PAPA_2D_WAVELET;
    else if (param.iterator_type == "PAPA-Cont") spect_param_.iterator_type = PAPA_CONT;
    else if (param.iterator_type == "PAPA-Cont-TV") spect_param_.iterator_type = PAPA_CONT_TV;
    else if (param.iterator_type == "PAPA-Cont-Wavelet") spect_param_.iterator_type = PAPA_CONT_WAVELET;
    else if (param.iterator_type == "PAPA-Dynamic") spect_param_.iterator_type = PAPA_DYNAMIC;
    else if (param.iterator_type == "PAPA-TV") spect_param_.iterator_type = PAPA_TV;
    else if (param.iterator_type == "MLEM") spect_param_.iterator_type = MLEM;
    else {
        qDebug() << "Invalid iterator_type: " << param.iterator_type << endl;
        exit(-1);
    }
}

void ReconThread::run()
{
    SPECTProject spect_project;
    progress_ = 0;
    Reconstruct();
}

void ReconThread::Reconstruct()
{
    QElapsedTimer timer;
    timer.start();
    SPECTProject spect_project;
    spect_project.SetSpectParams(spect_param_);
    std::vector<std::vector<double> > recon_result_array;
    spect_project.GenerateBackProject(&progress_, &recon_result_array);
    qDebug() << "Time consumed for reconstruction: " << timer.elapsed() << " (ms)." << endl;
    qDebug() << "Task " << " completed." << endl;
}
