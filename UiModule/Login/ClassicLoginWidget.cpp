// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ClassicLoginWidget.h"
#include "LoginContainer.h"

namespace CoreUi
{

ClassicLoginWidget::ClassicLoginWidget(LoginContainer *controller, RexLogic::OpenSimLoginHandler *os_login_handler, Foundation::Framework *framework) :
    AbstractLogin(controller),
    framework_(framework)
{
    SetLayout();
    SetLoginHandler(os_login_handler);
    InitWidget();
    ReadConfig();
    ShowSelectedMode();
}

ClassicLoginWidget::~ClassicLoginWidget()
{
    login_handler_ = 0;
}

void ClassicLoginWidget::DoCommandParameterLogin(QMap<QString, QString> &login_information)
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

void ClassicLoginWidget::SetLoginHandler(RexLogic::OpenSimLoginHandler *os_login_handler)
{
    login_handler_ = os_login_handler;
    QObject::connect(controller_, SIGNAL( CommandParameterLogin(QMap<QString, QString>&) ), this, SLOT( DoCommandParameterLogin(QMap<QString, QString>&) ));
    QObject::connect(login_handler_, SIGNAL( LoginStarted() ), controller_, SLOT( StartLoginProgressUI() ));
}

void ClassicLoginWidget::InitWidget()
{
    QUiLoader loader;
    QFile uiFile("./data/ui/login/login_traditional.ui");

    if ( uiFile.exists() )
    {
        internal_widget_ = loader.load(&uiFile, this);
        uiFile.close();

        radiobutton_opensim_ = findChild<QRadioButton *>("radioButton_OpenSim");
        radiobutton_realxtend_ = findChild<QRadioButton *>("radioButton_realXtend");
        pushbutton_connect_ = findChild<QPushButton *>("pushButton_Connect");
        pushbutton_close_ = findChild<QPushButton *>("pushButton_Close");
        line_edit_world_address_ = findChild<QLineEdit *>("lineEdit_WorldAddress");
        line_edit_username_ = findChild<QLineEdit *>("lineEdit_Username");
        line_edit_password_ = findChild<QLineEdit *>("lineEdit_Password");

        QObject::connect(radiobutton_opensim_, SIGNAL( clicked() ), this, SLOT( ShowSelectedMode() ));
        QObject::connect(radiobutton_realxtend_, SIGNAL( clicked() ), this, SLOT( ShowSelectedMode() ));
        QObject::connect(pushbutton_connect_, SIGNAL( clicked() ), this, SLOT( ParseInputAndConnect() ));
        QObject::connect(pushbutton_close_, SIGNAL( clicked() ), controller_, SLOT( QuitApplication() ));
        QObject::connect(this, SIGNAL( ConnectOpenSim(QMap<QString, QString>) ), login_handler_, SLOT( ProcessOpenSimLogin(QMap<QString, QString>) ));
        QObject::connect(this, SIGNAL( ConnectRealXtend(QMap<QString, QString>) ), login_handler_, SLOT( ProcessRealXtendLogin(QMap<QString, QString>) ));

        QObject::connect(line_edit_world_address_, SIGNAL( returnPressed() ), this, SLOT( ParseInputAndConnect() ));
        QObject::connect(line_edit_username_, SIGNAL( returnPressed() ), this, SLOT( ParseInputAndConnect() ));
        QObject::connect(line_edit_password_, SIGNAL( returnPressed() ), this, SLOT( ParseInputAndConnect() ));

        this->layout()->addWidget(internal_widget_);
    }
}

void ClassicLoginWidget::ReadConfig()
{
    // Recover the connection settings that were used in previous login
    // from the xml configuration file.
    QString value, configKey;
    QString configGroup("Login");

    configKey = QString("username");
    opensim_username_ = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(configGroup.toStdString(), configKey.toStdString()).c_str());
    if (opensim_username_.isEmpty())
        opensim_username_ = "Firstname Lastname";
    line_edit_username_->setText(opensim_username_);

    configKey = QString("auth_name");
    realxtend_username_ = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(configGroup.toStdString(), configKey.toStdString()).c_str());
    if (realxtend_username_.isEmpty())
        realxtend_username_ = "account@";

    configKey = QString("rex_server");
    realxtend_server_ = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(configGroup.toStdString(), configKey.toStdString()).c_str());
    if (realxtend_server_.isEmpty())
        realxtend_server_ = "server:port (if no port default is 9000)";

    configKey = QString("server");
    value = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(configGroup.toStdString(), configKey.toStdString()).c_str());
    if (value.isEmpty())
        value = "server:port (if no port default is 9000)";
    line_edit_world_address_->setText(value);
    opensim_server_ = value;

    configKey = QString("auth_server");
    value = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(configGroup.toStdString(), configKey.toStdString()).c_str());
    if (value.isEmpty())
        value = "server:port (if no port default is 10001)";
    realxtend_authserver_ = value;
}

void ClassicLoginWidget::ShowSelectedMode()
{
    if (radiobutton_opensim_->isChecked() == true)
    {
        if (line_edit_username_->text() == QString("%1@%2").arg(realxtend_username_, realxtend_authserver_))
            line_edit_username_->setText(opensim_username_);
        if (line_edit_world_address_->text() == realxtend_server_)
            line_edit_world_address_->setText(opensim_server_);
    }
    else if (radiobutton_realxtend_->isChecked() == true)
    {
        if (line_edit_username_->text() == opensim_username_)
        {
            line_edit_username_->setText(QString("%1@%2").arg(realxtend_username_, realxtend_authserver_));
        }
        if (line_edit_world_address_->text() == opensim_server_)
            line_edit_world_address_->setText(realxtend_server_);
    }
    line_edit_password_->clear();
}

void ClassicLoginWidget::ParseInputAndConnect()
{
    if (controller_->isLoginInProgress())
        return;

    int fieldMissingCount = 0;
    QStringList missingFields;
    QString errorMessage;

    if (!line_edit_world_address_->text().isEmpty() && !line_edit_username_->text().isEmpty() && !line_edit_password_->text().isEmpty())
    {
        QMap<QString, QString> map;
        map["WorldAddress"] = line_edit_world_address_->text();
        map["Username"] = line_edit_username_->text();
        map["Password"] = line_edit_password_->text();
        if (radiobutton_opensim_->isChecked() == true)
        {
            if (map["Username"].count(" ") == 1 && !map["Username"].endsWith(" "))
                emit ConnectOpenSim(map);
            else
                controller_->ShowMessageToUser(QString("Your OpenSim username must be 'Firstname Lastname', you gave '%1'").arg(map["Username"]), 7);
            return;
        }
        else if (radiobutton_realxtend_->isChecked() == true)
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

    if (line_edit_username_->text().isEmpty())
        missingFields.append("Username");

    if (line_edit_world_address_->text().isEmpty())
        missingFields.append("World address");

    if (line_edit_password_->text().isEmpty())
        missingFields.append("Password");

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
