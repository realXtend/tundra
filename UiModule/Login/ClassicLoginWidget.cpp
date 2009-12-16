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
        label_auth_address_ = findChild<QLabel *>("label_AuthenticationServer");
        line_edit_auth_address_ = findChild<QLineEdit *>("lineEdit_AuthenticationAddress");
        line_edit_world_address_ = findChild<QLineEdit *>("lineEdit_WorldAddress");
        line_edit_username_ = findChild<QLineEdit *>("lineEdit_Username");
        line_edit_password_ = findChild<QLineEdit *>("lineEdit_Password");

        QObject::connect(radiobutton_opensim_, SIGNAL( clicked() ), this, SLOT( ShowSelectedMode() ));
        QObject::connect(radiobutton_realxtend_, SIGNAL( clicked() ), this, SLOT( ShowSelectedMode() ));
        QObject::connect(pushbutton_connect_, SIGNAL( clicked() ), this, SLOT( ParseInputAndConnect() ));
        QObject::connect(pushbutton_close_, SIGNAL( clicked() ), controller_, SLOT( QuitApplication() ));
        QObject::connect(this, SIGNAL( ConnectOpenSim(QMap<QString, QString>) ), login_handler_, SLOT( ProcessOpenSimLogin(QMap<QString, QString>) ));
        QObject::connect(this, SIGNAL( ConnectRealXtend(QMap<QString, QString>) ), login_handler_, SLOT( ProcessRealXtendLogin(QMap<QString, QString>) ));

        QObject::connect(line_edit_auth_address_, SIGNAL( returnPressed() ), this, SLOT( ParseInputAndConnect() ));
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
    line_edit_username_->setText(opensim_username_);

    configKey = QString("auth_name");
    realxtend_username_ = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(configGroup.toStdString(), configKey.toStdString()).c_str());

    configKey = QString("rex_server");
    realxtend_server_ = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(configGroup.toStdString(), configKey.toStdString()).c_str());

    configKey = QString("server");
    value = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(configGroup.toStdString(), configKey.toStdString()).c_str());
    line_edit_world_address_->setText(value);
    opensim_server_ = value;

    configKey = QString("auth_server");
    value = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(configGroup.toStdString(), configKey.toStdString()).c_str());
    line_edit_auth_address_->setText(value);
    realxtend_authserver_ = value;
}

void ClassicLoginWidget::ShowSelectedMode()
{
    bool hide = false;
    if (radiobutton_opensim_->isChecked() == true)
    {
        hide = false;
        if (line_edit_username_->text() == realxtend_username_)
            line_edit_username_->setText(opensim_username_);
        if (line_edit_world_address_->text() == realxtend_server_)
            line_edit_world_address_->setText(opensim_server_);
    }
    else if (radiobutton_realxtend_->isChecked() == true)
    {
        hide = true;
        if (line_edit_username_->text() == opensim_username_)
            line_edit_username_->setText(realxtend_username_);
        if (line_edit_world_address_->text() == opensim_server_)
            line_edit_world_address_->setText(realxtend_server_);
    }
    label_auth_address_->setVisible(hide);
    line_edit_auth_address_->setVisible(hide);
    line_edit_password_->clear();
}

void ClassicLoginWidget::ParseInputAndConnect()
{
    if (controller_->isLoginInProgress())
        return;

    int fieldMissingCount = 0;
    QStringList missingFields;
    QString errorMessage;

    if (!line_edit_world_address_->text().isEmpty() && !line_edit_username_->text().isEmpty())
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
        else if (radiobutton_realxtend_->isChecked() == true && 
                 !line_edit_auth_address_->text().isEmpty() )
        {
            map["AuthenticationAddress"] = line_edit_auth_address_->text();
            emit ConnectRealXtend(map);
            return;
        }
    }

    if (line_edit_username_->text().isEmpty())
        missingFields.append("Username");

    if (line_edit_world_address_->text().isEmpty())
        missingFields.append("World address");

    if (line_edit_auth_address_->text().isEmpty())
        missingFields.append("Authentication address");

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
