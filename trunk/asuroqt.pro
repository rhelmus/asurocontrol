TEMPLATE = app
TARGET = asuroqt
QT += core \
    gui \
    network
HEADERS += utils.h \
    CIRIO.h \
    asuroqt.loc \
    asuroqt.h
SOURCES += CIRIO.cpp \
    asuroqt.rss \
    asuroqt_reg.rss \
    main.cpp \
    asuroqt.cpp
FORMS += asuroqt.ui
RESOURCES += 
MMP_RULES += "LIBRARY c32.lib bafl.lib"
symbian:TARGET.UID3 = 0xE5A258F1
