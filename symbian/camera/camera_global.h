#ifndef CAMERA_GLOBAL_H
#define CAMERA_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(XQCAMERA_LIBRARY)
#  define XQCAMERA_EXPORT Q_DECL_EXPORT
#else
#  if defined(XQCAMERA_NO_LIBRARY)
#    define XQCAMERA_EXPORT
#  else
#    define XQCAMERA_EXPORT Q_DECL_IMPORT
#  endif
#endif

#endif // CAMERA_GLOBAL_H
