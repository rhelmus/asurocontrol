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
class QSignalMapper;
class QTcpServer;
class QTcpSocket;

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

    QWidget *createSwitchWidget(void);
    QWidget *createLineWidget(void);
    QWidget *createOdoWidget(void);
    QWidget *createBatteryWidget(void);

    void setupServer(void);
    void parseTcpMsg(const QString &msg, quint16 data);

private slots:
    void clientConnected(void);
    void clientDisconnected(QObject *obj);
    void clientHasData(void);
    
public:
    asuroqt();
    ~asuroqt();
};

#endif
