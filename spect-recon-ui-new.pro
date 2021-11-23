QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

​​​​​​​DEFINES += ENABLE_PRECOMPILED_HEADERS=OFF
QMAKE_CFLAGS_ISYSTEM = -I

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

PROTOBUF_INCLUDE_DIR = $$(HOME)/local/include
PROTOBUF_LIB_DIR = $$(HOME)/local/lib

SPECT_OMP_INCLUDE_DIR = ../spect-recon/include
SPECT_OMP_LIB_DIR = $$(HOME)/local/lib

SPECT_NO_OMP_INCLUDE_DIR = ../spect-recon/include
SPECT_NO_OMP_LIB_DIR = $$(HOME)/local/lib

DCMTK_INCLUDE_DIR = /usr/include
DCMTK_LIB_DIR = /usr/lib

ONNXRUNTIME_INCLUDE_DIR = $$(HOME)/local/include/onnxruntime
ONNXRUNTIME_LIB_DIR = $$(HOME)/local/lib

# Set SPECT_TYPE to "OMP" for parallelized (OpenMP) version and "NO_OMP" for plain version.
SPECT_USE_OMP = "yes"
USE_APPIMAGE = "yes"


equals(SPECT_USE_OMP, "yes") {
    SPECT_INCLUDE_DIR = $$SPECT_OMP_INCLUDE_DIR
    SPECT_LIB_DIR = $$SPECT_OMP_LIB_DIR
} else {
equals(SPECT_USE_OMP, "no") {
    SPECT_INCLUDE_DIR = $$SPECT_NO_OMP_INCLUDE_DIR
    SPECT_LIB_DIR = $$SPECT_NO_OMP_LIB_DIR
} else {
    error("SPECT_USE_OMP should be yes or no.")
}
}

equals(USE_APPIMAGE, "yes") {
    DEFINES += USE_APPIMAGE
}

macx: {
    PROTOBUF_INCLUDE_DIR = /usr/local/Cellar/protobuf/3.17.2/include/
    PROTOBUF_LIB_DIR = /usr/local/Cellar/protobuf/3.17.2/lib/

    ONNXRUNTIME_INCLUDE_DIR = /usr/local/Cellar/onnxruntime/1.7.2/include/onnxruntime/core/session
    ONNXRUNTIME_LIB_DIR = /usr/local/Cellar/onnxruntime/1.7.2/lib
}

# QMAKE_CXXFLAGS += -fopenmp

LIBS += \
    -L$${PROTOBUF_LIB_DIR} \
    -lprotobuf \
    -lprotoc \
    -L$${SPECT_LIB_DIR} \
    -lspect \
    -L$${ONNXRUNTIME_LIB_DIR} \
    -lonnxruntime \
    -lpthread \
    -L$${DCMTK_LIB_DIR} \
    -ldcmdata -ldcmimgle -ldcmdata -loflog -lofstd -lz -licuuc
message($${LIBS})

INCLUDEPATH += \
    $${PROTOBUF_INCLUDE_DIR} \
    $${SPECT_INCLUDE_DIR} \
    /usr/local/include \
    $${ONNXRUNTIME_INCLUDE_DIR} \
    $${DCMTK_INCLUDE_DIR}

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    recontask.cpp \
    recontaskparameter.cpp \
    recontaskparameter.pb.cc \
    reconthread.cpp \
    resultdialog.cpp \
    resultwindow.cpp \
    sinogramfilereader.cpp \
    tensor.cpp \
    utils.cpp

HEADERS += \
    error_code.h \
    global_defs.h \
    mainwindow.h \
    recontask.h \
    recontaskparameter.h \
    recontaskparameter.pb.h \
    reconthread.h \
    resultdialog.h \
    resultwindow.h \
    scascnet.h \
    sinogram.h \
    sinogramfilereader.h \
    tensor.h \
    utils.h

FORMS += \
    mainwindow.ui \
    resultdialog.ui

TRANSLATIONS += \
    spect-recon-ui-new_zh_CN.ts

CONFIG += lrelease
CONFIG += embed_translations

PREFIX = /usr

target.path = $${PREFIX}/bin
INSTALLS += target

icon.files = spect-recon-ui-new.png
icon.path = $$PREFIX/share/pixmaps
INSTALLS += icon

shortcut.files = spect-recon-ui-new.desktop
shortcut.path = $$PREFIX/share/applications
INSTALLS += shortcut

model_ckpt.files = ckpt_e40_0_p25.2163251814763.pth
model_ckpt.path = $$PREFIX/share/model
INSTALLS += model_ckpt

DISTFILES += \
    recontaskparameter.proto \
    spect-recon-ui-new.desktop \
    spect-recon-ui-new.png \
    ckpt_e40_0_p25.2163251814763.pth.onnx
