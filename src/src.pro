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
    palettequantizer.cpp \
    propertiesdialog.cpp \
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
    zoomtool.cpp \
    palettetool.cpp \
    brushtool.cpp \
    brush.cpp \
    rectangletool.cpp \
    curvetool.cpp \
    ellipsetool.cpp \
    airtool.cpp \
    filltool.cpp \
    texttool.cpp \
    pentiptool.cpp \
    drawmodetool.cpp

HEADERS  += mainwindow.h \
    drawtool.h \
    algorithms.h \
    palettequantizer.h \
    propertiesdialog.h \
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
    zoomtool.h \
    palettetool.h \
    brushtool.h \
    brush.h \
    rectangletool.h \
    curvetool.h \
    ellipsetool.h \
    airtool.h \
    filltool.h \
    texttool.h \
    pentiptool.h \
    drawmodetool.h \
    spatial_color_quant.h

FORMS    += mainwindow.ui \
    bufferview.ui \
    propertiesdialog.ui

RESOURCES += \
    icons/icons.qrc

LIBS += -Lplugins/ilbm -Lplugins/raw
CONFIG(release, debug|release): LIBS += -lilbm -lraw
else:CONFIG(debug, debug|release): LIBS += -lilbm_debug -lraw_debug
