#include "recontaskparameter.h"

#include <QDebug>
#include <QDir>

#include "error_code.h"

recontaskparameter_pb::ReconTaskParameterPB_IteratorType
ReconTaskParameter::IteratorTypeStrToPB(const QString &iterator)
{
    using namespace recontaskparameter_pb;
    QString lowercase = iterator.toLower();
    ReconTaskParameterPB_IteratorType iterator_pb = ReconTaskParameterPB_IteratorType_MLEM;
    if (lowercase == "mlem") {
        iterator_pb = ReconTaskParameterPB_IteratorType_MLEM;
    } else if (lowercase == "em-tikhonov") {
        iterator_pb = ReconTaskParameterPB_IteratorType_EM_Tikhonov;
    } else if (lowercase == "papa-2dwavelet") {
        iterator_pb = ReconTaskParameterPB_IteratorType_PAPA_2DWavelet;
    } else if (lowercase == "papa-cont") {
        iterator_pb = ReconTaskParameterPB_IteratorType_PAPA_Cont;
    } else if (lowercase == "papa-cont-tv") {
        iterator_pb = ReconTaskParameterPB_IteratorType_PAPA_Cont_TV;
    } else if (lowercase == "papa-cont-wavelet") {
        iterator_pb = ReconTaskParameterPB_IteratorType_PAPA_Cont_Wavelet;
    } else if (lowercase == "papa-dynamic") {
        iterator_pb = ReconTaskParameterPB_IteratorType_PAPA_Dynamic;
    } else if (lowercase == "papa-tv") {
        iterator_pb = ReconTaskParameterPB_IteratorType_PAPA_TV;
    } else {
        qDebug() << "Invlid iterator type " << lowercase << endl;
        exit(-1);
    }
    return iterator_pb;
}

QString ReconTaskParameter::IteratorTypePBToStr(
        recontaskparameter_pb::ReconTaskParameterPB_IteratorType iterator_pb)
{
    using namespace recontaskparameter_pb;
    QString iterator;
    switch (iterator_pb) {
    case ReconTaskParameterPB_IteratorType_MLEM: {
        iterator = "MLEM";
        break;
    }
    case ReconTaskParameterPB_IteratorType_EM_Tikhonov: {
        iterator = "EM-Tikhonov";
        break;
    }
    case ReconTaskParameterPB_IteratorType_PAPA_2DWavelet: {
        iterator = "PAPA-2DWavelet";
        break;
    }
    case ReconTaskParameterPB_IteratorType_PAPA_Cont: {
        iterator = "PAPA-Cont";
        break;
    }
    case ReconTaskParameterPB_IteratorType_PAPA_Cont_TV: {
        iterator = "PAPA-Cont-TV";
        break;
    }
    case ReconTaskParameterPB_IteratorType_PAPA_Cont_Wavelet: {
        iterator = "PAPA-Cont-Wavelet";
        break;
    }
    case ReconTaskParameterPB_IteratorType_PAPA_Dynamic: {
        iterator = "PAPA-Dynamic";
        break;
    }
    case ReconTaskParameterPB_IteratorType_PAPA_TV: {
        iterator = "PAPA-TV";
        break;
    }
    }
    return iterator;
}

std::shared_ptr<recontaskparameter_pb::ReconTaskParameterPB>
ReconTaskParameter::ToProtobuf() const
{
    using namespace recontaskparameter_pb;
    auto param_ptr = std::make_shared<ReconTaskParameterPB>();
    param_ptr->set_path_sysmat(pathSysMat.toStdString());
    param_ptr->set_path_sinogdram(pathSinogram.toStdString());
    param_ptr->set_path_scatter_map(pathScatterMap.toStdString());
    param_ptr->set_output_dir(outputDir.toStdString());
    param_ptr->set_task_name(taskName.toStdString());
    param_ptr->set_num_iters(numIters);
    param_ptr->set_num_dual_iters(numDualIters);
    param_ptr->set_gamma(gamma);
    param_ptr->set_lambda(lambda);
    param_ptr->set_scatter_coeff(paramScatter);
    param_ptr->set_use_nn(useNN);
    param_ptr->set_use_scatter_map(useScatterMap);
    param_ptr->set_path_mu_map(pathMuMap.toStdString());
    param_ptr->set_iterator(IteratorTypeStrToPB(iteratorType));
    return param_ptr;
}


int ReconTaskParameter::FromProtobuf(
        const recontaskparameter_pb::ReconTaskParameterPB &param_pb)
{
    pathSysMat = QString::fromStdString(param_pb.path_sysmat());
    pathSinogram = QString::fromStdString(param_pb.path_sinogdram());
    taskName = QString::fromStdString(param_pb.task_name());
    lambda = param_pb.lambda();
    gamma = param_pb.lambda();
    useNN = param_pb.use_nn();
    useScatterMap = param_pb.use_scatter_map();
    numIters = param_pb.num_iters();
    numDualIters = param_pb.has_num_dual_iters();
    iteratorType = IteratorTypePBToStr(param_pb.iterator());
    if (param_pb.has_path_scatter_map()) {
        pathScatterMap = QString::fromStdString(param_pb.path_scatter_map());
    } else {
        pathScatterMap = QString();
    }
    if (param_pb.has_scatter_coeff()) {
        paramScatter = param_pb.scatter_coeff();
    } else {
        paramScatter = 0.;
    }
    return 0;
}


int ReconTaskParameter::ToProtobufFilePath(const QString &path) const
{
    std::ofstream outstream(path.toStdString(),
                            std::ios::out | std::ios::trunc | std::ios::binary);
    if (!outstream.is_open()) {
        return EINVALID_PATH;
    }
    auto paramPb = ToProtobuf();
    qDebug() << "Number of dual iterations: " << numDualIters;
    qDebug() << "Number of dual iterations (PB): " << paramPb->num_dual_iters();

    if (!paramPb->SerializeToOstream(&outstream)) {
        return EUNKNOWN;
    }
    return 0;
}

int ReconTaskParameter::FromProtobufFilePath(const QString &path)
{
    std::ifstream instream(path.toStdString(), std::ios::in | std::ios::binary);
    if (!instream.is_open()) {
        return EINVALID_PATH;
    }
    recontaskparameter_pb::ReconTaskParameterPB param_pb;
    if (!param_pb.ParseFromIstream(&instream)) {
        return EUNKNOWN;
    }

    FromProtobuf(param_pb);
    return 0;
}
