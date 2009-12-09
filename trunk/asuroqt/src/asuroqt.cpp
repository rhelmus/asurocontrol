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


#include <QtGui>

#include "asuroqt.h"

asuroqt::asuroqt()
{
    QWidget *cw = new QWidget;
    setCentralWidget(cw);

    QVBoxLayout *vbox = new QVBoxLayout(cw);

    QHBoxLayout *hbox = new QHBoxLayout;
    vbox->addLayout(hbox);

    hbox->addWidget(createSwitchWidget());
}

asuroqt::~asuroqt()
{
}

QWidget *asuroqt::createSwitchWidget()
{
    QFrame *ret = new QFrame;
    ret->setFrameStyle(QFrame::Box | QFrame::Sunken);

    QHBoxLayout *hbox = new QHBoxLayout(ret);

    for (int i=0; i<8; i++)
    {
        QCheckBox *check = new QCheckBox(QString::number(i));
        check->setEnabled(false);
        switchList.push_back(check);
        hbox->addWidget(check);
    }

    switchList.back()->setChecked(true);
    
    return ret;
}
