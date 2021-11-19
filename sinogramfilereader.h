#ifndef SINOGRAMFILEREADER_H
#define SINOGRAMFILEREADER_H
#include <string>
#include <map>
#include <sstream>
#include <QPixmap>
#include <QVector>
#include <QImage>

#include "tensor.h"

class SinogramInfo
{
public:
    std::string GetInfoString() const {
        std::stringstream ss;
        for (const auto& kv: data_)
        {
            ss << kv.first << ": " << kv.second << "\n";
        }
        return ss.str();
    }
    void Set(const std::string& key, const std::string& value)
    {
        data_[key] = value;
    }
private:
    std::map<std::string, std::string> data_;
};

class SinogramFileReader
{
public:
    enum class FileFormat
    {
        kRawProjection, // shape (num_detectors, num_slices, num_angles)
        kRawSinogram, // shape (num_detectors, num_angles, num_slices
        kDicom,
    };
    typedef Tensor::FileDataType FileDataType;
    enum class Status {
        kOK, // Reading procedure completed successfully.
        kFailToReadFile, // Cannot find specified file.
        kFailToParseFile, // Cannot parse the specified file due to, e.g., permission or incorrect file format.
        kInvalidShape, // The shape of the sinogram is invalid.
        kUnknown, // Unknown error occurred.
    };
    /**
     * @brief SinogramFileReader Read sinogram from file.
     * @param file_name
     * @param format the format of the sinogram files
     * @param num_slices ignore for DICOM format, as this is contained in the DICOM file
     */
    SinogramFileReader(const std::string& file_name, FileFormat format, int num_slices,
                       FileDataType data_format = FileDataType::kFloat32);
    const SinogramInfo& GetSinogramInfo() const
    {
        assert (status_ == Status::kOK);
        return sinogram_info_;
    }
    const Tensor& GetSinogram() const
    {
        assert (status_ == Status::kOK);
        return sinogram_;
    }
    const Tensor& GetProjection() const
    {
        assert (status_ == Status::kOK);
        return projection_;
    }
    QVector<QPixmap> GetSinogramPixmap() const {
        int num_images = sinogram_.shape()[0];
        int height = sinogram_.shape()[1];
        int width = sinogram_.shape()[2];
        QVector<QPixmap> result;
        for (int i = 0; i < num_images; ++i) {
            QImage sinogram_image(width, height, QImage::Format_RGB32);
            for (int j = 0; j < height; ++j) {
                for (int k = 0; k < width; ++k) {
                    uint8_t value = static_cast<uint8_t>(sinogram_[{i, j, k}] * 255);
                    sinogram_image.setPixel(k, j, qRgb(value, value, value));
                }
            }
            result.push_back(QPixmap::fromImage(sinogram_image));
        }
        return result;
    }
    QVector<QPixmap> GetProjectionPixmap() const {
        int num_images = projection_.shape()[0];
        int height = projection_.shape()[1];
        int width = projection_.shape()[2];
        QVector<QPixmap> result;
        for (int i = 0; i < num_images; ++i) {
            QImage image(width, height, QImage::Format_RGB32);
            for (int j = 0; j < height; ++j) {
                for (int k = 0; k < width; ++k) {
                    uint8_t value = static_cast<uint8_t>(projection_[{i, j, k}] * 255);
                    image.setPixel(k, j, qRgb(value, value, value));
                }
            }
            result.push_back(QPixmap::fromImage(image));
        }
        return result;
    }
    std::string GetSinogramInfoString() const {
        return sinogram_info_.GetInfoString();
    }
    Status GetStatus() const {
        return status_;
    }
private:
    void ReadDicom_(const std::string& file_name);
    SinogramInfo sinogram_info_;
    Tensor sinogram_;
    Tensor projection_;
    Status status_;
};
#endif // SINOGRAMFILEREADER_H
