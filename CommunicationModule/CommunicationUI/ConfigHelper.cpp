// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ConfigHelper.h"

#include "ui_LoginWidget.h"

namespace UiHelpers
{
    ConfigHelper::ConfigHelper(Foundation::Framework *framework)
        : QObject(),
          framework_(framework)
    {

    }

    ConfigHelper::~ConfigHelper()
    {
        framework_ = 0;
    }

    void ConfigHelper::ReadLoginData(Ui::LoginWidget *login_ui)
    {
        QString value, configKey;
        QString server, username;
		QString configGroup("InstantMessagingLogin");

		configKey = QString("username");
		username = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(configGroup.toStdString(), configKey.toStdString()).c_str());
        configKey = QString("server");
		server = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(configGroup.toStdString(), configKey.toStdString()).c_str());
		
        if (!username.isNull() && !username.isEmpty())
            login_ui->usernameLineEdit->setText(username);
        if (!server.isNull() && !server.isEmpty())
            login_ui->serverLineEdit->setText(server);
    }

    void ConfigHelper::SetPreviousData(Ui::LoginWidget *login_ui, QMap<QString,QString> data_map)
    {
        if (!data_map["username"].isEmpty() && !data_map["server"].isEmpty())
        {
            login_ui->usernameLineEdit->setText(data_map["username"]);
            login_ui->serverLineEdit->setText(data_map["server"]);
        }
    }

    void ConfigHelper::SaveLoginData(QMap<QString,QString> data_map)
    {
        framework_->GetConfigManager()->SetSetting<std::string>(std::string("InstantMessagingLogin"), std::string("server"), data_map["server"].toStdString());
        framework_->GetConfigManager()->SetSetting<std::string>(std::string("InstantMessagingLogin"), std::string("username"), data_map["username"].toStdString());
        framework_->GetConfigManager()->Export();
    }
}