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
    param_ptr->set_path_sysmat(path_sysmat.toStdString());
    param_ptr->set_path_sinogdram(path_sinogram.toStdString());
    param_ptr->set_path_scatter_map(path_scatter_map.toStdString());
    param_ptr->set_path_model(path_model.toStdString());
    param_ptr->set_output_dir(output_dir.toStdString());
    param_ptr->set_task_name(task_name.toStdString());
    param_ptr->set_sinogram_info(sinogram_info.toStdString());

    param_ptr->set_num_iters(num_iters);
    param_ptr->set_num_dual_iters(num_dual_iters);
    param_ptr->set_gamma(gamma);
    param_ptr->set_lambda(lambda);
    param_ptr->set_coeff_scatter(coeff_scatter);
    param_ptr->set_resolution(resolution);
    param_ptr->set_num_detectors(num_detectors);
    param_ptr->set_num_angles(num_angles);
    param_ptr->set_num_slices(num_slices);

    param_ptr->set_use_nn(use_nn);
    param_ptr->set_use_scatter_map(use_scatter_map);

    param_ptr->set_path_mu_map(path_mumap.toStdString());
    param_ptr->set_iterator(IteratorTypeStrToPB(iterator_type));

    param_ptr->set_file_data_type(file_data_type);
    param_ptr->set_file_format(file_format);
    param_ptr->set_num_input_images(num_input_images);
    param_ptr->set_index_sinogram(index_sinogram);
    param_ptr->set_index_projection(index_projection);

    // Serialize sinogram data and projection data.
    if (sinogram.shape().size() > 0) {
        if (sinogram.shape().size() != 3) {
            std::cerr << "Invalid size of sinogram shape: " << sinogram.shape().size() << std::endl;
            exit(-1);
        }
        if (projection.shape().size() != 3) {
            std::cerr << "Invalid size of projection shape: " << projection.shape().size() << std::endl;
            exit(-1);
        }
        param_ptr->set_loaded(true);
        for (size_t i = 0; i < sinogram.data().size(); ++i) {
            param_ptr->add_sinogram_data(sinogram.data()[i]);
        }
        param_ptr->set_num_slices(sinogram.shape()[0]);
        param_ptr->set_num_angles(sinogram.shape()[1]);
        param_ptr->set_num_detectors(sinogram.shape()[2]);
        param_ptr->set_num_results(reconstructed_tomographs.size());
        if (reconstructed_tomographs.size() > 0) {
            if (reconstructed_tomographs[0].shape()[0] != resolution) {
                std::cerr << "The shape of result is inconsistent to resolution " << resolution;
                exit(-1);
            }
            param_ptr->set_num_results(reconstructed_tomographs.size());
            param_ptr->set_done(true);
            param_ptr->set_resolution(reconstructed_tomographs[0].shape()[0]);
        } else {
            param_ptr->set_done(false);
            param_ptr->set_resolution(0);
        }
        for (size_t i = 0; i < reconstructed_tomographs.size(); ++i) {
            for (size_t j = 0; j < reconstructed_tomographs[i].data().size(); ++j) {
                param_ptr->add_reconstructed_tomographs_data(reconstructed_tomographs[i].data()[j]);
            }
        }
    } else {
        param_ptr->set_loaded(false);
        param_ptr->set_done(false);
        param_ptr->set_num_results(0);
    }
    return param_ptr;
}


int ReconTaskParameter::FromProtobuf(
        const recontaskparameter_pb::ReconTaskParameterPB &param_pb)
{
    path_sysmat = QString::fromStdString(param_pb.path_sysmat());
    path_sinogram = QString::fromStdString(param_pb.path_sinogdram());
    task_name = QString::fromStdString(param_pb.task_name());
    sinogram_info = QString::fromStdString(param_pb.sinogram_info());

    lambda = param_pb.lambda();
    gamma = param_pb.gamma();
    coeff_scatter = param_pb.coeff_scatter();
    use_nn = param_pb.use_nn();
    use_scatter_map = param_pb.use_scatter_map();
    num_iters = param_pb.num_iters();
    num_dual_iters = param_pb.has_num_dual_iters();
    num_input_images = param_pb.num_input_images();
    num_slices = param_pb.num_slices();
    num_detectors = param_pb.num_detectors();
    num_angles = param_pb.num_angles();
    resolution = param_pb.resolution();

    file_data_type = param_pb.file_data_type();
    file_format = param_pb.file_format();
    path_model = QString::fromStdString(param_pb.path_model());
    index_sinogram = param_pb.index_sinogram();
    index_projection = param_pb.index_projection();

    iterator_type = IteratorTypePBToStr(param_pb.iterator());
    if (param_pb.has_path_scatter_map()) {
        path_scatter_map = QString::fromStdString(param_pb.path_scatter_map());
    } else {
        path_scatter_map = QString();
    }
    if (param_pb.has_coeff_scatter()) {
        coeff_scatter = param_pb.coeff_scatter();
    } else {
        coeff_scatter = 0.;
    }


    if (param_pb.loaded()) {
        std::vector<double> sino;
        for (int i = 0; i < param_pb.sinogram_data().size(); ++i) {
            sino.push_back(param_pb.sinogram_data(i));
        }
        std::vector<int> shape{param_pb.num_slices(), param_pb.num_angles(), param_pb.num_detectors()};
        sinogram.Set(shape, std::move(sino));
        projection = sinogram.Permute({1, 0, 2});
    }

    if (param_pb.done()) {
        for (int i = 0; i < param_pb.num_results(); ++i) {
            std::vector<double> temp;
            const int offset = i * param_pb.resolution() * param_pb.resolution();
            for (int j = 0; j < param_pb.resolution() * param_pb.resolution(); ++j) {
                temp.push_back(param_pb.reconstructed_tomographs_data(offset + j));
            }
            std::vector<int> shape(2, param_pb.resolution());
            reconstructed_tomographs.push_back(Tensor(shape, std::move(temp)));
        }
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
    auto param_pb = ToProtobuf();

    if (!param_pb->SerializeToOstream(&outstream)) {
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
