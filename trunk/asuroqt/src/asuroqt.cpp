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

#include <qwt_knob.h>
#include <qwt_slider.h>

#include "asuroqt.h"
#include "controlwidget.h"
#include "sensorplot.h"

asuroqt::asuroqt() : clientSocket(0), tcpReadBlockSize(0)
{
    QWidget *cw = new QWidget;
    setCentralWidget(cw);

    QVBoxLayout *vbox = new QVBoxLayout(cw);

    vbox->addWidget(createSwitchWidget());

    QTabWidget *tabWidget = new QTabWidget;
    vbox->addWidget(tabWidget);

    tabWidget->addTab(createLineWidget(), "Line sensors");
    tabWidget->addTab(createOdoWidget(), "Odo sensors");
    tabWidget->addTab(createBatteryWidget(), "Battery");

    tabWidget = new QTabWidget;
    vbox->addWidget(tabWidget);

    tabWidget->addTab(createControlWidget(), "Control");
    tabWidget->addTab(createMotorWidget(), "Motor control");
    
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
    QWidget *ret = new QWidget;

    QHBoxLayout *hbox = new QHBoxLayout(ret);

    linePlot = new CSensorPlot("Line sensors");
    linePlot->addSensor("Left", Qt::red);
    linePlot->addSensor("Right", Qt::yellow);

    hbox->addWidget(linePlot);

    return ret;
}

QWidget *asuroqt::createOdoWidget()
{
    QWidget *ret = new QWidget;
    
    QHBoxLayout *hbox = new QHBoxLayout(ret);

    odoPlot = new CSensorPlot("Odo sensors");
    odoPlot->addSensor("Left", Qt::red);
    odoPlot->addSensor("Right", Qt::yellow);

    hbox->addWidget(odoPlot);

    return ret;
}

QWidget *asuroqt::createBatteryWidget()
{
    QWidget *ret = new QWidget;
    
    QHBoxLayout *hbox = new QHBoxLayout(ret);

    batteryPlot = new CSensorPlot("Battery");
    batteryPlot->addSensor("Battery", Qt::red);

    hbox->addWidget(batteryPlot);

    return ret;
}

QWidget *asuroqt::createControlWidget()
{
    QWidget *ret = new QWidget;
    
    QHBoxLayout *mainhbox = new QHBoxLayout(ret);

    mainhbox->addWidget(controlWidget = new CControlWidget);
    connect(controlWidget, SIGNAL(directionChanged()), this, SLOT(controlAsuro()));
    
    mainhbox->addWidget(createKnob("<qt><strong>Speed</strong>", controlSpeedKnob));
    
    mainhbox->addWidget(createSlider("<qt><strong>Left</strong>", controlLSlider));
    mainhbox->addWidget(createSlider("<qt><strong>Right</strong>", controlRSlider));

    return ret;
}

QWidget *asuroqt::createMotorWidget()
{
    QWidget *ret = new QWidget;
    
    QHBoxLayout *hbox = new QHBoxLayout(ret);

    QVBoxLayout *vbox = new QVBoxLayout;
    hbox->addLayout(vbox);

    hbox->addWidget(createKnob("<qt><strong>Left</strong>", leftMotorKnob));
    hbox->addWidget(createKnob("<qt><strong>Right</strong>", rightMotorKnob));

    QPushButton *applyB = new QPushButton("Apply");
    applyB->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(applyB, SIGNAL(clicked()), this, SLOT(applyMotors()));
    hbox->addWidget(applyB);

    return ret;
}

QWidget *asuroqt::createKnob(const QString &title, QwtKnob *&knob)
{
    QFrame *ret = new QFrame;
    ret->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    ret->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QVBoxLayout *vbox = new QVBoxLayout(ret);

    QLabel *label = new QLabel(title);
    label->setAlignment(Qt::AlignCenter);
    vbox->addWidget(label);

    vbox->addWidget(knob = new QwtKnob);
    knob->setScale(0.0, 255.0, 51.0);
    knob->setRange(0.0, 255.0, 25.5);
    knob->setTracking(false);
    
    return ret;
}

QWidget *asuroqt::createSlider(const QString &title, QwtSlider *&slider)
{
    QFrame *ret = new QFrame;
    ret->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    ret->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);

    QVBoxLayout *vbox = new QVBoxLayout(ret);

    QLabel *label = new QLabel(title);
    label->setAlignment(Qt::AlignCenter);
    vbox->addWidget(label);

    vbox->addWidget(slider = new QwtSlider(0, Qt::Vertical, QwtSlider::LeftScale));
    slider->setScale(-255.0, 255.0);
    slider->setRange(-255.0, 255.0);
    slider->setReadOnly(true);
    
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

void asuroqt::parseTcpMsg(const QString &msg, qint16 data)
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

void asuroqt::writeTcpMsg(const QString &msg, qint16 data)
{
    if (!clientSocket || (clientSocket->state() != QTcpSocket::ConnectedState))
    {
        qDebug("Cannot send data\n");
        return;
    }
    
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_5);

    out << (quint16)0; // Size
    out << msg;
    out << data;
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));

    clientSocket->write(block);
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
        qint16 data;
        in >> msg >> data;

        parseTcpMsg(msg, data);
        
        qDebug() << QString("Received msg: %1=%2 (%3 bytes)\n").arg(msg).arg(data).arg(tcpReadBlockSize);

        tcpReadBlockSize = 0;
    }
}

void asuroqt::controlAsuro()
{
    const qint16 movespeed = static_cast<qint16>(controlSpeedKnob->value());
    qint16 leftspeed = 0, rightspeed = 0;
    const bool forward = controlWidget->directionPressed(CControlWidget::FORWARD);
    const bool back = controlWidget->directionPressed(CControlWidget::BACK);
    const bool left = controlWidget->directionPressed(CControlWidget::LEFT);
    const bool right = controlWidget->directionPressed(CControlWidget::RIGHT);

    if (forward && back)
        ; // Reset speed
    else if (forward)
    {
        if (left)
            leftspeed = movespeed;
        else if (right)
            rightspeed = movespeed;
        else // both or neither
            leftspeed = rightspeed = movespeed;
    }
    else if (back)
    {
        if (left)
            rightspeed = -movespeed;
        else if (right)
            leftspeed = -movespeed;
        else // both or neither
            leftspeed = rightspeed = -movespeed;
    }

    qDebug() << "Apply: " << leftspeed << ", " << rightspeed << " - " << forward << back << left << right << "\n";

    controlLSlider->setValue(leftspeed);
    controlRSlider->setValue(rightspeed);
    
    writeTcpMsg("leftm", leftspeed);
    writeTcpMsg("rightm", rightspeed);
}

void asuroqt::applyMotors()
{
    qDebug() << "Apply: " << leftMotorKnob->value() << ", " << rightMotorKnob->value() << "\n";
    
    writeTcpMsg("leftm", static_cast<qint16>(leftMotorKnob->value()));
    writeTcpMsg("rightm", static_cast<qint16>(rightMotorKnob->value()));
}
