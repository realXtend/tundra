// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "InworldLoginDialog.h"
#include "ConfigurationManager.h"
#include "CoreDefines.h"
#include "Framework.h"
#include <QUiLoader>
#include <QFile>

#include "MemoryLeakCheck.h"

namespace CoreUi
{

    InworldLoginDialog::InworldLoginDialog(Foundation::Framework *framework)
        : QObject(),
          inworld_login_widget_(0),
          framework_(framework)
    {

    }

    InworldLoginDialog::~InworldLoginDialog()
    {
        SAFE_DELETE(inworld_login_widget_);
    }

    void InworldLoginDialog::ShowModalDialog(const QString &server_address)
    {
        current_server_address_ = server_address;

        QUiLoader loader;
        QFile uiFile("./data/ui/login/inworld_credentials_dialog.ui");
        if (uiFile.exists())
        {
            inworld_login_widget_ = (QDialog *)loader.load(&uiFile);
            inworld_login_widget_->setModal(true);
            inworld_login_widget_->setWindowTitle("Login Credentials");
            inworld_login_widget_->show();

            QPushButton *connect_button = inworld_login_widget_->findChild<QPushButton *>("button_connect");
            QPushButton *cancel_button = inworld_login_widget_->findChild<QPushButton *>("button_cancel");
            radiobutton_opensim_ = inworld_login_widget_->findChild<QRadioButton *>("radioButton_OpenSim");
            radiobutton_realxtend_ = inworld_login_widget_->findChild<QRadioButton *>("radioButton_realXtend");

            line_edit_username_ = inworld_login_widget_->findChild<QLineEdit *>("lineEdit_Username");
            line_edit_password_ = inworld_login_widget_->findChild<QLineEdit *>("lineEdit_Password");

            status_message_label_ = inworld_login_widget_->findChild<QLabel *>("status_message_label");
            status_message_label_->setText(QString("Credentials to %1").arg(server_address));

            QObject::connect(connect_button, SIGNAL( clicked() ), this, SLOT( ParseAndTryToConnect() ));
            QObject::connect(cancel_button, SIGNAL( clicked() ), inworld_login_widget_, SLOT( close() ));
            QObject::connect(radiobutton_opensim_, SIGNAL( clicked() ), this, SLOT( ToggleLoginShowMode() ));
            QObject::connect(radiobutton_realxtend_, SIGNAL( clicked() ), this, SLOT( ToggleLoginShowMode() ));

            SetConfigsToUi();
            ToggleLoginShowMode();
        }
    }

    void InworldLoginDialog::SetConfigsToUi()
    {
        QString value, configKey;
        QString configGroup("Login");

        configKey = QString("username");
        opensim_username_ = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(configGroup.toStdString(), configKey.toStdString()).c_str());
        line_edit_username_->setText(opensim_username_);

        configKey = QString("auth_name");
        realxtend_username_ = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(configGroup.toStdString(), configKey.toStdString()).c_str());

        configKey = QString("auth_server");
        value = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(configGroup.toStdString(), configKey.toStdString()).c_str());
        realxtend_authserver_ = value;
    }

    void InworldLoginDialog::ToggleLoginShowMode()
    {
        if (radiobutton_opensim_->isChecked() == true)
        {
            if (line_edit_username_->text() == QString("%1@%2").arg(realxtend_username_, realxtend_authserver_))
                line_edit_username_->setText(opensim_username_);

        }
        else if (radiobutton_realxtend_->isChecked() == true)
        {
            if (line_edit_username_->text() == opensim_username_)
                line_edit_username_->setText(QString("%1@%2").arg(realxtend_username_, realxtend_authserver_));
        }
        line_edit_password_->clear();
    }

    void InworldLoginDialog::ParseAndTryToConnect()
    {
        if ( !line_edit_username_->text().isEmpty() &&
             !line_edit_password_->text().isEmpty() )
        {
            QMap<QString, QString> map;

            map["WorldAddress"] = current_server_address_;
            map["Username"] = line_edit_username_->text();
            map["Password"] = line_edit_password_->text();

            if (radiobutton_opensim_->isChecked() == true)
            {
                map["AuthType"] = "OpenSim";
                if (map["Username"].count(" ") == 1 && !map["Username"].endsWith(" "))
                {
                    inworld_login_widget_->close();
                    emit TryLogin(map);
                }
                else
                    status_message_label_->setText(QString("Your OpenSim username must be 'Firstname Lastname', you gave '%1'").arg(map["Username"]));
            }
            else if (radiobutton_realxtend_->isChecked() == true)
            {
                map["AuthType"] = "RealXtend";
                if (map["Username"].count("@") == 1 && map["Username"].count(" ") == 0)
                {
                    QString username_input = map["Username"];
                    int at_index = username_input.indexOf("@");
                    QString rex_username = username_input.midRef(0, at_index).toString();
                    QString rex_auth_address = username_input.midRef(at_index+1).toString();

                    map["Username"] = rex_username;
                    map["AuthenticationAddress"] = rex_auth_address;
                    inworld_login_widget_->close();
                    emit TryLogin(map);
                }
                else
                    status_message_label_->setText(QString("Your realXtend username must be 'user@server:port', you gave '%1'").arg(map["Username"]));
            }
        }
        else
            status_message_label_->setText("Username or Password fields are required");
    }
}