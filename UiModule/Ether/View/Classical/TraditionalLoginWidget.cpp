// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "TraditionalLoginWidget.h"
#include "MemoryLeakCheck.h"

namespace CoreUi
{
    namespace Classical
    {
        TraditionalLoginWidget::TraditionalLoginWidget(QWidget *parent, QMap<QString,QString> stored_login_data) :
            QWidget(parent),
            progress_timer_(new QTimer(this))
        {
            InitWidget(stored_login_data);
            connect(progress_timer_, SIGNAL(timeout()), SLOT(UpdateProgressBar()));
        }

        void TraditionalLoginWidget::InitWidget(QMap<QString,QString> stored_login_data)
        {
            setupUi(this);

            // Init UI, hook up signals/slots
            DemoWorldFrame->hide();
            demoWorldLabel->hide();
            progressBar->hide();

            connect(pushButton_Connect, SIGNAL( clicked() ), this, SLOT( ParseInputAndConnect() ));
            connect(pushButton_ReturnToEther, SIGNAL( clicked() ), parent(), SLOT( hide() ));
            connect(pushButton_Exit, SIGNAL( clicked() ), parent(), SLOT( AppExitRequest() ));

            connect(lineEdit_WorldAddress, SIGNAL( returnPressed() ), this, SLOT( ParseInputAndConnect() ));
            connect(lineEdit_Username, SIGNAL( returnPressed() ), this, SLOT( ParseInputAndConnect() ));
            connect(lineEdit_Password, SIGNAL( returnPressed() ), this, SLOT( ParseInputAndConnect() ));

            // Init UI content
            QString avatar_type = stored_login_data["avatartype"];
            if (avatar_type == "opensim")
                radioButton_OpenSim->setChecked(true);
            else if (avatar_type == "realxtend")
                radioButton_realXtend->setChecked(true);

            lineEdit_Username->setText(stored_login_data["account"]);
            lineEdit_Password->setText(stored_login_data["password"]);
            lineEdit_WorldAddress->setText(stored_login_data["loginurl"]);
        }

        void TraditionalLoginWidget::RemoveEtherButton()
        {
            pushButton_ReturnToEther->hide();
        }

        QMap<QString, QString> TraditionalLoginWidget::GetLoginInfo()
        {
            QMap<QString, QString> ui_data_map;
            ui_data_map["account"] = lineEdit_Username->text();
            ui_data_map["password"] = lineEdit_Password->text();
            ui_data_map["loginurl"] = lineEdit_WorldAddress->text();

            if (radioButton_OpenSim->isChecked())
                ui_data_map["avatartype"] = "opensim";
            else if (radioButton_realXtend->isChecked())
                ui_data_map["avatartype"] = "realxtend";

            return ui_data_map;
        }

        void TraditionalLoginWidget::ParseInputAndConnect()
        {
            if (lineEdit_WorldAddress->text().isEmpty() || lineEdit_Username->text().isEmpty())
                return;

            QMap<QString, QString> map;
            map["WorldAddress"] = lineEdit_WorldAddress->text();
            map["Username"] = lineEdit_Username->text();
            map["Password"] = lineEdit_Password->text();

            if (radioButton_OpenSim->isChecked() == true)
            {
                if (map["Username"].count(" ") == 1 && !map["Username"].endsWith(" "))
                    emit ConnectOpenSim(map);
            }
            else if (radioButton_realXtend->isChecked() == true)
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

        void TraditionalLoginWidget::StatusUpdate(bool connecting, QString message)
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

        void TraditionalLoginWidget::UpdateProgressBar()
        {
            int value = progressBar->value();
            if (value == 0)
                progress_direction_ = 1;
            else if (value == 100)
                progress_direction_ = -1;
            progressBar->setValue(value + progress_direction_);
        }
    }
}
