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
#include "camwidget.h"
#include "controlwidget.h"
#include "sensorplot.h"
#include "tcputil.h"

asuroqt::asuroqt() : clientSocket(0), tcpReadBlockSize(0)
{
//     QWidget *cw = new QWidget;
//     setCentralWidget(cw);

    QTabWidget *mainTabWidget = new QTabWidget;
    mainTabWidget->setTabPosition(QTabWidget::West);
    setCentralWidget(mainTabWidget);

    QWidget *w = new QWidget;
    mainTabWidget->addTab(w, "Main");
    QVBoxLayout *vbox = new QVBoxLayout(w);

    vbox->addWidget(createSwitchWidget());

    QSplitter *splitter = new QSplitter(Qt::Vertical);
    vbox->addWidget(splitter);
    
    QTabWidget *tabWidget = new QTabWidget;
    splitter->addWidget(tabWidget);

    tabWidget->addTab(createLineWidget(), "Line sensors");
    tabWidget->addTab(createOdoWidget(), "Odo sensors");
    tabWidget->addTab(createBatteryWidget(), "Battery");

    
    QSplitter *subsplit = new QSplitter(Qt::Horizontal);
    splitter->addWidget(subsplit);
    
    subsplit->addWidget(createSmallCamWidget());

    subsplit->addWidget(tabWidget = new QTabWidget);
    tabWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    tabWidget->addTab(createControlWidget(), "Control");
    tabWidget->addTab(createMotorWidget(), "Motor control");
    tabWidget->addTab(createCamControlWidget(), "Camera control");
    
    mainTabWidget->addTab(createBigCamWidget(), "Camera");

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

QWidget *asuroqt::createCamControlWidget()
{
    QWidget *ret = new QWidget;
    
    QHBoxLayout *hbox = new QHBoxLayout(ret);
    hbox->setSpacing(20);
    
    QGridLayout *grid = new QGridLayout;
    grid->setHorizontalSpacing(2);
    hbox->addLayout(grid);
    
    grid->addWidget(new QLabel("Show frame every"), 0, 0);
    grid->addWidget(camFrameSpinBox = new QSpinBox, 0, 1);
    camFrameSpinBox->setMinimum(25);
    camFrameSpinBox->setMaximum(60000);
    camFrameSpinBox->setSingleStep(250);
    camFrameSpinBox->setValue(1000);
    camFrameSpinBox->setSuffix(" ms");
    
    grid->addWidget(new QLabel("Camera view angle"), 1, 0);
    grid->addWidget(camAngleSpinBox = new QSpinBox, 1, 1);
    camAngleSpinBox->setMaximum(315);
    camAngleSpinBox->setSingleStep(45);
    camAngleSpinBox->setValue(0);
    camAngleSpinBox->setWrapping(true);
    
    grid->addWidget(new QLabel("Picture size"), 2, 0);
    grid->addWidget(camPictureSize = new QComboBox, 2, 1);
    camPictureSize->addItems(QStringList() << "1600x1200" << "1152x864" << "640x480" << "320x240");

    grid->addWidget(new QLabel("Camera exposure"), 3, 0);
    grid->addWidget(camExposureCombo = new QComboBox, 3, 1);
    camExposureCombo->addItems(QStringList() << "Auto" << "Night" << "Backlight" << "Center");
    
    grid->addWidget(new QLabel("Cam white balance"), 0, 2);
    grid->addWidget(camWhiteBalanceCombo = new QComboBox, 0, 3);
    camWhiteBalanceCombo->addItems(QStringList() << "Auto" << "Daylight" << "Tungsten" << "Fluorescent");
    
    grid->addWidget(new QLabel("Camera zoom"), 1, 2);
    QHBoxLayout *zhbox = new QHBoxLayout;
    grid->addLayout(zhbox, 1, 3);
    QPushButton *button = new QPushButton("<<");
    button->setAutoRepeat(true);
    button->setAutoRepeatDelay(25);
    button->setAutoRepeatInterval(75);
    connect(button, SIGNAL(clicked()), this, SLOT(zoomCameraOut()));
    zhbox->addWidget(button);
    zhbox->addWidget(button = new QPushButton(">>"));
    button->setAutoRepeat(true);
    button->setAutoRepeatDelay(25);
    button->setAutoRepeatInterval(75);
    connect(button, SIGNAL(clicked()), this, SLOT(zoomCameraIn()));
    
    grid->addWidget(new QLabel("jpeg quality"), 2, 2);
    grid->addWidget(jpegQualitySlider = new QSlider(Qt::Horizontal), 2, 3);
    jpegQualitySlider->setTickPosition(QSlider::TicksBelow);
    jpegQualitySlider->setTickInterval(10);
    jpegQualitySlider->setRange(1, 100);
    jpegQualitySlider->setSingleStep(10);
    jpegQualitySlider->setValue(50);
    
    
    QVBoxLayout *vbox = new QVBoxLayout;
    hbox->addLayout(vbox);
    
    vbox->addWidget(button = new QPushButton("Apply settings"));
    connect(button, SIGNAL(clicked()), this, SLOT(applyCameraControl()));
    
    vbox->addWidget(button = new QPushButton("Toggle camera"));
    connect(button, SIGNAL(clicked()), this, SLOT(toggleCamera()));

    vbox->addWidget(button = new QPushButton("Take picture"));
    connect(button, SIGNAL(clicked()), this, SLOT(takePicture()));
    
    return ret;
}

QWidget *asuroqt::createSmallCamWidget()
{
    QFrame *ret = new QFrame;
    ret->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    QVBoxLayout *vbox = new QVBoxLayout(ret);

    QLabel *label = new QLabel("<qt><h2>Camera</h2>");
    label->setAlignment(Qt::AlignCenter);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    vbox->addWidget(label);
    
    vbox->addWidget(smallCameraWidget = new CCamWidget);

    return ret;
}

QWidget *asuroqt::createBigCamWidget()
{
    QWidget *ret = new QWidget;

    QVBoxLayout *vbox = new QVBoxLayout(ret);

    vbox->addWidget(cameraWidget = new CCamWidget);

    QPushButton *button = new QPushButton("Take picture");
    connect(button, SIGNAL(clicked()), this, SLOT(takePicture()));
    vbox->addWidget(button);

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

bool asuroqt::canSendTcp(void) const
{
    return (clientSocket && (clientSocket->state() == QTcpSocket::ConnectedState));
}

void asuroqt::parseTcp(QDataStream &stream)
{
    QString msg;
    stream >> msg;

    if (msg == "camera")
    {
        QByteArray data;
        stream >> data;
        showCamera(data);
    }
    else if (msg == "camframe")
    {
        QImage data;
        stream >> data;
        showFrame(data);
    }
    else
    {
        // Message with qint16 data
        qint16 data;
        stream >> data;
        
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
    
    qDebug() << QString("Received msg: %1 (%2 bytes)\n").arg(msg).arg(tcpReadBlockSize);
}

void asuroqt::writeTcpMsg(const QString &msg, qint16 data)
{
    if (!canSendTcp())
        return;

    CTcpWriter tcpWriter(clientSocket);
    tcpWriter << msg;
    tcpWriter << data;
    tcpWriter.write();
}

void asuroqt::showCamera(QByteArray &data)
{
    QPixmap pm;
    pm.loadFromData(data);
    cameraWidget->loadPixmap(pm);
    smallCameraWidget->loadPixmap(pm);
}

void asuroqt::showFrame(QImage &data)
{
    smallCameraWidget->loadPixmap(QPixmap::fromImage(data));
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
//     qDebug() << "clientHasData: " << clientSocket->bytesAvailable() << "\n";
    QDataStream in(clientSocket);
    in.setVersion(QDataStream::Qt_4_5);

    while (true)
    {
        if (tcpReadBlockSize == 0)
        {
            if (clientSocket->bytesAvailable() < (int)sizeof(quint32))
                return;

            in >> tcpReadBlockSize;
        }

        if (clientSocket->bytesAvailable() < tcpReadBlockSize)
            return;

        parseTcp(in);
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

void asuroqt::applyCameraControl()
{
    if (!canSendTcp())
        return;

    // Frame delay
    writeTcpMsg("framedelay", static_cast<qint16>(camFrameSpinBox->value()));
    
    // View angle
    CTcpWriter tcpWriter(clientSocket);
    qreal angle = static_cast<qreal>(camAngleSpinBox->value());
    cameraWidget->setRotation(angle);
    smallCameraWidget->setRotation(angle);

    // Pic size
    tcpWriter << QString("picsize");
    tcpWriter << (quint16)camPictureSize->currentText().section("x", 0, 0).toInt();
    tcpWriter << (quint16)camPictureSize->currentText().section("x", 1, 1).toInt();
    tcpWriter.write();
    
    // Cam exposure
    tcpWriter << QString("camexposure") << camExposureCombo->currentText();
    tcpWriter.write();
    
    // Cam white balance
    tcpWriter << QString("camwb") << camWhiteBalanceCombo->currentText();
    tcpWriter.write();

    // jpeg quality
    writeTcpMsg("camjpegq", jpegQualitySlider->value());
}

void asuroqt::zoomCameraIn()
{
    if (!canSendTcp())
        return;

    CTcpWriter tcpWriter(clientSocket);
    tcpWriter << QString("camzoomin");
    tcpWriter.write();
}

void asuroqt::zoomCameraOut()
{
    if (!canSendTcp())
        return;

    CTcpWriter tcpWriter(clientSocket);
    tcpWriter << QString("camzoomout");
    tcpWriter.write();
}

void asuroqt::toggleCamera()
{
    if (!canSendTcp())
        return;

    CTcpWriter tcpWriter(clientSocket);
    tcpWriter << QString("togglecam");
    tcpWriter.write();
}

void asuroqt::takePicture()
{
    if (!canSendTcp())
        return;

    CTcpWriter tcpWriter(clientSocket);
    tcpWriter << QString("takepic");
    tcpWriter.write();
}
