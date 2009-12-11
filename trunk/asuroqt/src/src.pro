SOURCES += asuroqt.cpp \
           main.cpp \
 sensorplot.cpp
HEADERS += asuroqt.h \
 sensorplot.h
TEMPLATE = app
CONFIG += warn_on \
           qt
TARGET = asuroqt
DESTDIR = ../bin
INCLUDEPATH += /usr/include/qwt/

LIBS += -lqwt

