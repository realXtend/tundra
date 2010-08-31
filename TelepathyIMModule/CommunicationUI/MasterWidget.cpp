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
          ui_state_(ImUiDefines::UiStates::Disconnected),
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
        stacked_layout_ = new QStackedLayout(this);
        setLayout(stacked_layout_);
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

    void MasterWidget::ChangeContext(ImUiDefines::UiStates::ConnectionState new_state)
    {
        ui_state_ = (new_state == ImUiDefines::UiStates::NoStateChange ? ui_state_ : new_state);
        current_size_ = size();

        switch (ui_state_)
        {
            case ImUiDefines::UiStates::Disconnected:
            {
                if (!login_widget_)
                {
                    login_widget_ = new QWidget();
                    login_ui_->setupUi(login_widget_);
                    stacked_layout_->addWidget(login_widget_);

                    connect(login_ui_->connectPushButton, SIGNAL( clicked() ), login_helper_, SLOT( TryLogin() ));
                    connect(login_ui_->presetsComboBox, SIGNAL( currentIndexChanged(int) ), this, SLOT( PresetSelected(int) ));
                    connect(login_helper_, SIGNAL( StateChange(ImUiDefines::UiStates::ConnectionState) ), this, SLOT( ChangeContext(ImUiDefines::UiStates::ConnectionState) ));
                }
                stacked_layout_->setCurrentWidget(login_widget_);

                config_helper_->SetPreviousData(login_ui_, login_helper_->GetPreviousCredentials());
                login_ui_->passwordLineEdit->setFocus(Qt::MouseFocusReason);
                if (!login_helper_->GetErrorMessage().isEmpty())
                    login_ui_->statusLabel->setText(login_helper_->GetErrorMessage());
                break;
            }
            case ImUiDefines::UiStates::Connecting:
            {
                if (!loading_widget_)
                {
                    loading_widget_ = new QWidget();
                    loading_ui_->setupUi(loading_widget_);
                    stacked_layout_->addWidget(loading_widget_);
                
                    connect(loading_ui_->cancelPushButton, SIGNAL( clicked() ), login_helper_, SLOT( LoginCanceled() ));
                }
                stacked_layout_->setCurrentWidget(loading_widget_);                
                break;
            }
            case ImUiDefines::UiStates::Connected:
            {
                config_helper_->SaveLoginData(login_helper_->GetPreviousCredentials());
                
                if (!session_manager_widget_)
                {
                    session_manager_widget_ = new QWidget();
                    session_manager_ui_->setupUi(session_manager_widget_);
                    stacked_layout_->addWidget(session_manager_widget_);
                
                    connect(session_manager_, SIGNAL( StateChange(ImUiDefines::UiStates::ConnectionState) ), this, SLOT( ChangeContext(ImUiDefines::UiStates::ConnectionState) ));
                }
                stacked_layout_->setCurrentWidget(session_manager_widget_);

                session_manager_->Start(login_helper_->GetPreviousCredentials()["username"], login_helper_->GetConnectionInterface(), event_handler_);
                break;
            }
            case ImUiDefines::UiStates::Exit:
            {
                hide();
                return;
            }
        }
        resize(current_size_);
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