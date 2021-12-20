#ifndef SPECT_RECON_UI_UTILS_H
#define SPECT_RECON_UI_UTILS_H

#include <vector>

#include <QPixmap>
#include <QLabel>
#include <QPixmap>

#include "tensor.h"

void SetLabelImage(QLabel& label, const QPixmap& pixmap);
std::vector<QPixmap> GetPixmapArrayFromTensor3D(const Tensor& tensor);
QPixmap GetPixmapFromTensor3D(const Tensor& tensor, int index);
QPixmap GetPixmapFromTensor2D(const Tensor& tensor);

#endif // SPECT_RECON_UI_UTILS_H
