TEMPLATE = app
TARGET = curvetest
CONFIG   += console

QT += testlib
QT -= gui

LIBS += -lm -llevmar

INCLUDEPATH = ../src

HEADERS += ../src/curvefitter.h
SOURCES += ../src/curvefitter.cpp
HEADERS += ../src/pointerarray.h

HEADERS += curvetest.h
SOURCES += curvetest.cpp
