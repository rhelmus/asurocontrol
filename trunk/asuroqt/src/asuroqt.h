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


#ifndef ASUROQT_H
#define ASUROQT_H

#include <vector>

#include <QMainWindow>

class QCheckBox;
class QIcon;
class QKeyEvent;
class QSignalMapper;
class QTcpServer;
class QTcpSocket;

class QwtKnob;
class QwtSlider;

class CControlWidget;
class CSensorPlot;

class asuroqt: public QMainWindow
{
    Q_OBJECT

    typedef std::vector<QCheckBox *> TSwitchList;
    TSwitchList switchList;
    
    QTcpServer *tcpServer;
    QTcpSocket *clientSocket;
    QSignalMapper *disconnectMapper;
    quint16 tcpReadBlockSize;

    CSensorPlot *linePlot, *odoPlot, *batteryPlot;
    QwtKnob *leftMotorKnob, *rightMotorKnob, *controlSpeedKnob;
    CControlWidget *controlWidget;
    QwtSlider *controlLSlider, *controlRSlider;

    QWidget *createSwitchWidget(void);
    QWidget *createLineWidget(void);
    QWidget *createOdoWidget(void);
    QWidget *createBatteryWidget(void);
    QWidget *createControlWidget(void);
    QWidget *createMotorWidget(void);

    QWidget *createKnob(const QString &title, QwtKnob *&knob);
    QWidget *createSlider(const QString &title, QwtSlider *&slider);

    void setupServer(void);
    void parseTcpMsg(const QString &msg, qint16 data);
    void writeTcpMsg(const QString &msg, qint16 data);

private slots:
    void clientConnected(void);
    void clientDisconnected(QObject *obj);
    void clientHasData(void);
    void controlAsuro(void);
    void applyMotors(void);

public:
    asuroqt();
    ~asuroqt();
};

#endif
