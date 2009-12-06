/*
 * utils.h
 *
 *  Created on: 5-dec-2009
 *      Author: Rick
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <QString>

inline QString toQString(const TDes &src) { return QString((QChar *)(src.Ptr()), src.Length()); }
inline QString toQString(const TDesC &src) { return QString((QChar *)(src.Ptr()), src.Length()); }

#endif /* UTILS_H_ */
