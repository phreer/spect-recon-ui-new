#ifndef SPECT_RECON_UI_GLOBAL_DEFS_H
#define SPECT_RECON_UI_GLOBAL_DEFS_H

#include <QString>
#include <QDir>

// Constant values used by neural networks for restoration.
const int kNumDetectors = 128;
const int kNumAngles = 120;
const int kNumSlices = 16;

const QString kBaseDir = QDir::home().filePath("spect-recon-data");

#endif // SPECT_RECON_UI_GLOBAL_DEFS_H
