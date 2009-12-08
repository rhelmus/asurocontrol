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

#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QStatusBar>
#include <QTabWidget>
#include <QTcpServer>
#include <QVBoxLayout>

#include "asuroqt.h"
#include "CIRIO.h"

asuroqt::asuroqt(QWidget *parent) : QMainWindow(parent), IRReceiveCode(IR_NONE), IRBytesReceived(0)
{
	//ui.setupUi(this);
	
	IRIO = CIRIO::NewL(this);
	createUI();	
	
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
	
	QMenuBar *menubar = new QMenuBar(this);
	menubar->setGeometry(QRect(0, 0, 800, 21));
	setMenuBar(menubar);
	
	// UNDONE: Needed?
	QStatusBar *statusbar = new QStatusBar(this);
	setStatusBar(statusbar);
	
#if 0
	tcpServer = new QTcpServer(this);
	if (!tcpServer->listen(/*QHostAddress::LocalHost*/))
	{
		QMessageBox::critical(this, tr("Fortune Server"),
				tr("Unable to start the server: %1.").arg(tcpServer->errorString()));
		close();
		return;
	}
	
	connect(tcpServer, SIGNAL(newConnection()), this, SLOT(sendFortune()));
#else
	tcpSocket = new QTcpSocket(this);
	connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readFortune()));
	connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
	             this, SLOT(displayError(QAbstractSocket::SocketError)));
	
#endif
	appendLogText("Started Asuro control.\n");
	
	//appendLogText(QString("Listening for connections on port %1:%2").arg(tcpServer->serverAddress().toString()).arg(tcpServer->serverPort()));
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
	
	fortuneButton = new QPushButton("Fortune");
	connect(fortuneButton, SIGNAL(clicked()), this, SLOT(reqFortune()));
	vbox->addWidget(fortuneButton);
	
	return ret;
}

void asuroqt::setSwitch(char sw)
{
	switchLabel->setText(QString("Switch: %1").arg((int)sw, 0, 2));
}

void asuroqt::setLine(char line, ESensorSide side)
{
	if (side == SENSOR_LEFT)
		lineLabel->setText(QString("Line: %1").arg((int)line));
	else
		lineLabel->setText(lineLabel->text() + QString(", %1").arg((int)line));
}

void asuroqt::setOdo(char odo, ESensorSide side)
{
	if (side == SENSOR_LEFT)
		odoLabel->setText(QString("Odo: %1").arg((int)odo));
	else
		odoLabel->setText(odoLabel->text() + QString(", %1").arg((int)odo));
}

void asuroqt::setBattery(char bat)
{
	batteryLabel->setText(QString("Battery: %1").arg((int)bat));
}

void asuroqt::sendRC5()
{
	IRIO->sendRC5(_L("11000000000100"));
}

void asuroqt::sendFortune()
{
	QStringList fortunes = QStringList() << tr("You've been leading a dog's life. Stay off the furniture.")
			  << tr("You've got to think about tomorrow.")
			  << tr("You will be surprised by a loud noise.")
			  << tr("You will feel hungry again in another hour.")
			  << tr("You might have mail.")
			  << tr("You cannot kill time without injuring eternity.")
			  << tr("Computers are not intelligent. They only think they are.");
	 
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    out << (quint16)0;
    out << fortunes.at(qrand() % fortunes.size());
    out.device()->seek(0);
    out << (quint16)(block.size() - sizeof(quint16));

    QTcpSocket *clientConnection = tcpServer->nextPendingConnection();
    connect(clientConnection, SIGNAL(disconnected()),
            clientConnection, SLOT(deleteLater()));

    clientConnection->write(block);
    clientConnection->disconnectFromHost();
}

void asuroqt::reqFortune()
 {
	fortuneButton->setEnabled(false);
	blockSize = 0;
	tcpSocket->abort();
	tcpSocket->connectToHost("192.168.1.40", 40000);
 }

 void asuroqt::readFortune()
 {
	 appendLogText(QString("Ready read on %1:%2 - bytes: %3\n").arg(tcpSocket->peerAddress().toString())
			 .arg(tcpSocket->peerPort()).arg(tcpSocket->bytesAvailable()));
	 
     QDataStream in(tcpSocket);
     in.setVersion(QDataStream::Qt_4_0);

     if (blockSize == 0) {
         if (tcpSocket->bytesAvailable() < (int)sizeof(quint16))
             return;

         in >> blockSize;
     }

     if (tcpSocket->bytesAvailable() < blockSize)
         return;

     QString nextFortune;
     in >> nextFortune;
     appendLogText(nextFortune);
     fortuneButton->setEnabled(true);
 }

 void asuroqt::displayError(QAbstractSocket::SocketError socketError)
 {
     switch (socketError) {
     case QAbstractSocket::RemoteHostClosedError:
         break;
     case QAbstractSocket::HostNotFoundError:
         QMessageBox::information(this, tr("Fortune Client"),
                                  tr("The host was not found. Please check the "
                                     "host name and port settings."));
         break;
     case QAbstractSocket::ConnectionRefusedError:
         QMessageBox::information(this, tr("Fortune Client"),
                                  tr("The connection was refused by the peer. "
                                     "Make sure the fortune server is running, "
                                     "and check that the host name and port "
                                     "settings are correct."));
         break;
     default:
         QMessageBox::information(this, tr("Fortune Client"),
                                  tr("The following error occurred: %1.")
                                  .arg(tcpSocket->errorString()));
     }
     
     fortuneButton->setEnabled(true);
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
		default: appendLogText(QString("Unrecognized code: %1\n").arg(byte)); break;
		}
		appendLogText(QString("Got code: %1\n").arg(QChar(byte)));
	}
	else
	{
		IRBytesReceived++;
		
		appendLogText(QString("Got byte: %1\n").arg(int(byte)));
		
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
