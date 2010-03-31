// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ControlPanelManager.h"
#include "UiDefines.h"

#include "Common/AnchorLayoutManager.h"
#include "Common/UiAction.h"
#include "Common/ControlButtonAction.h"

#include "Inworld/ControlPanel/BackdropWidget.h"
#include "Inworld/ControlPanel/ControlPanelButton.h"
#include "Inworld/ControlPanel/SettingsWidget.h"
#include "Inworld/ControlPanel/BindingWidget.h"
#include "Inworld/ControlPanel/PersonalWidget.h"

#include <QAction>

namespace CoreUi
{
    ControlPanelManager::ControlPanelManager(QObject *parent, CoreUi::AnchorLayoutManager *layout_manager) :
        QObject(parent),
        layout_manager_(layout_manager),
        backdrop_widget_(new CoreUi::BackdropWidget()),
        settings_widget_(0),
        binding_widget_(0)
    {
        // Controls panel
        layout_manager_->AddCornerAnchor(backdrop_widget_, Qt::TopRightCorner, Qt::TopRightCorner);
        CreateBasicControls();

        // Settings widget
        settings_widget_ = new SettingsWidget(layout_manager_->GetScene(), this);
        ControlButtonAction *settings_action = new ControlButtonAction(GetButtonForType(UiDefines::Settings), settings_widget_, this);
        
        SetHandler(UiDefines::Settings, settings_action);
        connect(settings_action, SIGNAL(toggled(bool)), SLOT(ToggleSettingsVisibility(bool)));
        connect(settings_widget_, SIGNAL(Hidden()), SLOT(CheckSettingsButtonStyle()));

        // Binding widget as settings tab
        binding_widget_ = new BindingWidget(settings_widget_);
        settings_widget_->AddWidget(binding_widget_, "Controls");

        // Personal widget
        personal_widget_ = new PersonalWidget();
        layout_manager_->AddCornerAnchor(personal_widget_, Qt::BottomRightCorner, Qt::BottomRightCorner);
    }

    ControlPanelManager::~ControlPanelManager()
    {
        SAFE_DELETE(settings_widget_);
    }

    // Private 

    void ControlPanelManager::CreateBasicControls()
    {
        QList<UiDefines::ControlButtonType> buttons;
        buttons << UiDefines::Notifications << UiDefines::Settings << UiDefines::Quit << UiDefines::Ether;

        ControlPanelButton *button = 0;
        ControlPanelButton *previous_button = 0;
        foreach(UiDefines::ControlButtonType button_type, buttons)
        {
            // Create the button and anchor in scene
            button = new ControlPanelButton(button_type); 
            if (previous_button)
                layout_manager_->AnchorWidgetsHorizontally(previous_button, button);
            else
                layout_manager_->AddCornerAnchor(button, Qt::TopRightCorner, Qt::TopRightCorner);

            // Add to internal lists
            control_buttons_.append(button);
            if (button_type == UiDefines::Notifications || button_type == UiDefines::Settings)
                backdrop_area_buttons_map_[button_type] = button;

            connect(button, SIGNAL(ControlButtonClicked(UiDefines::ControlButtonType)), SLOT(ControlButtonClicked(UiDefines::ControlButtonType)));
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

    void ControlPanelManager::ControlButtonClicked(UiDefines::ControlButtonType type)
    {
        // Hide others if type is toggle core ui
        switch (type)
        {
            case UiDefines::Settings:
            {
                ControlButtonAction *action = dynamic_cast<ControlButtonAction*>(action_map_[UiDefines::Notifications]);
                if (action)
                    action->RequestHide();
                break;
            }
            case UiDefines::Notifications:
            {
                ControlButtonAction *action = dynamic_cast<ControlButtonAction*>(action_map_[UiDefines::Settings]);
                if (action)
                    action->RequestHide();
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
        {
            settings_widget_->AnimatedHide();
        }
    }

    void ControlPanelManager::CheckSettingsButtonStyle()
    {
        backdrop_area_buttons_map_[UiDefines::Settings]->CheckStyle(false);
    }

    // Public

    void ControlPanelManager::SetHandler(UiDefines::ControlButtonType type, UiServices::UiAction *action)
    {
        action_map_[type] = action;
    }

    ControlPanelButton *ControlPanelManager::GetButtonForType(UiDefines::ControlButtonType type)
    {
        if (backdrop_area_buttons_map_.contains(type))
            return backdrop_area_buttons_map_[type];
        else
            return 0;
    }

    qreal ControlPanelManager::GetContentHeight()
    { 
        return backdrop_widget_->GetContentHeight();
    }

    qreal ControlPanelManager::GetContentWidth() 
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