#-------------------------------------------------
#
# Project created by QtCreator 2016-01-27T20:10:21
#
#-------------------------------------------------

QT       += core gui
CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = excellence
TEMPLATE = app
macx:TARGET = Excellence

SOURCES += main.cpp\
        mainwindow.cpp \
    drawtool.cpp \
    algorithms.cpp \
    tool.cpp \
    pen.cpp \
    pentip.cpp \
    palettebutton.cpp \
    currentcolorsbutton.cpp \
    buffer.cpp \
    bufferview.cpp \
    linetool.cpp \
    undobuffer.cpp \
    cleartool.cpp \
    undotool.cpp \
    zoomtool.cpp

HEADERS  += mainwindow.h \
    drawtool.h \
    algorithms.h \
    tool.h \
    pen.h \
    pentip.h \
    palettebutton.h \
    currentcolorsbutton.h \
    buffer.h \
    bufferview.h \
    linetool.h \
    undobuffer.h \
    cleartool.h \
    undotool.h \
    zoomtool.h

FORMS    += mainwindow.ui \
    bufferview.ui

RESOURCES += \
    icons/icons.qrc

LIBS += -Lplugins
CONFIG(release, debug|release): LIBS += -lilbm
else:CONFIG(debug, debug|release): LIBS += -lilbm_debug
