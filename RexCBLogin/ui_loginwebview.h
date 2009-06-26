/********************************************************************************
** Form generated from reading ui file 'loginwebview.ui'
**
** Created: Fri 26. Jun 10:14:46 2009
**      by: Qt User Interface Compiler version 4.5.1
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_LOGINWEBVIEW_H
#define UI_LOGINWEBVIEW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include <QtWebKit/QWebView>

QT_BEGIN_NAMESPACE

class Ui_LoginWebView
{
public:
    QVBoxLayout *verticalLayout;
    QWebView *webView;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButtonCancel;

    void setupUi(QWidget *LoginWebView)
    {
        if (LoginWebView->objectName().isEmpty())
            LoginWebView->setObjectName(QString::fromUtf8("LoginWebView"));
        LoginWebView->resize(600, 450);
        LoginWebView->setMinimumSize(QSize(600, 450));
        verticalLayout = new QVBoxLayout(LoginWebView);
        verticalLayout->setSpacing(0);
        verticalLayout->setMargin(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        webView = new QWebView(LoginWebView);
        webView->setObjectName(QString::fromUtf8("webView"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(webView->sizePolicy().hasHeightForWidth());
        webView->setSizePolicy(sizePolicy);
        webView->setUrl(QUrl("about:blank"));

        verticalLayout->addWidget(webView);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(5);
        horizontalLayout->setMargin(5);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        pushButtonCancel = new QPushButton(LoginWebView);
        pushButtonCancel->setObjectName(QString::fromUtf8("pushButtonCancel"));

        horizontalLayout->addWidget(pushButtonCancel);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(LoginWebView);

        QMetaObject::connectSlotsByName(LoginWebView);
    } // setupUi

    void retranslateUi(QWidget *LoginWebView)
    {
        LoginWebView->setWindowTitle(QApplication::translate("LoginWebView", "Form", 0, QApplication::UnicodeUTF8));
        pushButtonCancel->setText(QApplication::translate("LoginWebView", "Cancel", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(LoginWebView);
    } // retranslateUi

};

namespace Ui {
    class LoginWebView: public Ui_LoginWebView {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINWEBVIEW_H
