#include "sinogramfilereader.h"
#include "global_defs.h"

#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/dcmdata/dcdeftag.h"

#include "dcmtk/dcmimgle/dcmimage.h"

SinogramFileReader::SinogramFileReader(const std::string& file_name,
                                       FileFormat format,
                                       int num_slices,
                                       int num_angles,
                                       int num_detectors,
                                       Tensor::FileDataType data_type)
{
    switch (format) {
    case FileFormat::kDicom: {
        ReadDicom_(file_name);
        break;
    }
    case FileFormat::kRawSinogram: {
        std::vector<int> shape{num_slices, num_angles, num_detectors};
        try {
          sinogram_ = Tensor::CreateTensorFromRawFile(file_name, shape, data_type);
        } catch (const Tensor::CannotOpenFileError &e) {
            status_ = Status::kFailToReadFile;
            return;
        } catch (const Tensor::FileLengthError &e) {
            status_ = Status::kFailToParseFile;
            return;
        }
        projection_ = sinogram_.Permute({1, 0, 2});
        break;
    }
    case FileFormat::kRawProjection: {
        std::vector<int> shape{num_angles, num_slices, num_detectors};
        try {
          projection_ = Tensor::CreateTensorFromRawFile(file_name, shape, data_type);
        } catch (const Tensor::CannotOpenFileError &e) {
          status_ = Status::kFailToReadFile;
          return;
        } catch (const Tensor::FileLengthError &e) {
          status_ = Status::kFailToParseFile;
          return;
        }
        sinogram_ = projection_.Permute({1, 0, 2});
        break;
    }
    default:
      status_ = Status::kUnknown;
      return;
    }
    sinogram_.NormalizeInPlace();
    projection_.NormalizeInPlace();
    char buffer[256];
    snprintf(buffer, sizeof(buffer) - 1, "%d", num_slices);
    sinogram_info_.Set("Number of Slices", buffer);
    snprintf(buffer, sizeof(buffer) - 1, "%d", num_detectors);
    sinogram_info_.Set("Number of Detectors", buffer);
    snprintf(buffer, sizeof(buffer) - 1, "%d", num_angles);
    sinogram_info_.Set("Number of Angles", buffer);
    status_ = Status::kOK;
}

void SinogramFileReader::ReadDicom_(const std::string& file_name)
{
    DcmFileFormat dcm_file_format;
    auto status = dcm_file_format.loadFile(file_name.c_str());
    if (status.good()) {
        dcm_file_format.getDataset()->print(std::cout);
        OFString patient_name;
        status = dcm_file_format.getDataset()->findAndGetOFString(DCM_PatientName, patient_name);
        if (status.bad()) {
            std::cerr << "Cannot read Tag PatientName.\n";
            patient_name = "Unknown";
        }
        sinogram_info_.Set("PatientName", patient_name.c_str());

        OFString study_description;
        status = dcm_file_format.getDataset()->findAndGetOFString(DCM_StudyDescription, study_description);
        if (status.bad()) {
            std::cerr << "Cannot read Tag StudyDescription.\n";
            study_description = "Unknown";
        }
        sinogram_info_.Set("StudyDescription", study_description.c_str());

        OFString modality;
        status = dcm_file_format.getDataset()->findAndGetOFString(DCM_Modality, modality);
        if (status.bad()) {
            std::cerr << "Cannot read Tag Modality.\n";
            modality = "Unknown";
        }
        sinogram_info_.Set("Modality", modality.c_str());

    } else {
        std::cerr << "Cannot open file '" << file_name << "'\n";
        status_ = Status::kFailToParseFile;
    }

    DicomImage dicom_image(file_name.c_str());

    int image_width = dicom_image.getWidth();
    int image_height = dicom_image.getHeight();
    int num_frames = dicom_image.getNumberOfFrames();
    if (image_width != kNumDetectors || num_frames != kNumAngles) {
        status_ = Status::kInvalidShape;
        std::cerr << "Invalid shape: (H: " << image_height << ", W: " << image_width << ", N: " << num_frames << ")" << std::endl;
        return;
    }
    std::cout << "Image Shape (W, H, F): (" << image_width << ", " << image_height << ", " << num_frames << ")\n";
    std::vector<int> shape{num_frames, image_height, image_width};
    projection_ = Tensor(shape);
    int depth = dicom_image.getDepth();
    std::cout << "Depth: " << depth << std::endl;
    if (depth == 16) {
        std::vector<uint16_t> data(dicom_image.getOutputDataSize(depth));
        for (int i = 0; i < num_frames; ++i) {
            int ret = dicom_image.getOutputData(data.data(), data.size(), depth, i);
            if (ret == 0) {
                std::cerr << "Cannot read output data.\n";
                status_ = Status::kFailToParseFile;
            }
            std::vector<int> index{i, 0, 0};
            int pixel_count = 0;
            for (int row_i = 0; row_i < image_height; ++row_i) {
                index[1] = row_i;
                for (int col_i = 0; col_i < image_width; ++col_i) {
                    index[2] = col_i;
                    projection_[index] = static_cast<Tensor::DataType>(data[pixel_count]);
                    pixel_count++;
                }
            }
        }
    }
    sinogram_ = projection_.Permute({1, 0, 2});
}
