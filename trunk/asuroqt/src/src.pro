SOURCES += asuroqt.cpp \
           main.cpp \
 sensorplot.cpp \
 controlwidget.cpp \
 camwidget.cpp
HEADERS += asuroqt.h \
 sensorplot.h \
 controlwidget.h \
 camwidget.h
TEMPLATE = app
CONFIG += warn_on \
           qt
TARGET = asuroqt
DESTDIR = ../bin
INCLUDEPATH += /usr/include/qwt/

LIBS += -lqwt

QT += network

