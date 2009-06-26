/********************************************************************************
** Form generated from reading ui file 'cbloginframe.ui'
**
** Created: Fri 26. Jun 10:14:59 2009
**      by: Qt User Interface Compiler version 4.5.1
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_CBLOGINFRAME_H
#define UI_CBLOGINFRAME_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFrame>
#include <QtGui/QHeaderView>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_CBLoginWidget
{
public:
    QVBoxLayout *verticalLayout;

    void setupUi(QFrame *CBLoginWidget)
    {
        if (CBLoginWidget->objectName().isEmpty())
            CBLoginWidget->setObjectName(QString::fromUtf8("CBLoginWidget"));
        CBLoginWidget->resize(800, 520);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(CBLoginWidget->sizePolicy().hasHeightForWidth());
        CBLoginWidget->setSizePolicy(sizePolicy);
        CBLoginWidget->setMinimumSize(QSize(800, 520));
        CBLoginWidget->setFrameShape(QFrame::StyledPanel);
        CBLoginWidget->setFrameShadow(QFrame::Raised);
        verticalLayout = new QVBoxLayout(CBLoginWidget);
        verticalLayout->setMargin(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));

        retranslateUi(CBLoginWidget);

        QMetaObject::connectSlotsByName(CBLoginWidget);
    } // setupUi

    void retranslateUi(QFrame *CBLoginWidget)
    {
        CBLoginWidget->setWindowTitle(QApplication::translate("CBLoginWidget", "Frame", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(CBLoginWidget);
    } // retranslateUi

};

namespace Ui {
    class CBLoginWidget: public Ui_CBLoginWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CBLOGINFRAME_H
