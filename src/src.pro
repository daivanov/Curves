QT += core gui
TARGET = curves
TEMPLATE = app
CONFIG += debug

SOURCES += main.cpp
HEADERS += pane.h
SOURCES += pane.cpp
HEADERS += curvefitter.h
SOURCES += curvefitter.cpp
HEADERS += pointarray.h
SOURCES += pointarray.cpp

LIBS += -lm -llevmar
