/********************************************************************************
** Form generated from reading ui file 'loginpanel.ui'
**
** Created: Fri 26. Jun 10:14:50 2009
**      by: Qt User Interface Compiler version 4.5.1
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_LOGINPANEL_H
#define UI_LOGINPANEL_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LoginPanel
{
public:
    QGridLayout *gridLayoutTopContainer;
    QGridLayout *gridLayout;
    QLabel *labelWorld;
    QLabel *labelOpenID;
    QLineEdit *lineEditWorld;
    QLineEdit *lineEditOpenID;
    QHBoxLayout *horizontalLayout;
    QLabel *labelPresetWorld;
    QComboBox *comboBoxWorlds;
    QHBoxLayout *horizontalLayout_2;
    QLabel *labelPresetOpenID;
    QComboBox *comboBoxOpenIDs;
    QHBoxLayout *horizontalLayout_3;
    QCheckBox *checkBoxSave;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButtonLogin;

    void setupUi(QWidget *LoginPanel)
    {
        if (LoginPanel->objectName().isEmpty())
            LoginPanel->setObjectName(QString::fromUtf8("LoginPanel"));
        LoginPanel->resize(500, 170);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(LoginPanel->sizePolicy().hasHeightForWidth());
        LoginPanel->setSizePolicy(sizePolicy);
        LoginPanel->setMinimumSize(QSize(500, 170));
        gridLayoutTopContainer = new QGridLayout(LoginPanel);
        gridLayoutTopContainer->setObjectName(QString::fromUtf8("gridLayoutTopContainer"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setSizeConstraint(QLayout::SetDefaultConstraint);
        gridLayout->setVerticalSpacing(10);
        labelWorld = new QLabel(LoginPanel);
        labelWorld->setObjectName(QString::fromUtf8("labelWorld"));
        labelWorld->setMinimumSize(QSize(100, 0));
        QFont font;
        font.setFamily(QString::fromUtf8("Arial"));
        font.setPointSize(10);
        labelWorld->setFont(font);

        gridLayout->addWidget(labelWorld, 0, 0, 1, 1);

        labelOpenID = new QLabel(LoginPanel);
        labelOpenID->setObjectName(QString::fromUtf8("labelOpenID"));
        labelOpenID->setMinimumSize(QSize(10, 0));
        labelOpenID->setFont(font);

        gridLayout->addWidget(labelOpenID, 2, 0, 1, 1);

        lineEditWorld = new QLineEdit(LoginPanel);
        lineEditWorld->setObjectName(QString::fromUtf8("lineEditWorld"));
        lineEditWorld->setMinimumSize(QSize(250, 0));

        gridLayout->addWidget(lineEditWorld, 0, 1, 1, 1);

        lineEditOpenID = new QLineEdit(LoginPanel);
        lineEditOpenID->setObjectName(QString::fromUtf8("lineEditOpenID"));
        lineEditOpenID->setMinimumSize(QSize(250, 0));

        gridLayout->addWidget(lineEditOpenID, 2, 1, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(10);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setSizeConstraint(QLayout::SetDefaultConstraint);
        labelPresetWorld = new QLabel(LoginPanel);
        labelPresetWorld->setObjectName(QString::fromUtf8("labelPresetWorld"));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(labelPresetWorld->sizePolicy().hasHeightForWidth());
        labelPresetWorld->setSizePolicy(sizePolicy1);

        horizontalLayout->addWidget(labelPresetWorld);

        comboBoxWorlds = new QComboBox(LoginPanel);
        comboBoxWorlds->setObjectName(QString::fromUtf8("comboBoxWorlds"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(comboBoxWorlds->sizePolicy().hasHeightForWidth());
        comboBoxWorlds->setSizePolicy(sizePolicy2);

        horizontalLayout->addWidget(comboBoxWorlds);


        gridLayout->addLayout(horizontalLayout, 1, 1, 1, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(10);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        labelPresetOpenID = new QLabel(LoginPanel);
        labelPresetOpenID->setObjectName(QString::fromUtf8("labelPresetOpenID"));
        sizePolicy1.setHeightForWidth(labelPresetOpenID->sizePolicy().hasHeightForWidth());
        labelPresetOpenID->setSizePolicy(sizePolicy1);

        horizontalLayout_2->addWidget(labelPresetOpenID);

        comboBoxOpenIDs = new QComboBox(LoginPanel);
        comboBoxOpenIDs->setObjectName(QString::fromUtf8("comboBoxOpenIDs"));
        QSizePolicy sizePolicy3(QSizePolicy::Preferred, QSizePolicy::Maximum);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(comboBoxOpenIDs->sizePolicy().hasHeightForWidth());
        comboBoxOpenIDs->setSizePolicy(sizePolicy3);

        horizontalLayout_2->addWidget(comboBoxOpenIDs);


        gridLayout->addLayout(horizontalLayout_2, 3, 1, 1, 1);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        checkBoxSave = new QCheckBox(LoginPanel);
        checkBoxSave->setObjectName(QString::fromUtf8("checkBoxSave"));
        checkBoxSave->setChecked(false);

        horizontalLayout_3->addWidget(checkBoxSave);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer);

        pushButtonLogin = new QPushButton(LoginPanel);
        pushButtonLogin->setObjectName(QString::fromUtf8("pushButtonLogin"));

        horizontalLayout_3->addWidget(pushButtonLogin);


        gridLayout->addLayout(horizontalLayout_3, 4, 1, 1, 1);


        gridLayoutTopContainer->addLayout(gridLayout, 0, 0, 1, 1);


        retranslateUi(LoginPanel);

        QMetaObject::connectSlotsByName(LoginPanel);
    } // setupUi

    void retranslateUi(QWidget *LoginPanel)
    {
        LoginPanel->setWindowTitle(QApplication::translate("LoginPanel", "Form", 0, QApplication::UnicodeUTF8));
        labelWorld->setText(QApplication::translate("LoginPanel", "World URL", 0, QApplication::UnicodeUTF8));
        labelOpenID->setText(QApplication::translate("LoginPanel", "OpenID URL", 0, QApplication::UnicodeUTF8));
        lineEditWorld->setText(QString());
        lineEditOpenID->setText(QString());
        labelPresetWorld->setText(QApplication::translate("LoginPanel", "Load preset", 0, QApplication::UnicodeUTF8));
        labelPresetOpenID->setText(QApplication::translate("LoginPanel", "Load preset", 0, QApplication::UnicodeUTF8));
        checkBoxSave->setText(QApplication::translate("LoginPanel", "Save entries to presets", 0, QApplication::UnicodeUTF8));
        pushButtonLogin->setText(QApplication::translate("LoginPanel", "Login", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(LoginPanel);
    } // retranslateUi

};

namespace Ui {
    class LoginPanel: public Ui_LoginPanel {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LOGINPANEL_H
