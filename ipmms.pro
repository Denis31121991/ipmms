#-------------------------------------------------
#
# Project created by QtCreator 2016-09-18T16:13:59
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ipmms
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    configuration.cpp \
    view.cpp \
    pictureworker.cpp \
    controls.cpp

HEADERS  += mainwindow.h \
    configuration.h \
    view.h \
    pictureworker.h \
    controls.h

OPENCVDIR = "C:/Libs/OpenCV/2_4_10/opencv"

CONFIG( release, debug | release ){
    LIBS += -L$${OPENCVDIR}/build/x64/vc10/lib/ \
                -lopencv_core2410 \
                -lopencv_imgproc2410\
                -lopencv_highgui2410
}

CONFIG( debug, debug | release ){
    LIBS += -L$${OPENCVDIR}/build/x64/vc10/lib/ \
                -lopencv_core2410d \
                -lopencv_imgproc2410d\
                -lopencv_highgui2410d
}

INCLUDEPATH += $${OPENCVDIR}/build/include\
               $${OPENCVDIR}/3rdparty/include\

DEPENDPATH += $${OPENCVDIR}/build/x64/vc10/bin\

RESOURCES += \
    resource.qrc
