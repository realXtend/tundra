// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MasterWidget.h"

#include "LoginHelper.h"
#include "ConfigHelper.h"
#include "SessionManager.h"
#include "EventHandler.h"

#include "ui_LoginWidget.h"
#include "ui_LoadingWidget.h"
#include "ui_SessionManagerWidget.h"

#include <QFile>
#include <QDebug>

#include "MemoryLeakCheck.h"

namespace CommunicationUI
{
    MasterWidget::MasterWidget(Foundation::Framework *framework)
        : QWidget(),
          ui_state_(UiDefines::UiStates::Disconnected),
          login_helper_(new UiHelpers::LoginHelper),
          config_helper_(new UiHelpers::ConfigHelper(framework)),
          session_manager_(new UiManagers::SessionManager(this, framework)),
          login_ui_(new Ui::LoginWidget),
          loading_ui_(new Ui::LoadingWidget),
          session_manager_ui_(new Ui::SessionManagerWidget()),
          event_handler_(new EventHandler(framework)),
          to_be_removed_(),
          login_widget_(0),
          loading_widget_(0),
          session_manager_widget_(0)

    {
        setWindowIcon(QIcon(":/images/iconUsers.png"));
        setLayout(new QVBoxLayout(this));
        layout()->setMargin(0);

        ChangeContext();
        InitializeSelf();
    }

    MasterWidget::~MasterWidget()
    {
        SAFE_DELETE(login_helper_);
        SAFE_DELETE(config_helper_);
        SAFE_DELETE(session_manager_);
        SAFE_DELETE(event_handler_);
        SAFE_DELETE(login_ui_);
        SAFE_DELETE(loading_ui_);
        SAFE_DELETE(session_manager_ui_);

        SAFE_DELETE(login_widget_);
        SAFE_DELETE(loading_widget_);
        SAFE_DELETE(session_manager_widget_);
    }

    void MasterWidget::InitializeSelf()
    {
        // Read config and setup ui pointers for helpers
        config_helper_->ReadLoginData(login_ui_);
        login_helper_->SetupUi(login_ui_);
        session_manager_->SetupUi(session_manager_ui_);

        // Read main style sheet from file
        QFile style_sheet_file("../TelepathyIMModule/CommunicationUI/ui/StyleSheet.css");
        if (style_sheet_file.open(QIODevice::ReadOnly | QIODevice::Text))
            setStyleSheet(QString(style_sheet_file.readAll()));
        else
        {
            QFile style_sheet_file("./data/ui/StyleSheet.css");
            if (style_sheet_file.open(QIODevice::ReadOnly | QIODevice::Text))
                setStyleSheet(QString(style_sheet_file.readAll()));
        }

        // Set initial size
        resize(350, 120);
    }

    void MasterWidget::ChangeContext(UiDefines::UiStates::ConnectionState new_state)
    {
        ui_state_ = (new_state == UiDefines::UiStates::NoStateChange ? ui_state_ : new_state);
        current_size_ = CleanSelf();

        switch (ui_state_)
        {
            case UiDefines::UiStates::Disconnected:
            {
                SAFE_DELETE(login_widget_);
                login_widget_ = new QWidget();
                login_ui_->setupUi(login_widget_);
                layout()->addWidget(login_widget_);
                to_be_removed_ << "login_ui_";

                config_helper_->SetPreviousData(login_ui_, login_helper_->GetPreviousCredentials());
                login_ui_->passwordLineEdit->setFocus(Qt::MouseFocusReason);
                if (!login_helper_->GetErrorMessage().isEmpty())
                    login_ui_->statusLabel->setText(login_helper_->GetErrorMessage());
                
                connect(login_ui_->connectPushButton, SIGNAL( clicked() ), login_helper_, SLOT( TryLogin() ));
                connect(login_ui_->presetsComboBox, SIGNAL( currentIndexChanged(int) ), this, SLOT( PresetSelected(int) ));
                connect(login_helper_, SIGNAL( StateChange(UiDefines::UiStates::ConnectionState) ), this, SLOT( ChangeContext(UiDefines::UiStates::ConnectionState) ));
                break;
            }
            case UiDefines::UiStates::Connecting:
            {
                SAFE_DELETE(loading_widget_);
                loading_widget_ = new QWidget();
                loading_ui_->setupUi(loading_widget_);
                layout()->addWidget(loading_widget_);
                to_be_removed_ << "loading_ui_";

                connect(loading_ui_->cancelPushButton, SIGNAL( clicked() ), login_helper_, SLOT( LoginCanceled() ));
                break;
            }
            case UiDefines::UiStates::Connected:
            {
                config_helper_->SaveLoginData(login_helper_->GetPreviousCredentials());

                SAFE_DELETE(session_manager_widget_);
                session_manager_widget_ = new QWidget();
                session_manager_ui_->setupUi(session_manager_widget_);
                layout()->addWidget(session_manager_widget_);
                to_be_removed_ << "session_manager_ui_";

                connect(session_manager_, SIGNAL( StateChange(UiDefines::UiStates::ConnectionState) ), this, SLOT( ChangeContext(UiDefines::UiStates::ConnectionState) ));
                session_manager_->Start(login_helper_->GetPreviousCredentials()["username"], login_helper_->GetConnectionInterface(), event_handler_);
                break;
            }
            case UiDefines::UiStates::Exit:
            {
                hide();
                return;
            }
        }
        resize(current_size_);
    }

    QSize MasterWidget::CleanSelf()
    {
        foreach (QString ui_class, to_be_removed_)
        {
            if (ui_class == "login_ui_")
            {               
                login_widget_->disconnect();
                login_widget_->hide();
                layout()->removeItem(layout()->itemAt(layout()->indexOf(login_widget_)));
                login_widget_->setParent(0);
            }
            else if (ui_class == "loading_ui_")
            {
                loading_widget_->disconnect();
                loading_widget_->hide();
                layout()->removeItem(layout()->itemAt(layout()->indexOf(loading_widget_)));
                loading_widget_->setParent(0);
            }
            else if (ui_class == "session_manager_ui_")
            {
                session_manager_widget_->disconnect();
                session_manager_widget_->hide();
                layout()->removeItem(layout()->itemAt(layout()->indexOf(session_manager_widget_)));
                session_manager_widget_->setParent(0);
            }
        }
        to_be_removed_.clear();
        return size();
    }

    void MasterWidget::hideEvent(QHideEvent *hide_event)
    {
        QWidget::hideEvent(hide_event);
        if (session_manager_)
            session_manager_->HideFriendListWidget();
    }

    void MasterWidget::PresetSelected(int index)
    {
        QMap<QString,QString> data_map;
        switch (index)
        {
            case 0:
                config_helper_->ReadLoginData(login_ui_);
                break;
            case 1:
                data_map["username"] = "username@gmail.com";
                data_map["server"] = "talk.google.com";
                data_map["password"] = "";
                config_helper_->SetPreviousData(login_ui_, data_map);
                break;
            case 2:
                data_map["username"] = "username@chat.facebook.com";
                data_map["server"] = "chat.facebook.com";
                data_map["password"] = "";
                config_helper_->SetPreviousData(login_ui_, data_map);
                break;
        }
    }

}