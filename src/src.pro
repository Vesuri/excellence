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
    bufferview.cpp

HEADERS  += mainwindow.h \
    drawtool.h \
    algorithms.h \
    tool.h \
    pen.h \
    pentip.h \
    palettebutton.h \
    currentcolorsbutton.h \
    buffer.h \
    bufferview.h

FORMS    += mainwindow.ui \
    imagewindow.ui

RESOURCES += \
    icons/icons.qrc
