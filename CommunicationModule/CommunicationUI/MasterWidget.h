// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_MasterWidget_h
#define incl_Communication_MasterWidget_h

#include "Foundation.h"
#include "UiDefines.h"

#include "ui_LoginWidget.h"
#include "ui_LoadingWidget.h"

namespace UiHelpers
{
    class LoginHelper;
}

namespace CommunicationUI
{
    class MasterWidget : public QWidget
    {
    
    Q_OBJECT
    Q_PROPERTY(UiDefines::UiStates::ConnectionState ui_state_ READ uiState WRITE setUiState)

    public:
        MasterWidget();
        virtual ~MasterWidget();

    public slots:
        //! Getters and setters
        void setUiState(UiDefines::UiStates::ConnectionState state) { ui_state_ = state; }
        UiDefines::UiStates::ConnectionState uiState() { return ui_state_; }

        void ChangeContext(UiDefines::UiStates::ConnectionState new_state = UiDefines::UiStates::NoStateChange);

    private:
        Ui::LoginWidget login_ui_;
        Ui::LoadingWidget loading_ui_;

        UiDefines::UiStates::ConnectionState ui_state_;
        UiHelpers::LoginHelper *login_helper_;
        
    };
}

#endif // incl_Communication_MasterWidget_h