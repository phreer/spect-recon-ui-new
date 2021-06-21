#include "reconthread.h"

#include <stdlib.h>
#include <string.h>

#include <QDir>
#include <QDebug>
#include <QElapsedTimer>

#include "spect.h"
#include "scascnet.h"
#include "sinogram.h"

#define OS_LINUX

ReconThread::ReconThread(QObject *parent, const QVector<ReconTaskParameter> &reconTaskParamList, int startTaskIndex)
    : QThread(parent), _reconTaskParamList(reconTaskParamList), _startTaskIndex(startTaskIndex)
{
    SPECTSetLogLevel(LOG_INFO, true);
#if defined (OS_LINUX) && defined (USE_APPIMAGE)
    const char *appdir = getenv("APPDIR");
    _modelPath = QString::fromStdString(appdir) + "/usr/share/model/ckpt_e40_0_p25.2163251814763.pth.onnx";
#else
    const char *envModelPath = getenv("MODEL_PATH");
    if (strlen(envModelPath)) {
        _modelPath = QString::fromStdString(envModelPath);
    } else {
        _modelPath = "ckpt_e40_0_p25.2163251814763.pth.onnx";
    }
#endif
    qDebug() << "Model path: " << _modelPath << endl;

    // 初始化内存全局变量
    int rv;
    rv = SPECTInitialize();
    if (0 != rv)
    {
        qDebug() << __FUNCTION__ << ": ";
        qDebug() << "fail to init planning..., error:%s\n";
        qDebug() << SPECTGetErrString((ERR_CODE)rv) << endl;
    }
}

void ReconThread::run()
{
    SPECTProject spectProject;
    for (auto i = 0; i < _reconTaskParamList.size(); ++i)
    {
        _taskIndexOffset = i;
        _process = 0;
        reconstruct(spectProject, _reconTaskParamList[i]);
        emit(milestone(getTaskID(), 100));
    }
}

void ReconThread::reconstruct(SPECTProject &spectProject, const ReconTaskParameter &param)
{
    qDebug() << "Start task " << getTaskID() << "..." << endl;
    QString baseDir = QDir::home().filePath("spect-recon");

    SPECTParam spectParam;
    spectParam.io_param.sino_path = param.pathSinogram.toStdString();
    spectParam.io_param.sysmat_path = param.pathSysMat.toStdString();
    spectParam.io_param.sca_path = param.pathScatterMap.toStdString();

    spectParam.io_param.mumap_path = "./attenuation.mmap";

    spectParam.num_iters = param.numIters;
    spectParam.num_dual_iters = param.numDualIters;
    spectParam.sca_param = param.paramScatter;

    QString basename = param.taskName;
    spectParam.io_param.recon_filename = basename.toStdString();
    spectParam.io_param.recon_hr_filename = basename.toStdString();

    // Setup output directory.
    QString outputDir = param.outputDir.trimmed();
    if (QDir(outputDir).isRelative()) {
        QDir d = QDir(baseDir);
        outputDir = d.filePath(outputDir);
        d = QDir(outputDir);
        d.mkpath(basename);
        outputDir = d.filePath(basename);
    }
    spectParam.io_param.outputdir = outputDir.toStdString();

    Sinogram<float> inputSinogram(param.pathSinogram.toStdString(), kNumSlices, kNumAngles, kNumDetectors);
    if (param.useNN) {
        Scascnet net(_modelPath.toStdString().c_str());
        qDebug() << "Running neural networks..." << endl;

        QElapsedTimer timer;
        timer.start();
        Sinogram<float> restoredSinogram = net.Run(inputSinogram);
        qDebug() << "Time consumed for restoration: " << timer.elapsed() << " (ms)." << endl;

        Sinogram<double> restoredSinogramDouble = restoredSinogram.TransformType<double>();

        qDebug() << "Restoration completed." << endl;
        QFileInfo infoInputSinogram(param.pathSinogram);
        std::string restoredSinogramOutputPath = QDir(outputDir).filePath(
                    infoInputSinogram.fileName() + ".resf").toStdString();
        restoredSinogram.WriteToFilePath(restoredSinogramOutputPath);

        restoredSinogramOutputPath = QDir(outputDir).filePath(
                    infoInputSinogram.fileName() + ".resd").toStdString();
        restoredSinogramDouble.WriteToFilePath(restoredSinogramOutputPath);

        spectParam.io_param.sino_path = restoredSinogramOutputPath;
        qDebug() << "Restored sinogram saved to "
            << QString::fromStdString(restoredSinogramOutputPath) << endl;
    } else {
        Sinogram<double> sinogramDouble = inputSinogram.TransformType<double>();
        QFileInfo infoInputSinogram(param.pathSinogram);
        std::string sinogramDoubleOutputPath = QDir(outputDir).filePath(
                    infoInputSinogram.fileName() + ".sino").toStdString();
        sinogramDouble.WriteToFilePath(sinogramDoubleOutputPath);
        qDebug() << "Sinogram (double) saved to "
            << QString::fromStdString(sinogramDoubleOutputPath) << endl;
        spectParam.io_param.sino_path = sinogramDoubleOutputPath;
    }

    spectParam.io_param.asum_filename = (basename + ".asum").toStdString();
    spectParam.Print();

    if (param.iteratorType == "EM-Tikhonov") spectParam.iterator_type = EM_TIKHONOV;
    else if (param.iteratorType == "PAPA-2DWavelet") spectParam.iterator_type = PAPA_2D_WAVELET;
    else if (param.iteratorType == "PAPA-Cont") spectParam.iterator_type = PAPA_CONT;
    else if (param.iteratorType == "PAPA-Cont-TV") spectParam.iterator_type = PAPA_CONT_TV;
    else if (param.iteratorType == "PAPA-Cont-Wavelet") spectParam.iterator_type = PAPA_CONT_WAVELET;
    else if (param.iteratorType == "PAPA-Dynamic") spectParam.iterator_type = PAPA_DYNAMIC;
    else if (param.iteratorType == "PAPA-TV") spectParam.iterator_type = PAPA_TV;
    else if (param.iteratorType == "MLEM") spectParam.iterator_type = MLEM;
    else {
        qDebug() << "Invalid iteratorType: " << param.iteratorType << endl;
        exit(-1);
    }
    spectProject.SetSpectParams(spectParam);

    QElapsedTimer timer;
    timer.start();
    spectProject.GenerateBackProject(&_process);
    qDebug() << "Time consumed for reconstruction: " << timer.elapsed() << " (ms)." << endl;
    qDebug() << "Task " << getTaskID() << " completed." << endl;
}
