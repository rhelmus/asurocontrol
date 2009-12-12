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
#include <QtNetwork>

#include "asuroqt.h"
#include "sensorplot.h"

asuroqt::asuroqt() : clientSocket(0)
{
    QWidget *cw = new QWidget;
    setCentralWidget(cw);

    QVBoxLayout *vbox = new QVBoxLayout(cw);
    vbox->addWidget(createSwitchWidget());

    QHBoxLayout *hbox = new QHBoxLayout;
    vbox->addLayout(hbox);

    hbox->addWidget(createLineWidget());
    hbox->addWidget(createOdoWidget());

    vbox->addWidget(createBatteryWidget());
}

asuroqt::~asuroqt()
{
}

QWidget *asuroqt::createSwitchWidget()
{
    QWidget *ret = new QWidget;
    QHBoxLayout *mainhbox = new QHBoxLayout(ret);
    
    QFrame *frame = new QFrame;
    frame->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
    frame->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    mainhbox->addWidget(frame);

    QVBoxLayout *vbox = new QVBoxLayout(frame);
    
    QLabel *label = new QLabel("<qt><h2>Touch sensors</h2>");
    label->setAlignment(Qt::AlignCenter);
    vbox->addWidget(label);

    QHBoxLayout *hbox = new QHBoxLayout;
    vbox->addLayout(hbox);

    for (int i=0; i<6; i++)
    {
        QCheckBox *check = new QCheckBox(QString::number(i));
        check->setEnabled(false);
        switchList.push_back(check);
        hbox->addWidget(check);
    }

    return ret;
}

QWidget *asuroqt::createLineWidget()
{
    QFrame *ret = new QFrame;
    ret->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    QHBoxLayout *hbox = new QHBoxLayout(ret);

    CSensorPlot *plot = new CSensorPlot("Line sensors");
    plot->addSensor("Left", Qt::red);
    plot->addSensor("Right", Qt::yellow);

    for (int i=0; i<3; i++)
    {
        plot->addData("Left", i, (float)i * 2.0);
        plot->addData("Right", i, (float)i * 1.75);
    }
    
    hbox->addWidget(plot);

    return ret;
}

QWidget *asuroqt::createOdoWidget()
{
    QFrame *ret = new QFrame;
    ret->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    QHBoxLayout *hbox = new QHBoxLayout(ret);

    CSensorPlot *plot = new CSensorPlot("Odo sensors");
    plot->addSensor("Left", Qt::red);
    plot->addSensor("Right", Qt::yellow);

    for (int i=0; i<3; i++)
    {
        plot->addData("Left", i, (float)i * 2.0);
        plot->addData("Right", i, (float)i * 1.75);
    }
    
    hbox->addWidget(plot);

    return ret;
}

QWidget *asuroqt::createBatteryWidget()
{
    QFrame *ret = new QFrame;
    ret->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    QHBoxLayout *hbox = new QHBoxLayout(ret);

    CSensorPlot *plot = new CSensorPlot("Battery");
    plot->addSensor("Battery", Qt::red);

    for (int i=0; i<3; i++)
    {
        plot->addData("Battery", i, (float)i * 2.0);
    }
    
    hbox->addWidget(plot);

    return ret;
}

void asuroqt::setupServer()
{
    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen(QHostAddress::Any, 40000))
    {
        QMessageBox::critical(this, "Server error", QString("Failed to start server: %1\n").
            arg(tcpServer->errorString()));
        close();
        return;
    }

    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(clientConnected()));

    disconnectMapper = new QSignalMapper(this);
    connect(disconnectMapper, SIGNAL(mapped(const QObject *)), this,
            SLOT(clientDisconnected(QObject *)));
}

void asuroqt::clientConnected()
{
    if (clientSocket)
        clientSocket->disconnectFromHost();

    clientSocket = tcpServer->nextPendingConnection();
    connect(clientSocket, SIGNAL(disconnected()), disconnectMapper,
            SLOT(map()));
}

void asuroqt::clientDisconnected(QObject *obj)
{
    // Current client?
    if (clientSocket == obj)
        clientSocket = 0;

    obj->deleteLater();
}

void asuroqt::clientHasData()
{
    
}
