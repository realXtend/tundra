// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_MasterWidget_h
#define incl_Communication_MasterWidget_h

//#include "Foundation.h"
#include "ImUiDefines.h"

//#include "EventHandler.h"

#include "TelepathyIMModuleFwd.h"

#include <QWidget>
#include <QSize>

namespace UiHelpers
{
    class ConfigHelper;
    class LoginHelper;
}

namespace UiManagers
{
    class SessionManager;
}

namespace Ui
{
    class LoginWidget;
    class LoadingWidget;
    class SessionManagerWidget;
}

namespace Foundation
{
    class Framework;
}

namespace CommunicationUI
{
    class MasterWidget : public QWidget
    {
    
    Q_OBJECT
    Q_PROPERTY(ImUiDefines::UiStates::ConnectionState ui_state_ READ uiState WRITE setUiState)

    public:
        explicit MasterWidget(Foundation::Framework *framework);
        virtual ~MasterWidget();

    public slots:
        //! Getters and setters
        void setUiState(ImUiDefines::UiStates::ConnectionState state) { ui_state_ = state; }
        ImUiDefines::UiStates::ConnectionState uiState() { return ui_state_; }
        EventHandler *GetEventHandler() { return event_handler_; }

        void ChangeContext(ImUiDefines::UiStates::ConnectionState new_state = ImUiDefines::UiStates::NoStateChange);
        void PresetSelected(int index);

    protected:
        void hideEvent(QHideEvent *hide_event);

    private:
        void InitializeSelf();
        QSize CleanSelf();
        bool connecting_;

        Ui::LoginWidget *login_ui_;
        Ui::LoadingWidget *loading_ui_;
        Ui::SessionManagerWidget *session_manager_ui_;

        QWidget *login_widget_;
        QWidget *loading_widget_;
        QWidget *session_manager_widget_;

        ImUiDefines::UiStates::ConnectionState ui_state_;

        UiHelpers::LoginHelper *login_helper_;
        UiHelpers::ConfigHelper *config_helper_;
        UiManagers::SessionManager *session_manager_;

        EventHandler *event_handler_;

        QSize current_size_;
        QStringList to_be_removed_;
    };
}

#endif // incl_Communication_MasterWidget_h