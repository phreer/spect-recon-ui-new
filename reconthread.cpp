#include "reconthread.h"
#include "spect.h"
#include "scascnet.h"
#include "sinogram.h"

#include <QDir>
#include <QDebug>

QString ReconThread::model_path = "/Users/phree/workspace/SPECT/scascnet/scascnet/ckpt_e40_0_p25.2163251814763.pth.onnx";

ReconThread::ReconThread(QObject *parent, const QVector<ReconTaskParameter> &reconTaskParamList)
    : QThread(parent), _reconTaskParamList(reconTaskParamList)
{
    SPECTSetLogLevel(LOG_INFO, true);

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
        _currentTask = i;
        _process = 0;
        reconstruct(spectProject, _reconTaskParamList[i]);
        emit(milestone(i, 100));
    }
}

void ReconThread::reconstruct(SPECTProject &spectProject, const ReconTaskParameter &param)
{
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

    if (param.useNN) {
        Scascnet net(model_path.toStdString().c_str());
        Sinogram<float> inputSinogram(param.pathSinogram.toStdString(), kNumSlices, kNumAngles, kNumDetectors);
        qDebug() << "Running neural networks..." << endl;
        Sinogram<float> restoredSinogram = net.Run(inputSinogram);
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
        qDebug() << "Writing result to " << QString::fromStdString(restoredSinogramOutputPath) << endl;
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
    spectProject.GenerateBackProject(&_process);
}
