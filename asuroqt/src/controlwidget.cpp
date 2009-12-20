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

#include <QDebug>
#include <QGridLayout>
#include <QKeyEvent>
#include <QPushButton>
#include <QStyle>

#include "controlwidget.h"

CControlWidget::CControlWidget(QWidget *parent,
                               Qt::WindowFlags f) : QWidget(parent, f)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setFocusPolicy(Qt::StrongFocus);
    
    QGridLayout *grid = new QGridLayout(this);
    
    buttonMap[FORWARD] = createControlButton(style()->standardIcon(QStyle::SP_ArrowUp));
    grid->addWidget(buttonMap[FORWARD], 0, 1);

    buttonMap[BACK] = createControlButton(style()->standardIcon(QStyle::SP_ArrowDown));
    grid->addWidget(buttonMap[BACK], 1, 1);

    buttonMap[LEFT] = createControlButton(style()->standardIcon(QStyle::SP_ArrowLeft));
    grid->addWidget(buttonMap[LEFT], 1, 0);

    buttonMap[RIGHT] = createControlButton(style()->standardIcon(QStyle::SP_ArrowRight));
    grid->addWidget(buttonMap[RIGHT], 1, 2);
}

QPushButton *CControlWidget::createControlButton(const QIcon &icon)
{
    QPushButton *ret = new QPushButton;
    ret->setIcon(icon);
    ret->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ret->setAutoRepeat(true);
    ret->setAutoRepeatInterval(1000);
    return ret;
}

CControlWidget::EDirection CControlWidget::getDirFromKey(int key)
{
    EDirection ret = NONE;
    
    switch (key)
    {
        case Qt::Key_Up: ret = FORWARD; break;
        case Qt::Key_Down: ret = BACK; break;
        case Qt::Key_Left: ret = LEFT; break;
        case Qt::Key_Right: ret = RIGHT; break;
    }

    return ret;
}

void CControlWidget::updateDirections()
{
    for (TButtonMap::iterator it=buttonMap.begin(); it!=buttonMap.end(); it++)
    {
        const bool pressed = buttonPressMap[it->first];

        if (it->second->isDown() == pressed)
            continue;
        
        it->second->setDown(pressed);
        emit directionChanged();
    }
}

void CControlWidget::keyPressEvent(QKeyEvent *event)
{
    EDirection dir = getDirFromKey(event->key());

    if ((dir == NONE) || event->isAutoRepeat())
    {
        event->ignore();
        QWidget::keyPressEvent(event);
    }
    else
    {
        event->accept();
        buttonPressMap[dir] = true;
        updateDirections();
    }
}

void CControlWidget::keyReleaseEvent(QKeyEvent *event)
{
    EDirection dir = getDirFromKey(event->key());
    
    if ((dir == NONE) || event->isAutoRepeat())
    {
        event->ignore();
        QWidget::keyReleaseEvent(event);
    }
    else
    {
        event->accept();
        buttonPressMap[dir] = false;
        updateDirections();
    }
}
