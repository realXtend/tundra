// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "MasterWidget.h"
#include "LoginHelper.h"

namespace CommunicationUI
{
    MasterWidget::MasterWidget()
        : QWidget(),
        ui_state_(UiDefines::UiStates::Disconnected),
          login_helper_(new UiHelpers::LoginHelper(login_ui_))
    {
        ChangeContext();
        show();
    }

    MasterWidget::~MasterWidget()
    {
        SAFE_DELETE(login_helper_);
    }

    void MasterWidget::ChangeContext(UiDefines::UiStates::ConnectionState new_state)
    {
        ui_state_ = (new_state == UiDefines::UiStates::NoStateChange ? ui_state_ : new_state);

        switch (ui_state_)
        {
            case UiDefines::UiStates::Disconnected:
            {
                login_ui_.setupUi(this);
                QObject::connect(login_ui_.connectPushButton, SIGNAL( clicked() ), login_helper_, SLOT( TryLogin() ));
                QObject::connect(login_helper_, SIGNAL( StateChange(UiDefines::UiStates::ConnectionState) ), this, SLOT( ChangeContext(UiDefines::UiStates::ConnectionState) ));
                break;
            }
            case UiDefines::UiStates::Connecting:
            {
                loading_ui_.setupUi(this);
                QObject::connect(loading_ui_.cancelPushButton, SIGNAL( clicked() ), login_helper_, SLOT( LoginCancelled() ));
                break;
            }
            case UiDefines::UiStates::Connected:
            {
                break;
            }
        }
    }
}