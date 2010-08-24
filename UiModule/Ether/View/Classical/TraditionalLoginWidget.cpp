// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "TraditionalLoginWidget.h"
#include "MemoryLeakCheck.h"

namespace CoreUi
{
    namespace Classical
    {
        TraditionalLoginWidget::TraditionalLoginWidget(QWidget *p, const QMap<QString,QString> &stored_login_data) :
            QWidget(p),
            progress_timer_(new QTimer(this))
        {
            setupUi(this);

            // Init UI, hook up signals/slots
            DemoWorldFrame->hide();
            demoWorldLabel->hide();
            progressBar->hide();

            connect(pushButton_ReturnToEther, SIGNAL( clicked() ), p, SLOT( hide() ));
            connect(pushButton_Exit, SIGNAL( clicked() ), p, SLOT( AppExitRequest() ));

            connect(pushButton_Connect, SIGNAL( clicked() ), this, SLOT( ParseInputAndConnect() ));
            connect(lineEdit_WorldAddress, SIGNAL( returnPressed() ), this, SLOT( ParseInputAndConnect() ));
            connect(lineEdit_StartLocation, SIGNAL( returnPressed() ), this, SLOT( ParseInputAndConnect() ));
            connect(lineEdit_Username, SIGNAL( returnPressed() ), this, SLOT( ParseInputAndConnect() ));
            connect(lineEdit_Password, SIGNAL( returnPressed() ), this, SLOT( ParseInputAndConnect() ));

            // Init UI content

            /*
            QString avatar_type = stored_login_data["avatartype"];
            if (avatar_type == "opensim")
                radioButton_OpenSim->setChecked(true);
            else if (avatar_type == "realxtend")
                radioButton_realXtend->setChecked(true);
            */
            // Hide OS/reX radiobuttons.
            groupBox->hide();

            lineEdit_Username->setText(stored_login_data["account"]);
            lineEdit_Password->setText(stored_login_data["password"]);
            lineEdit_WorldAddress->setText(stored_login_data["loginurl"]);
            lineEdit_StartLocation->setText(stored_login_data["startlocation"]);

            connect(progress_timer_, SIGNAL(timeout()), SLOT(UpdateProgressBar()));
        }

        void TraditionalLoginWidget::RemoveEtherButton()
        {
            pushButton_ReturnToEther->hide();
        }

        QMap<QString, QString> TraditionalLoginWidget::GetLoginInfo() const
        {
            QMap<QString, QString> info;
            info["account"] = lineEdit_Username->text();
            info["password"] = lineEdit_Password->text();
            info["loginurl"] = lineEdit_WorldAddress->text();
            info["startlocation"] = lineEdit_StartLocation->text();

            if (lineEdit_Username->text().contains('@'))
                info["avatartype"] = "realxtend";
            else
                info["avatartype"] = "opensim";
/*
            if (radioButton_OpenSim->isChecked())
                info["avatartype"] = "opensim";
            else if (radioButton_realXtend->isChecked())
                info["avatartype"] = "realxtend";
*/

            return info;
        }

        void TraditionalLoginWidget::ParseInputAndConnect()
        {
            if (lineEdit_Username->text().isEmpty())
            {
                SetStatus(tr("Username is missing!"));
                return;
            }

            if (lineEdit_WorldAddress->text().isEmpty())
            {
                SetStatus(tr("World address is missing!"));
                return;
            }

            QMap<QString, QString> map;
            map["WorldAddress"] = lineEdit_WorldAddress->text();
            map["Username"] = lineEdit_Username->text();
            map["Password"] = lineEdit_Password->text();
            map["StartLocation"] = lineEdit_StartLocation->text();

            if (lineEdit_Username->text().contains('@'))
            {
                if (map["Username"].count("@") == 1 && map["Username"].count(" ") == 0)
                {
                    map["AvatarType"] = "RealXtend";
                    QString username_input = map["Username"];
                    int at_index = username_input.indexOf("@");
                    QString rex_username = username_input.midRef(0, at_index).toString();
                    QString rex_auth_address = username_input.midRef(at_index+1).toString();

                    map["Username"] = rex_username;
                    map["AuthenticationAddress"] = rex_auth_address;
                    emit ConnectRealXtend(map);
                }
            }
            else
            {
                if (map["Username"].count(" ") == 1)
                {
                    map["AvatarType"] = "OpenSim";
                    emit ConnectOpenSim(map);
                }
            }

/*
            if (radioButton_OpenSim->isChecked() == true)
            {
                if (map["Username"].count(" ") == 1 && !map["Username"].endsWith(" "))
                {
                    map["AvatarType"] = "OpenSim";
                    emit ConnectOpenSim(map);
                }
            }
            else if (radioButton_realXtend->isChecked() == true)
            {
                if (map["Username"].count("@") == 1 && map["Username"].count(" ") == 0)
                {
                    QString username_input = map["Username"];
                    int at_index = username_input.indexOf("@");
                    QString rex_username = username_input.midRef(0, at_index).toString();
                    QString rex_auth_address = username_input.midRef(at_index+1).toString();

                    map["AvatarType"] = "RealXtend";
                    map["Username"] = rex_username;
                    map["AuthenticationAddress"] = rex_auth_address;
                    emit ConnectRealXtend(map);
                }
            }

        StatusUpdate(true, QString("Connecting to %1 with %2").arg(map["WorldAddress"], map["Username"]));
*/

        }

        void TraditionalLoginWidget::StatusUpdate(bool connecting, const QString &message)
        {
            if (connecting)
                StartProgressBar();
            else
                StopProgressBar();

            SetStatus(message);
        }

        void TraditionalLoginWidget::SetStatus(const QString &message)
        {
            statusLabel->setText(message);
        }

        void TraditionalLoginWidget::StartProgressBar()
        {
            SetStatus(QString(tr("Connecting to %1 with %2")).arg(lineEdit_WorldAddress->text(), lineEdit_Username->text()));

            progressBar->setValue(0);
            progressBar->show();
            progress_timer_->start(30);
        }

        void TraditionalLoginWidget::StopProgressBar()
        {
            progress_timer_->stop();
            progressBar->hide();
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
