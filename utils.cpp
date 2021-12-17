#include "utils.h"


std::vector<QPixmap> GetPixmapArrayFromTensor3D(const Tensor& tensor)
{
    int num_images = tensor.shape()[0];
    int height = tensor.shape()[1];
    int width = tensor.shape()[2];
    std::vector<QPixmap> result;
    for (int i = 0; i < num_images; ++i) {
        QImage image(width, height, QImage::Format_RGB32);
        for (int j = 0; j < height; ++j) {
            for (int k = 0; k < width; ++k) {
                uint8_t value = static_cast<uint8_t>(tensor[{i, j, k}] * 255);
                image.setPixel(k, j, qRgb(value, value, value));
            }
        }
        result.push_back(QPixmap::fromImage(image));
    }
    return result;
}

QPixmap GetPixmapFromTensor2D(const Tensor& tensor)
{
    int height = tensor.shape()[0];
    int width = tensor.shape()[1];
    QImage image(width, height, QImage::Format_RGB32);
    for (int j = 0; j < height; ++j) {
        for (int k = 0; k < width; ++k) {
            uint8_t value = static_cast<uint8_t>(tensor[{j, k}] * 255);
            image.setPixel(k, j, qRgb(value, value, value));
        }
    }
    return QPixmap::fromImage(image);
}

QPixmap GetPixmapFromTensor3D(const Tensor& tensor, int index) {
    int num_images = tensor.shape()[0];
    int height = tensor.shape()[1];
    int width = tensor.shape()[2];
    if (index >= num_images || index < -num_images) {
        std::cerr << "Index (" << index << ") exceeds the number of images (" << num_images << ")." << std::endl;
    }
    if (index < 0) {
        index += num_images;
    }
    QImage image(width, height, QImage::Format_RGB32);
    for (int j = 0; j < height; ++j) {
        for (int k = 0; k < width; ++k) {
            uint8_t value = static_cast<uint8_t>(tensor[{index, j, k}] * 255);
            image.setPixel(k, j, qRgb(value, value, value));
        }
    }
    return QPixmap::fromImage(image);
}
