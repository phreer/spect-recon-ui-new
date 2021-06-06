#ifndef RECONTASKPARAMETER_H
#define RECONTASKPARAMETER_H

#include <QString>
#include <fstream>
#include <memory>
#include "recontaskparameter.pb.h"

struct ReconTaskParameter
{
    // I/O parameters
    QString taskName = "untitled_task";
    QString pathSysMat;
    QString pathSinogram;
    QString pathMuMap; // Path of attenuation map
    QString pathScatterMap;
    QString outputDir;
    QString outputName;

    // Algorithmic parameters
    QString iteratorType = "MLEM"; // name of the reconstructing iterator
    double lambda = 0.1;
    double gamma = 0.01;
    double paramScatter = 0.5;
    uint numIters = 100;
    uint numDualIters = 1;

    // The parameters below is unused for now
    // Numerical parameters
    uint numBases;

    // Device parameters
    uint numCams = 128; // The number of cameras

    // Use neural networks to perform restoration and scatter correction?
    bool useNN = false;;

    bool useScatterMap = false;

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
