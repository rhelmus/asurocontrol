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

#ifndef CONTROLWIDGET_H
#define CONTROLWIDGET_H

#include <map>

#include <QWidget>

class QIcon;
class QPushButton;

class CControlWidget: public QWidget
{
    Q_OBJECT

public:
    enum EDirection { FORWARD, BACK, LEFT, RIGHT, NONE };
    
private:
    typedef std::map<EDirection, QPushButton *> TButtonMap;
    TButtonMap buttonMap;

    typedef std::map<EDirection, bool> TButtonPressMap;
    TButtonPressMap buttonPressMap;

    QPushButton *createControlButton(const QIcon &icon);
    EDirection getDirFromKey(int key);
    void updateDirections(void);
            
protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    
public:
    CControlWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);

    bool directionPressed(EDirection dir) { return buttonPressMap[dir]; }

signals:
    void directionChanged(void);
};

#endif
