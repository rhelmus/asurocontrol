HEADERS += camera_global.h \
    xqcamera.h \
    xqcamera_p.h \
    xqviewfinderwidget.h \
    xqviewfinderwidget_p.h
SOURCES += xqcamera.cpp \
    xqcamera_p.cpp \
    xqviewfinderwidget.cpp \
    xqviewfinderwidget_p.cpp
    
!exists($${EPOCROOT}epoc32/include/cameraengine.h):message("Warning: CameraEngine not found! Extract camerawrapper_epoc32.zip under epocroot.")
symbian:LIBS += -lcamerawrapper \
    -lfbscli
