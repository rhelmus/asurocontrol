/***************************************************************************
 *   Copyright (C) 2009 by Rick Helmus   *
 *   rhelmus_AT_gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <QPainter>
#include <QPixmap>
#include <QResizeEvent>

#include "camwidget.h"

// Code based on http://wiki.forum.nokia.com/index.php/CS001347_-_Scaling_QPixmap_image

CCamWidget::CCamWidget(QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f)
{
    setMinimumSize(250, 100);
}

void CCamWidget::paintEvent(QPaintEvent *)
{
    if (cameraPixmap.isNull())
        return;

    QPainter painter(this);
    QPoint centerPoint(0,0);
    QPixmap scaledPixmap = cameraPixmap.scaled(widgetSize, Qt::KeepAspectRatio);
    centerPoint.setX((widgetSize.width()-scaledPixmap.width()) / 2);
    centerPoint.setY((widgetSize.height()-scaledPixmap.height()) / 2);
    painter.drawPixmap(centerPoint, scaledPixmap);
}

void CCamWidget::resizeEvent(QResizeEvent *event)
{
    widgetSize = event->size();
    QWidget::resizeEvent(event);
}

void CCamWidget::loadPixmap(const QPixmap &pixmap)
{
    cameraPixmap = pixmap;
    update();
}
