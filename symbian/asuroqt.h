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

#ifndef ASUROQT_H
#define ASUROQT_H

#include <QtGui/QMainWindow>
#include <QTcpSocket>

class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;

class CIRIO; 

class asuroqt : public QMainWindow
{
    Q_OBJECT

    QPlainTextEdit *logWidget;
    QTcpSocket *tcpSocket;
    QAction *connectAction;
    CIRIO *IRIO;
    QLineEdit *debugIRInput, *debugIRPulse;
    
    enum EIRReceiveCode { IR_NONE, IR_SWITCH, IR_LINE, IR_ODO, IR_BATTERY };
    enum ESensorSide { SENSOR_LEFT, SENSOR_RIGHT };
    
    EIRReceiveCode IRReceiveCode;
    int IRBytesReceived;
    QLabel *switchLabel, *lineLabel, *odoLabel, *batteryLabel;
    
    void createUI(void);
    QWidget *createGeneralTab(void);
    QWidget *createLogTab(void);
    QWidget *createDebugTab(void);
    
    void resetIRReceive(void) { IRReceiveCode = IR_NONE; IRBytesReceived = 0; }
    void setSwitch(char sw);
    void setLine(char line, ESensorSide side);
    void setOdo(char odo, ESensorSide side);
    void setBattery(char bat);
    void sendSensorData(const QString &sensor, quint16 data);
    
private slots:
	void sendDummyIR(void);
	void connectToServer(void);
	void disconnectFromServer(void);
	void socketError(QAbstractSocket::SocketError socketError);
	void sendDummyData(void);
	void parseIRByte(char byte);
	
public:
	asuroqt(QWidget *parent = 0);
    ~asuroqt();
    
    void appendLogText(const QString &text);
    void IRByte(char byte) { emit(gotIRByte(byte)); }
    
signals:
    void gotIRByte(char byte);
};

#endif // ASUROQT_H
