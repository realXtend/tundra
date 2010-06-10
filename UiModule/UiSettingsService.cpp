// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "UiSettingsService.h"

#include "Inworld/ControlPanelManager.h"
#include "Inworld/ControlPanel/SettingsWidget.h"
#include "Inworld/ControlPanel/BindingWidget.h"
#include "Inworld/ControlPanel/PersonalWidget.h"
#include "Inworld/ControlPanel/LanguageWidget.h"
#include "Inworld/ControlPanel/TeleportWidget.h"
#include "Inworld/ControlPanel/CacheSettingsWidget.h"

namespace UiServices
{
    UiSettingsService::UiSettingsService(CoreUi::ControlPanelManager *control_panel_manager) :
        control_panel_manager_(control_panel_manager)
    {

    }

    UiSettingsService::~UiSettingsService()
    {

    }

    QObject *UiSettingsService::GetMainSettingsWidget() const
    {
        return dynamic_cast<QObject*>(control_panel_manager_->GetSettingsWidget());
    }

    QObject *UiSettingsService::GetPersonalWidget() const
    {
        return dynamic_cast<QObject*>(control_panel_manager_->GetPersonalWidget());
    }

    QObject *UiSettingsService::GetTeleportWidget() const
    {
        return dynamic_cast<QObject*>(control_panel_manager_->GetTeleportWidget());
    }

    QObject *UiSettingsService::GetLanguageWidget() const
    {
        return dynamic_cast<QObject*>(control_panel_manager_->GetLanguageWidget());
    }

    QObject *UiSettingsService::GeBindingsWidget() const
    {
        return dynamic_cast<QObject*>(control_panel_manager_->GeBindingsWidget());
    }

    QObject *UiSettingsService::GetCacheSettingsWidget() const
    {
        return dynamic_cast<QObject*>(control_panel_manager_->GetCacheSettingsWidget());
    }
}