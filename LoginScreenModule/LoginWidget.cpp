/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   
 *  @brief  
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "LoginWidget.h"
#include "MemoryLeakCheck.h"

LoginWidget::LoginWidget(QWidget *parent, const QMap<QString,QString> &login_data) :
    QWidget(parent),
    progress_timer_(new QTimer(this))
{
    InitWidget(login_data);
    connect(progress_timer_, SIGNAL(timeout()), SLOT(UpdateProgressBar()));
}

void LoginWidget::InitWidget(const QMap<QString,QString> &stored_login_data)
{
/*
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(this);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
*/
    setupUi(this);

    progressBar->hide();

    connect(pushButton_Connect, SIGNAL(clicked()), SLOT(ParseInputAndConnect()));
//    connect(pushButton_Exit, SIGNAL(clicked()), parent(), SLOT(AppExitRequest()));

    connect(lineEdit_WorldAddress, SIGNAL(returnPressed() ), SLOT(ParseInputAndConnect()));
    connect(lineEdit_StartLocation, SIGNAL(returnPressed() ), SLOT(ParseInputAndConnect()));
    connect(lineEdit_Username, SIGNAL(returnPressed()), SLOT(ParseInputAndConnect()));
    connect(lineEdit_Password, SIGNAL(returnPressed()), SLOT(ParseInputAndConnect()));

    lineEdit_Username->setText(stored_login_data["account"]);
    lineEdit_Password->setText(stored_login_data["password"]);
    lineEdit_WorldAddress->setText(stored_login_data["loginurl"]);
    lineEdit_StartLocation->setText(stored_login_data["startlocation"]);
}

QMap<QString, QString> LoginWidget::GetLoginInfo() const
{
    QMap<QString, QString> ui_data_map;
    ui_data_map["account"] = lineEdit_Username->text();
    ui_data_map["password"] = lineEdit_Password->text();
    ui_data_map["loginurl"] = lineEdit_WorldAddress->text();
    ui_data_map["startlocation"] = lineEdit_StartLocation->text();

    if (lineEdit_Username->text().contains('@'))
        ui_data_map["avatartype"] = "realxtend";
    else
        ui_data_map["avatartype"] = "opensim";

    return ui_data_map;
}

void LoginWidget::ParseInputAndConnect()
{
    if (lineEdit_WorldAddress->text().isEmpty() || lineEdit_Username->text().isEmpty())
        return;

    QMap<QString, QString> map;
    map["WorldAddress"] = lineEdit_WorldAddress->text();
    map["Username"] = lineEdit_Username->text();
    map["Password"] = lineEdit_Password->text();
    map["StartLocation"] = lineEdit_StartLocation->text();

    if (1/*os*/)
    {
        if (map["Username"].count(" ") == 1 && !map["Username"].endsWith(" "))
            emit ConnectOpenSim(map);
    }
    else if (0/*rex*/)
    {
        if (map["Username"].count("@") == 1 && map["Username"].count(" ") == 0)
        {
            QString username_input = map["Username"];
            int at_index = username_input.indexOf("@");
            QString rex_username = username_input.midRef(0, at_index).toString();
            QString rex_auth_address = username_input.midRef(at_index+1).toString();

            map["Username"] = rex_username;
            map["AuthenticationAddress"] = rex_auth_address;
            emit ConnectRealXtend(map);
        }
    }

    StatusUpdate(true, QString("Connecting to %1 with %2").arg(map["WorldAddress"], map["Username"]));
}

void LoginWidget::StatusUpdate(bool connecting, const QString &message)
{
    if (connecting)
    {
        progressBar->setValue(0);
        progressBar->show();
        progress_timer_->start(30);
    }
    else
    {
        progress_timer_->stop();
        progressBar->hide();
    }
    statusLabel->setText(message);
}

void LoginWidget::UpdateProgressBar()
{
    int value = progressBar->value();
    if (value == 0)
        progress_direction_ = 1;
    else if (value == 100)
        progress_direction_ = -1;
    progressBar->setValue(value + progress_direction_);
}

