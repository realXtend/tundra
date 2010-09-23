// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ControlPanelManager.h"

#include "Common/AnchorLayoutManager.h"
#include "Common/UiAction.h"
#include "Common/ControlButtonAction.h"

#include "Inworld/ControlPanel/BackdropWidget.h"
#include "Inworld/ControlPanel/ControlPanelButton.h"
#include "Inworld/ControlPanel/SettingsWidget.h"
#include "Inworld/ControlPanel/BindingWidget.h"
#include "Inworld/ControlPanel/PersonalWidget.h"
#include "Inworld/ControlPanel/LanguageWidget.h"
#include "Inworld/ControlPanel/TeleportWidget.h"
#include "Inworld/ControlPanel/CacheSettingsWidget.h"
#include "Inworld/ControlPanel/ChangeThemeWidget.h"

#include <QAction>

#include "MemoryLeakCheck.h"

namespace CoreUi
{
    ControlPanelManager::ControlPanelManager(QObject *parent, AnchorLayoutManager *layout_manager) :
        QObject(parent),
        layout_manager_(layout_manager),
        backdrop_widget_(new CoreUi::BackdropWidget()),
        settings_widget_(0),
        binding_widget_(0),
        language_widget_(0),
        teleport_widget_(0),
        changetheme_widget_(0)
    {
        // Controls panel
        layout_manager_->AddCornerAnchor(backdrop_widget_, Qt::TopRightCorner, Qt::TopRightCorner);
        CreateBasicControls();

        // Settings widget
        settings_widget_ = new SettingsWidget(layout_manager_->GetScene(), this);
        ControlButtonAction *settings_action = new ControlButtonAction(GetButtonForType(UiServices::Settings), settings_widget_, this);
        
        SetHandler(UiServices::Settings, settings_action);
        connect(settings_action, SIGNAL(toggled(bool)), SLOT(ToggleSettingsVisibility(bool)));
        connect(settings_widget_, SIGNAL(Hidden()), SLOT(CheckSettingsButtonStyle()));

        // Binding widget as settings tab
        //binding_widget_ = new BindingWidget(settings_widget_);
        //settings_widget_->AddWidget(binding_widget_, "Controls");

        // Adding cache tab
        cache_settings_widget_ = new CacheSettingsWidget(settings_widget_);
        settings_widget_->AddWidget(cache_settings_widget_, "Cache");

        // Adding a language tab.
        language_widget_ = new LanguageWidget(settings_widget_);
        settings_widget_->AddWidget(language_widget_, "Language");

        // Personal widget
        personal_widget_ = new PersonalWidget();
        layout_manager_->AddCornerAnchor(personal_widget_, Qt::BottomRightCorner, Qt::BottomRightCorner);
        connect(personal_widget_, SIGNAL(ControlButtonClicked(UiServices::ControlButtonType)), SLOT(ControlButtonClicked(UiServices::ControlButtonType)));

        // Teleport widget
        teleport_widget_ = new TeleportWidget(layout_manager_->GetScene(), this);
        ControlButtonAction *teleport_action = new ControlButtonAction(GetButtonForType(UiServices::Teleport), teleport_widget_, this);
        
        SetHandler(UiServices::Teleport, teleport_action);
        connect(teleport_action, SIGNAL(toggled(bool)), SLOT(ToggleTeleportVisibility(bool)));
        connect(teleport_widget_, SIGNAL(Hidden()), SLOT(CheckTeleportButtonStyle()));

        // Adding change theme tab
        changetheme_widget_ = new ChangeThemeWidget(settings_widget_);
        settings_widget_->AddWidget(changetheme_widget_, "Change theme");
    }

    ControlPanelManager::~ControlPanelManager()
    {
        SAFE_DELETE(settings_widget_);
    }

    // Private 

    void ControlPanelManager::CreateBasicControls()
    {
        QList<UiServices::ControlButtonType> buttons;
        buttons << UiServices::Notifications << UiServices::Teleport << UiServices::Settings << UiServices::Quit << UiServices::Build << UiServices::Ether;

        ControlPanelButton *button = 0;
        ControlPanelButton *previous_button = 0;
        foreach(UiServices::ControlButtonType button_type, buttons)
        {
            // Create the button and anchor in scene
            button = new ControlPanelButton(button_type); 
            if (previous_button)
                layout_manager_->AnchorWidgetsHorizontally(previous_button, button);
            else
                layout_manager_->AddCornerAnchor(button, Qt::TopRightCorner, Qt::TopRightCorner);

            // Add to internal lists
            control_buttons_.append(button);
            if (button_type == UiServices::Notifications || button_type == UiServices::Settings || button_type == UiServices::Teleport)
                backdrop_area_buttons_map_[button_type] = button;

            connect(button, SIGNAL(ControlButtonClicked(UiServices::ControlButtonType)), SLOT(ControlButtonClicked(UiServices::ControlButtonType)));
            previous_button = button;
        }
        UpdateBackdrop();
    }

    void ControlPanelManager::UpdateBackdrop()
    {
        qreal width = 0;
        foreach (ControlPanelButton *button, backdrop_area_buttons_map_.values())
            width += button->GetContentWidth();
        backdrop_widget_->SetContentWidth(width);
    }

    void ControlPanelManager::ControlButtonClicked(UiServices::ControlButtonType type)
    {
        // Hide others if type is toggle core ui
        switch (type)
        {
            case UiServices::Settings:
            {
                ControlButtonAction *action_notifications = dynamic_cast<ControlButtonAction*>(action_map_[UiServices::Notifications]);
                ControlButtonAction *action_teleport = dynamic_cast<ControlButtonAction*>(action_map_[UiServices::Teleport]);
                if (action_notifications)
                    action_notifications->RequestHide();
                if (action_teleport)
                    action_teleport->RequestHide();
                break;
            }
            case UiServices::Teleport:
            {
                ControlButtonAction *action_notifications = dynamic_cast<ControlButtonAction*>(action_map_[UiServices::Notifications]);
                ControlButtonAction *action_settings = dynamic_cast<ControlButtonAction*>(action_map_[UiServices::Settings]);
                if (action_notifications)
                    action_notifications->RequestHide();
                if (action_settings)
                    action_settings->RequestHide();
                break;
            }
            case UiServices::Notifications:
            {
                ControlButtonAction *action_settings = dynamic_cast<ControlButtonAction*>(action_map_[UiServices::Settings]);
                ControlButtonAction *action_teleport = dynamic_cast<ControlButtonAction*>(action_map_[UiServices::Teleport]);
                if (action_settings)
                    action_settings->RequestHide();
                if (action_teleport)
                    action_teleport->RequestHide();
                break;
            }
            default:
                break;
        }

        if (action_map_.contains(type))
            action_map_[type]->trigger();
    }

    void ControlPanelManager::ToggleSettingsVisibility(bool visible)
    {
        if (visible)
            settings_widget_->show();
        else
            settings_widget_->AnimatedHide();
    }

    void ControlPanelManager::ToggleTeleportVisibility(bool visible)
    {
        if (visible)
            teleport_widget_->show();
        else
            teleport_widget_->AnimatedHide();
    }

    void ControlPanelManager::CheckSettingsButtonStyle()
    {
        backdrop_area_buttons_map_[UiServices::Settings]->CheckStyle(false);
    }

    void ControlPanelManager::CheckTeleportButtonStyle()
    {
        backdrop_area_buttons_map_[UiServices::Teleport]->CheckStyle(false);
    }

    // Public

    void ControlPanelManager::SetHandler(UiServices::ControlButtonType type, UiServices::UiAction *action)
    {
        action_map_[type] = action;
    }

    ControlPanelButton *ControlPanelManager::GetButtonForType(UiServices::ControlButtonType type) const
    {
        if (backdrop_area_buttons_map_.contains(type))
            return backdrop_area_buttons_map_[type];
        else
            return 0;
    }

    qreal ControlPanelManager::GetContentHeight() const
    { 
        return backdrop_widget_->GetContentHeight();
    }

    qreal ControlPanelManager::GetContentWidth() const
    { 
        return backdrop_widget_->GetContentWidth();
    }

    void ControlPanelManager::SetServiceGetter(QObject *service_getter)
    {
        if (binding_widget_)
        {
            connect(service_getter, SIGNAL(KeyBindingsChanged(Foundation::KeyBindings*)),
                    binding_widget_, SLOT(UpdateContent(Foundation::KeyBindings*)));
            connect(binding_widget_, SIGNAL(KeyBindingsUpdated(Foundation::KeyBindings*)),
                    service_getter, SLOT(SetKeyBindings(Foundation::KeyBindings*)));
            connect(binding_widget_, SIGNAL(RestoreDefaultBindings()),
                    service_getter, SLOT(RestoreKeyBindings()));
        }
    }
}
