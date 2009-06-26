/********************************************************************************
** Form generated from reading ui file 'cbloginwidget.ui'
**
** Created: Tue 23. Jun 12:19:56 2009
**      by: Qt User Interface Compiler version 4.5.1
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_CBLOGINWIDGET_H
#define UI_CBLOGINWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CBLoginWidget_old
{
public:
    QWidget *centralWidget;
    QVBoxLayout *VerticalLayout;

    void setupUi(QMainWindow *CBLoginWidget_old)
    {
        if (CBLoginWidget_old->objectName().isEmpty())
            CBLoginWidget_old->setObjectName(QString::fromUtf8("CBLoginWidget_old"));
        CBLoginWidget_old->resize(800, 520);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(CBLoginWidget_old->sizePolicy().hasHeightForWidth());
        CBLoginWidget_old->setSizePolicy(sizePolicy);
        CBLoginWidget_old->setMinimumSize(QSize(800, 520));
        centralWidget = new QWidget(CBLoginWidget_old);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        sizePolicy.setHeightForWidth(centralWidget->sizePolicy().hasHeightForWidth());
        centralWidget->setSizePolicy(sizePolicy);
        centralWidget->setMinimumSize(QSize(700, 350));
        VerticalLayout = new QVBoxLayout(centralWidget);
        VerticalLayout->setSpacing(6);
        VerticalLayout->setMargin(0);
        VerticalLayout->setObjectName(QString::fromUtf8("VerticalLayout"));
        CBLoginWidget_old->setCentralWidget(centralWidget);

        retranslateUi(CBLoginWidget_old);

        QMetaObject::connectSlotsByName(CBLoginWidget_old);
    } // setupUi

    void retranslateUi(QMainWindow *CBLoginWidget_old)
    {
        CBLoginWidget_old->setWindowTitle(QApplication::translate("CBLoginWidget_old", "RexNG testbed", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(CBLoginWidget_old);
    } // retranslateUi

};

namespace Ui {
    class CBLoginWidget_old: public Ui_CBLoginWidget_old {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CBLOGINWIDGET_H
