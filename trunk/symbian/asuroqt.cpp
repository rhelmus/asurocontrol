/****************************************************************************
**
** Trolltech hereby grants a license to use the Qt/Eclipse Integration
** plug-in (the software contained herein), in binary form, solely for the
** purpose of creating code to be used with Trolltech's Qt software.
**
** Qt Designer is licensed under the terms of the GNU General Public
** License versions 2.0 and 3.0 ("GPL License"). Trolltech offers users the
** right to use certain no GPL licensed software under the terms of its GPL
** Exception version 1.2 (http://trolltech.com/products/qt/gplexception).
**
** THIS SOFTWARE IS PROVIDED BY TROLLTECH AND ITS CONTRIBUTORS (IF ANY) "AS
** IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
** TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
** PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
** OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
** EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
** PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
** PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
** LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
** NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** Since we now have the GPL exception I think that the "special exception
** is no longer needed. The license text proposed above (other than the
** special exception portion of it) is the BSD license and we have added
** the BSD license as a permissible license under the exception.
**
****************************************************************************/

#include <QCoreApplication>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QStatusBar>
#include <QTabWidget>
#include <QVBoxLayout>

#include "asuroqt.h"
#include "CIRIO.h"

asuroqt::asuroqt(QWidget *parent) : QMainWindow(parent), IRReceiveCode(IR_NONE), IRBytesReceived(0)
{
	IRIO = CIRIO::NewL(this);
	createUI();	
	
	connect(this, SIGNAL(gotIRByte(char)), this, SLOT(parseIRByte(char)));
	
	IRIO->Start();
}

asuroqt::~asuroqt()
{
	delete IRIO;
	IRIO = 0;
}

void asuroqt::appendLogText(const QString &text)
{
    QTextCursor cur = logWidget->textCursor();
    cur.movePosition(QTextCursor::End);
    logWidget->setTextCursor(cur);
    logWidget->insertPlainText(text);
}

void asuroqt::createUI()
{
	QWidget *cw = new QWidget;
	setCentralWidget(cw);

	QVBoxLayout *vbox = new QVBoxLayout(cw);
	
	QTabWidget *tabW = new QTabWidget;
	tabW->addTab(createGeneralTab(), "General");
	tabW->addTab(createLogTab(), "Log");
	tabW->addTab(createDebugTab(), "Debug");
	vbox->addWidget(tabW);
	
	QMenuBar *menuBar = new QMenuBar(this);
	menuBar->setGeometry(QRect(0, 0, 800, 21));
	setMenuBar(menuBar);
	
	connectAction = menuBar->addAction("Connect", this, SLOT(connectToServer()));
	menuBar->addAction("Disconnect", this, SLOT(disonnectFromServer()));
	
	// UNDONE: Needed?
	/*QStatusBar *statusbar = new QStatusBar(this);
	setStatusBar(statusbar);*/
	
	tcpSocket = new QTcpSocket(this);
	
	connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
	        this, SLOT(socketError(QAbstractSocket::SocketError)));
	
	appendLogText("Started Asuro control.\n");
}

QWidget *asuroqt::createGeneralTab()
{
	QScrollArea *ret = new QScrollArea;
	ret->setFrameStyle(QFrame::Box | QFrame::Plain);
	ret->setWidgetResizable(true);

	QVBoxLayout *vbox = new QVBoxLayout(ret);
	
	vbox->addWidget(switchLabel = new QLabel);
	vbox->addWidget(lineLabel = new QLabel);
	vbox->addWidget(odoLabel = new QLabel);
	vbox->addWidget(batteryLabel = new QLabel);
		
	return ret;
}

QWidget *asuroqt::createLogTab()
{
	QWidget *ret = new QWidget;
	
	QVBoxLayout *vbox = new QVBoxLayout(ret);
	
	vbox->addWidget(logWidget = new QPlainTextEdit);
	logWidget->setReadOnly(true);
	logWidget->setCenterOnScroll(true);
	
	QPushButton *clearB = new QPushButton("Clear");
	connect(clearB, SIGNAL(clicked()), logWidget, SLOT(clear()));
	vbox->addWidget(clearB);
	
	return ret;	
}

QWidget *asuroqt::createDebugTab()
{
	QWidget *ret = new QWidget;
	
	QVBoxLayout *vbox = new QVBoxLayout(ret);
	
	QPushButton *button = new QPushButton("Send RC5");
	connect(button, SIGNAL(clicked()), this, SLOT(sendRC5()));
	vbox->addWidget(button);
	
	button = new QPushButton("Send TCP");
	connect(button, SIGNAL(clicked()), this, SLOT(sendDummyData()));
	vbox->addWidget(button);
		
	return ret;
}

void asuroqt::setSwitch(char sw)
{
	switchLabel->setText(QString("Switch: %1").arg((int)sw, 0, 2));
	sendSensorData("switch", sw);
}

void asuroqt::setLine(char line, ESensorSide side)
{
	if (side == SENSOR_LEFT)
	{
		lineLabel->setText(QString("Line: %1").arg((int)line));
		sendSensorData("linel", line);
	}
	else
	{
		lineLabel->setText(lineLabel->text() + QString(", %1").arg((int)line));
		sendSensorData("liner", line);
	}
}

void asuroqt::setOdo(char odo, ESensorSide side)
{
	if (side == SENSOR_LEFT)
	{
		odoLabel->setText(QString("Odo: %1").arg((int)odo));
		sendSensorData("odol", odo);
	}
	else
	{
		odoLabel->setText(odoLabel->text() + QString(", %1").arg((int)odo));
		sendSensorData("odor", odo);
	}
}

void asuroqt::setBattery(char bat)
{
	batteryLabel->setText(QString("Battery: %1").arg((int)bat));
	sendSensorData("battery", bat);
}

void asuroqt::sendSensorData(const QString &sensor, quint16 data)
{
	if (tcpSocket->state() != QTcpSocket::ConnectedState)
	{
		appendLogText(QString("Cannot send data: %1\n").arg(tcpSocket->state()));
		return;
	}
	
	QByteArray block;
	QDataStream out(&block, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_5);

	out << (quint16)0; // Size
	out << sensor;
	out << data;
	out.device()->seek(0);
	out << (quint16)(block.size() - sizeof(quint16));

	tcpSocket->write(block);
}

void asuroqt::sendRC5()
{
	IRIO->sendRC5(_L("11000000000100"));
}

void asuroqt::connectToServer()
{
	connectAction->setEnabled(false);
	tcpSocket->abort();
	tcpSocket->connectToHost("192.168.1.40", 40000);
}

void asuroqt::disconnectFromServer()
{
	tcpSocket->abort();
	connectAction->setEnabled(true);
}

void asuroqt::socketError(QAbstractSocket::SocketError socketError)
{
	appendLogText(QString("Socket error!: %1\n").arg(tcpSocket->errorString()));
	connectAction->setEnabled(true);
}

void asuroqt::sendDummyData()
{
	sendSensorData("switch", 1);
}

void asuroqt::parseIRByte(char byte)
{
	if (IRReceiveCode == IR_NONE)
	{
		switch (byte)
		{
		case 'S': IRReceiveCode = IR_SWITCH; break;
		case 'L': IRReceiveCode = IR_LINE; break;
		case 'O': IRReceiveCode = IR_ODO; break;
		case 'B': IRReceiveCode = IR_BATTERY; break;
		default: appendLogText(QString("Unrecognized code: %1\n").arg((int)byte)); break;
		}
//		appendLogText(QString("Got code: %1\n").arg(QChar(byte)));
	}
	else
	{
		IRBytesReceived++;
		
//		appendLogText(QString("Got byte: %1\n").arg(int(byte)));
		
		switch (IRReceiveCode)
		{
		case IR_SWITCH:
			setSwitch(byte);
			resetIRReceive();
			break;
		case IR_LINE:
			if (IRBytesReceived == 1)
				setLine(byte, SENSOR_LEFT);
			else
			{
				setLine(byte, SENSOR_RIGHT);
				resetIRReceive();
			}
			break;
		case IR_ODO:
			if (IRBytesReceived == 1)
				setOdo(byte, SENSOR_LEFT);
			else
			{
				setOdo(byte, SENSOR_RIGHT);
				resetIRReceive();
			}
			break;
		case IR_BATTERY:
			setBattery(byte);
			resetIRReceive();
			break;
		}
	}
}
