#ifndef RECONTASKPARAMETER_H
#define RECONTASKPARAMETER_H

#include <QString>
#include <QVector>
#include <fstream>
#include <memory>

#include "recontaskparameter.pb.h"
#include "tensor.h"


struct ReconTaskParameter
{
    ReconTaskParameter() {
        const char *env_path_model = getenv("MODEL_PATH");
        if (env_path_model && strlen(env_path_model)) {
            path_model = QString::fromStdString(env_path_model);
        } else {
            const char *env_appdir = getenv("APPDIR");
            if (env_appdir && strlen(env_appdir)) {
                path_model = QString::fromStdString(env_appdir) + "/usr/share/model/ckpt_e40_0_p25.2163251814763.pth.onnx";
            } else {
                path_model = "ckpt_e40_0_p25.2163251814763.pth.onnx";
            }
        }
    }
    typedef recontaskparameter_pb::ReconTaskParameterPB_FileDataType FileDataType;
    typedef recontaskparameter_pb::ReconTaskParameterPB_FileFormat FileFormat;
    // I/O parameters
    QString task_name = "untitled_task";
    QString path_sysmat;
    QString path_sinogram;
    QString path_mumap; // Path to the attenuation map.
    QString path_scatter_map;
    QString path_model; // Path to the neural network model for scatter correction.
    QString output_dir;
    QString output_name;

    QString sinogram_info;

    // Algorithmic parameters
    QString iterator_type = "MLEM"; // name of the reconstructing iterator
    double lambda = 0.1;
    double gamma = 0.01;
    double coeff_scatter = 0.5;
    uint num_iters = 100;
    uint num_dual_iters = 1;

    // The parameters below is unused for now
    // Numerical parameters
    int num_bases = 4;

    // Device parameters
    int num_detectors = 128; // The number of cameras
    int num_angles = 120;
    int num_slices = 64;

    // Use neural networks to perform restoration and scatter correction?
    bool use_nn = false;;

    bool use_scatter_map = false;

    FileDataType file_data_type = FileDataType::ReconTaskParameterPB_FileDataType_FLOAT32;
    FileFormat file_format = FileFormat::ReconTaskParameterPB_FileFormat_RAW_PROJECTION;

    std::vector<Tensor> reconstructed_tomographs;
    Tensor projection;
    Tensor sinogram;
    int num_input_images;

    int index_sinogram = 0;
    int index_projection = 0;
    int resolution = 128;
    // Load task parameter from given protobuf object `param_pb`.
    int FromProtobuf(const recontaskparameter_pb::ReconTaskParameterPB &param_pb);
    // Load task parameter from protobuf object that will be constructed from file
    // specified by `path`.
    int FromProtobufFilePath(const QString &path);

    std::shared_ptr<recontaskparameter_pb::ReconTaskParameterPB> ToProtobuf() const;
    int ToProtobufFilePath(const QString &path) const;

    static recontaskparameter_pb::ReconTaskParameterPB_IteratorType
    IteratorTypeStrToPB(const QString &iteratorType);

    static QString IteratorTypePBToStr(
            recontaskparameter_pb::ReconTaskParameterPB_IteratorType type_pb);
};

#endif // RECONTASKPARAMETER_H
