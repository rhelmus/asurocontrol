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
#include <QPlainTextEdit>
#include <QPushButton>
#include <QstatusBar>
#include <QTabWidget>
#include <QVBoxLayout>

#include "asuroqt.h"
#include "CIRIO.h"

asuroqt::asuroqt(QWidget *parent) : QMainWindow(parent)
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
	vbox->addWidget(tabW);
	
	QMenuBar *menubar = new QMenuBar(this);
	menubar->setGeometry(QRect(0, 0, 800, 21));
	setMenuBar(menubar);
	
	// UNDONE: Needed?
	QStatusBar *statusbar = new QStatusBar(this);
	setStatusBar(statusbar);
	
	appendLogText("Started Asuro control.\n");
}

QWidget *asuroqt::createGeneralTab()
{
	QWidget *ret = new QWidget;
	
	QVBoxLayout *vbox = new QVBoxLayout(ret);
	
	QPushButton *button = new QPushButton("Send RC5");
	connect(button, SIGNAL(clicked()), this, SLOT(sendRC5()));
	vbox->addWidget(button);
	
	
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

void asuroqt::sendRC5()
{
	IRIO->sendRC5(_L("11000000000100"));
}

void asuroqt::parseIRByte(char byte)
{
	appendLogText(QChar(byte));
}
