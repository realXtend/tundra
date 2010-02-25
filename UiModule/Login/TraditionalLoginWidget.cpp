// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "TraditionalLoginWidget.h"
#include "LoginContainer.h"
#include "EventHandlers/LoginHandler.h"

namespace CoreUi
{
    TraditionalLoginWidget::TraditionalLoginWidget(LoginContainer *controller, RexLogic::OpenSimLoginHandler *os_login_handler, Foundation::Framework *framework) 
        : AbstractLogin(controller),
          framework_(framework)
    {
        SetLoginHandler(os_login_handler);
        InitWidget();
        ReadConfig();
        ShowSelectedMode();
    }

    TraditionalLoginWidget::~TraditionalLoginWidget()
    {
        login_handler_ = 0;
    }

    void TraditionalLoginWidget::DoCommandParameterLogin(QMap<QString, QString> &login_information)
    {
        if (login_information["AuthType"] == "OpenSim")
        {
            if (login_information["Username"].count(" ") == 1 && !login_information["Username"].endsWith(" "))
                emit ConnectOpenSim(login_information);
            else
                controller_->ShowMessageToUser(QString("Your OpenSim username must be 'Firstname Lastname', you gave '%1'").arg(login_information["Username"]), 7);
        }
        else if (login_information["AuthType"] == "RealXtend")
        {
            emit ConnectRealXtend(login_information);
        }
    }

    void TraditionalLoginWidget::SetLoginHandler(RexLogic::OpenSimLoginHandler *os_login_handler)
    {
        login_handler_ = os_login_handler;
        connect(controller_, SIGNAL( CommandParameterLogin(QMap<QString, QString>&) ), this, SLOT( DoCommandParameterLogin(QMap<QString, QString>&) ));
        connect(login_handler_, SIGNAL( LoginStarted() ), controller_, SLOT( StartLoginProgressUI() ));
    }

    void TraditionalLoginWidget::InitWidget()
    {
        ui_.setupUi(this);
        // Hide the demo container and label, show to user when the actual widget gets there
        ui_.DemoWorldFrame->hide();
        ui_.demoWorldLabel->hide();

        connect(ui_.radioButton_OpenSim, SIGNAL( clicked() ), this, SLOT( ShowSelectedMode() ));
        connect(ui_.radioButton_realXtend, SIGNAL( clicked() ), this, SLOT( ShowSelectedMode() ));
        connect(ui_.pushButton_Connect, SIGNAL( clicked() ), this, SLOT( ParseInputAndConnect() ));
        connect(ui_.pushButton_Close, SIGNAL( clicked() ), controller_, SLOT( QuitApplication() ));

        connect(ui_.lineEdit_WorldAddress, SIGNAL( returnPressed() ), this, SLOT( ParseInputAndConnect() ));
        connect(ui_.lineEdit_Username, SIGNAL( returnPressed() ), this, SLOT( ParseInputAndConnect() ));
        connect(ui_.lineEdit_Password, SIGNAL( returnPressed() ), this, SLOT( ParseInputAndConnect() ));

        connect(this, SIGNAL( ConnectOpenSim(QMap<QString, QString>) ), login_handler_, SLOT( ProcessOpenSimLogin(QMap<QString, QString>) ));
        connect(this, SIGNAL( ConnectRealXtend(QMap<QString, QString>) ), login_handler_, SLOT( ProcessRealXtendLogin(QMap<QString, QString>) ));
    }

    void TraditionalLoginWidget::ReadConfig()
    {
        // Recover the connection settings that were used in previous login
        // from the xml configuration file.
        QString value, configKey;
        QString configGroup("Login");

        configKey = QString("username");
        opensim_username_ = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(configGroup.toStdString(), configKey.toStdString()).c_str());
        if (opensim_username_.isEmpty())
            opensim_username_ = "Firstname Lastname";
        ui_.lineEdit_Username->setText(opensim_username_);

        configKey = QString("auth_name");
        realxtend_username_ = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(configGroup.toStdString(), configKey.toStdString()).c_str());
        if (realxtend_username_.isEmpty())
            realxtend_username_ = "account";

        configKey = QString("rex_server");
        realxtend_server_ = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(configGroup.toStdString(), configKey.toStdString()).c_str());
        if (realxtend_server_.isEmpty())
            realxtend_server_ = "server:port (if no port default is 9000)";

        configKey = QString("server");
        value = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(configGroup.toStdString(), configKey.toStdString()).c_str());
        if (value.isEmpty())
            value = "server:port (if no port default is 9000)";
        ui_.lineEdit_WorldAddress->setText(value);
        opensim_server_ = value;

        configKey = QString("auth_server");
        value = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(configGroup.toStdString(), configKey.toStdString()).c_str());
        if (value.isEmpty())
            value = "server:port (if no port default is 10001)";
        realxtend_authserver_ = value;
    }

    void TraditionalLoginWidget::ShowSelectedMode()
    {
        if (ui_.radioButton_OpenSim->isChecked() == true)
        {
            if (ui_.lineEdit_Username->text() == QString("%1@%2").arg(realxtend_username_, realxtend_authserver_))
                ui_.lineEdit_Username->setText(opensim_username_);
            if (ui_.lineEdit_WorldAddress->text() == realxtend_server_)
                ui_.lineEdit_WorldAddress->setText(opensim_server_);
        }
        else if (ui_.radioButton_realXtend->isChecked() == true)
        {
            if (ui_.lineEdit_Username->text() == opensim_username_)
                ui_.lineEdit_Username->setText(QString("%1@%2").arg(realxtend_username_, realxtend_authserver_));
            if (ui_.lineEdit_WorldAddress->text() == opensim_server_)
                ui_.lineEdit_WorldAddress->setText(realxtend_server_);
        }
        ui_.lineEdit_Password->clear();
    }

    void TraditionalLoginWidget::ParseInputAndConnect()
    {
        if (controller_->isLoginInProgress())
            return;

        int fieldMissingCount = 0;
        QStringList missingFields;
        QString errorMessage;

        if (!ui_.lineEdit_WorldAddress->text().isEmpty() && !ui_.lineEdit_Username->text().isEmpty())
        {
            QMap<QString, QString> map;
            map["WorldAddress"] = ui_.lineEdit_WorldAddress->text();
            map["Username"] = ui_.lineEdit_Username->text();
            map["Password"] = ui_.lineEdit_Password->text();
            if (ui_.radioButton_OpenSim->isChecked() == true)
            {
                if (map["Username"].count(" ") == 1 && !map["Username"].endsWith(" "))
                    emit ConnectOpenSim(map);
                else
                    controller_->ShowMessageToUser(QString("Your OpenSim username must be 'Firstname Lastname', you gave '%1'").arg(map["Username"]), 7);
                return;
            }
            else if (ui_.radioButton_realXtend->isChecked() == true)
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
                else
                    controller_->ShowMessageToUser(QString("Your realXtend username must be 'user@server:port', you gave '%1'").arg(map["Username"]), 7);
                return;
            }
        }

        if (ui_.lineEdit_Username->text().isEmpty())
            missingFields.append("Username");

        if (ui_.lineEdit_WorldAddress->text().isEmpty())
            missingFields.append("World address");

        if (missingFields.count() >= 3)
        {
            errorMessage = QString("Please input the required fields ");
            int beforeAnd = missingFields.count() - 2;
            for (int i=0; i <beforeAnd; ++i)
            {
                errorMessage.append(missingFields.value(i));
                errorMessage.append(", ");
            }
            errorMessage.append(missingFields.value(beforeAnd));
            errorMessage.append(" and ");
            errorMessage.append(missingFields.value(++beforeAnd));
        }
        else if (missingFields.count() == 2)
        {
            errorMessage = QString("Please input the required fields ");
            errorMessage.append(missingFields.value(0));
            errorMessage.append(" and ");
            errorMessage.append(missingFields.value(1));
        }
        else if (missingFields.count() == 1)
        {
            errorMessage = QString("Please input the required field ");
            errorMessage.append(missingFields.value(0));
        }

        controller_->ShowMessageToUser(errorMessage, 7);
    }
}
