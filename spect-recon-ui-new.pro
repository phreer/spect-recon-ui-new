QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

PROTOBUF_DIR = /usr/local/Cellar/protobuf/3.17.2
SPECT_LIB_DIR = ../spect-recon/build/lib
SPECT_INCLUDE_DIR = ../spect-recon/include
ONNXRUNTIME_DIR = /usr/local/Cellar/onnxruntime/1.7.2

LIBS += \
    -L$${PROTOBUF_DIR}/lib \
    -lprotobuf \
    -lprotoc \
    -L$${SPECT_LIB_DIR} \
    -lspect \
    -L/usr/local/lib \
    -lpthread \
    -lomp \
    -L$${ONNXRUNTIME_DIR} \
    -lonnxruntime

INCLUDEPATH += \
    $${PROTOBUF_DIR}/include \
    $${SPECT_INCLUDE_DIR} \
    /usr/local/include \
    $${ONNXRUNTIME_DIR}\include

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

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
