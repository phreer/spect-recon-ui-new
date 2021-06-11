QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

PROTOBUF_INCLUDE_DIR = $$(HOME)/local/include
PROTOBUF_LIB_DIR = $$(HOME)/local/lib
SPECT_INCLUDE_DIR = ../spect-recon/include
SPECT_LIB_DIR = $$(HOME)/local/lib

ONNXRUNTIME_INCLUDE_DIR = $$(HOME)/local/onnxruntime-linux-x64-1.8.0/include
ONNXRUNTIME_LIB_DIR = $$(HOME)/local/lib

QMAKE_CXXFLAGS += -fopenmp

LIBS += \
    -L$${PROTOBUF_LIB_DIR} \
    -lprotobuf \
    -lprotoc \
    -L$${SPECT_LIB_DIR} \
    -lspect \
    -L$${ONNXRUNTIME_LIB_DIR} \
    -lonnxruntime \
    -lpthread \
    -fopenmp

message($${LIBS})

INCLUDEPATH += \
    $${PROTOBUF_INCLUDE_DIR} \
    $${SPECT_INCLUDE_DIR} \
    /usr/local/include \
    $${ONNXRUNTIME_INCLUDE_DIR}

message($${INCLUDEPATH})
SOURCES += \
    main.cpp \
    mainwindow.cpp \
    recontaskparameter.cpp \
    recontaskparameter.pb.cc \
    reconthread.cpp

HEADERS += \
    error_code.h \
    mainwindow.h \
    recontaskparameter.h \
    recontaskparameter.pb.h \
    reconthread.h \
    scascnet.h \
    sinogram.h

FORMS += \
    mainwindow.ui

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
    spect-recon-ui-new.desktop \
    spect-recon-ui-new.png \
    ckpt_e40_0_p25.2163251814763.pth.onnx
