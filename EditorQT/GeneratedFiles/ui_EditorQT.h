/********************************************************************************
** Form generated from reading UI file 'EditorQT.ui'
**
** Created: Sun 13. Jan 17:17:28 2013
**      by: Qt User Interface Compiler version 5.0.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EDITORQT_H
#define UI_EDITORQT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_EditorQTClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *EditorQTClass)
    {
        if (EditorQTClass->objectName().isEmpty())
            EditorQTClass->setObjectName(QStringLiteral("EditorQTClass"));
        EditorQTClass->resize(600, 400);
        menuBar = new QMenuBar(EditorQTClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        EditorQTClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(EditorQTClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        EditorQTClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(EditorQTClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        EditorQTClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(EditorQTClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        EditorQTClass->setStatusBar(statusBar);

        retranslateUi(EditorQTClass);

        QMetaObject::connectSlotsByName(EditorQTClass);
    } // setupUi

    void retranslateUi(QMainWindow *EditorQTClass)
    {
        EditorQTClass->setWindowTitle(QApplication::translate("EditorQTClass", "EditorQT", 0));
    } // retranslateUi

};

namespace Ui {
    class EditorQTClass: public Ui_EditorQTClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EDITORQT_H
