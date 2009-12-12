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

asuroqt::asuroqt() : clientSocket(0), tcpReadBlockSize(0)
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

    setupServer();
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

    for (int i=1; i<=6; i++)
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

    linePlot = new CSensorPlot("Line sensors");
    linePlot->addSensor("Left", Qt::red);
    linePlot->addSensor("Right", Qt::yellow);

    hbox->addWidget(linePlot);

    return ret;
}

QWidget *asuroqt::createOdoWidget()
{
    QFrame *ret = new QFrame;
    ret->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    QHBoxLayout *hbox = new QHBoxLayout(ret);

    odoPlot = new CSensorPlot("Odo sensors");
    odoPlot->addSensor("Left", Qt::red);
    odoPlot->addSensor("Right", Qt::yellow);

    hbox->addWidget(odoPlot);

    return ret;
}

QWidget *asuroqt::createBatteryWidget()
{
    QFrame *ret = new QFrame;
    ret->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    QHBoxLayout *hbox = new QHBoxLayout(ret);

    batteryPlot = new CSensorPlot("Battery");
    batteryPlot->addSensor("Battery", Qt::red);

    hbox->addWidget(batteryPlot);

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
    connect(disconnectMapper, SIGNAL(mapped(QObject *)), this,
            SLOT(clientDisconnected(QObject *)));
}

void asuroqt::parseTcpMsg(const QString &msg, quint16 data)
{
    if (msg == "switch")
    {
        for (int i=0; i<6; i++)
        {
            switchList[i]->setChecked((data & (1<<i)));
        }
    }
    else if (msg == "linel")
        linePlot->addData("Left", data);
    else if (msg == "liner")
        linePlot->addData("Right", data);
    else if (msg == "odol")
        odoPlot->addData("Left", data);
    else if (msg == "odor")
        odoPlot->addData("Right", data);
    else if (msg == "battery")
        batteryPlot->addData("Battery", data);
}

void asuroqt::clientConnected()
{
    qDebug("Client connected\n");
    
    if (clientSocket)
        clientSocket->disconnectFromHost();

    clientSocket = tcpServer->nextPendingConnection();
    connect(clientSocket, SIGNAL(disconnected()), disconnectMapper,
            SLOT(map()));
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(clientHasData()));
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
    qDebug() << "clientHasData: " << clientSocket->bytesAvailable() << "\n";
    QDataStream in(clientSocket);
    in.setVersion(QDataStream::Qt_4_5);

    while (true)
    {
        if (tcpReadBlockSize == 0)
        {
            if (clientSocket->bytesAvailable() < (int)sizeof(quint16))
                return;

            in >> tcpReadBlockSize;
        }

        if (clientSocket->bytesAvailable() < tcpReadBlockSize)
            return;

        QString msg;
        quint16 data;
        in >> msg >> data;

        parseTcpMsg(msg, data);
        
        qDebug() << QString("Received msg: %1=%2 (%3 bytes)\n").arg(msg).arg(data).arg(tcpReadBlockSize);

        tcpReadBlockSize = 0;
    }
}
