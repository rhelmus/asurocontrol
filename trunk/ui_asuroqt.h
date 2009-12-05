/********************************************************************************
** Form generated from reading UI file 'asuroqt.ui'
**
** Created: Sat 5. Dec 00:19:48 2009
**      by: Qt User Interface Compiler version 4.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ASUROQT_H
#define UI_ASUROQT_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_asuroqtClass
{
public:
    QWidget *centralwidget;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *asuroqtClass)
    {
        if (asuroqtClass->objectName().isEmpty())
            asuroqtClass->setObjectName(QString::fromUtf8("asuroqtClass"));
        asuroqtClass->resize(800, 600);
        centralwidget = new QWidget(asuroqtClass);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        asuroqtClass->setCentralWidget(centralwidget);
        menubar = new QMenuBar(asuroqtClass);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 21));
        asuroqtClass->setMenuBar(menubar);
        statusbar = new QStatusBar(asuroqtClass);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        asuroqtClass->setStatusBar(statusbar);

        retranslateUi(asuroqtClass);

        QMetaObject::connectSlotsByName(asuroqtClass);
    } // setupUi

    void retranslateUi(QMainWindow *asuroqtClass)
    {
        asuroqtClass->setWindowTitle(QApplication::translate("asuroqtClass", "MainWindow", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class asuroqtClass: public Ui_asuroqtClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ASUROQT_H
