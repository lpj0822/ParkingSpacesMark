#-------------------------------------------------
#
# Project created by QtCreator 2017-05-10T17:04:25
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ParkingSpacesMark
TEMPLATE = app

include(utility/utility.pri)

SOURCES += main.cpp\
        controlwindow.cpp \
    editablelabel.cpp

HEADERS  += controlwindow.h \
    editablelabel.h

RESOURCES += \
    style.qrc \
    images.qrc

RC_ICONS = appico.ico

INCLUDEPATH+= D:\opencv\opencv310\build\include \
              D:\opencv\opencv310\build\include\opencv \
              D:\opencv\opencv310\build\include\opencv2 \
              D:\TrackingWithCan1\include

#LIBS+=D:\opencv\opencv310\build\x64\vc12\lib\opencv_world310d.lib
LIBS+=D:\opencv\opencv310\build\x64\vc12\lib\opencv_world310.lib

LIBS+=D:\TrackingWithCan1\lib\apd_detection_and_track.lib
