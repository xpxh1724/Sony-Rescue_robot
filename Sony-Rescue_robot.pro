#-------------------------------------------------
#
# Project created by QtCreator 2023-11-09T14:11:35
#
#-------------------------------------------------

QT       += core gui mqtt network  webenginewidgets webchannel
#QT += axcontainer #加载地图
QT +=multimediawidgets multimedia
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Sony-RescueRobot
TEMPLATE = app
RC_ICONS = logo.ico

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += C++11

SOURCES += \
        biaozhuwidget.cpp \
        gifthread.cpp \
        log.cpp \
        main.cpp \
        mainwindow.cpp \
        mapbridge.cpp \
        menu.cpp \
        systemstate.cpp \
        videothread.cpp \
        waterbottom.cpp \
        watertop.cpp

HEADERS += \
        biaozhuwidget.h \
        gifthread.h \
        log.h \
        mainwindow.h \
        mapbridge.h \
        menu.h \
        systemstate.h \
        videothread.h \
        waterbottom.h \
        watertop.h

FORMS += \
        biaozhuwidget.ui \
        log.ui \
        mainwindow.ui \
        menu.ui \
        systemstate.ui \
        waterbottom.ui \
        watertop.ui

#指定可执行文件目录 放到这里省了拷贝动态库动作 专为小白懒人考虑
DESTDIR     = $$PWD/exe

#-----------------------------加载opencv库文件---------------------------------------
LIBS += $$PWD/lib/opencv_world454.lib
LIBS += -L$$PWD/bin/
INCLUDEPATH += $$PWD/include
#解决中文乱码问题
msvc{
QMAKE_CXXFLAGS += -utf-8
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
