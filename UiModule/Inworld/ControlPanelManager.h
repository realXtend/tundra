// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_ControlPanelManager_h
#define incl_UiModule_ControlPanelManager_h

#include "UiModuleApi.h"
#include "UiDefines.h"

#include <QObject>
#include <QAction>
#include <QMap>

namespace Foundation { class KeyBindings; }
namespace UiServices { class UiAction; }

namespace CoreUi
{
    class AnchorLayoutManager;
    class BackdropWidget;
    class ControlPanelButton;

    class SettingsWidget;
    class BindingWidget;
    class PersonalWidget;
    class LanguageWidget;
    class TeleportWidget;
    class CacheSettingsWidget;
    class ChangeThemeWidget;

    class UI_MODULE_API ControlPanelManager : public QObject
    {
    
    Q_OBJECT

    public:
        ControlPanelManager(QObject *parent, CoreUi::AnchorLayoutManager *layout_manager);
        virtual ~ControlPanelManager();

    public slots:
        void SetHandler(UiDefines::ControlButtonType type, UiServices::UiAction *action);
        ControlPanelButton *GetButtonForType(UiDefines::ControlButtonType type);

        qreal GetContentHeight();
        qreal GetContentWidth();

        void SetServiceGetter(QObject *service_getter);
        SettingsWidget *GetSettingsWidget() { return settings_widget_; }
        PersonalWidget *GetPersonalWidget() { return personal_widget_; }
        TeleportWidget *GetTeleportWidget() { return teleport_widget_; }
        LanguageWidget *GetLanguageWidget() { return language_widget_; }
        BindingWidget *GeBindingsWidget() { return binding_widget_; }
        CacheSettingsWidget *GetCacheSettingsWidget() { return cache_settings_widget_; }

    private slots:
        void CreateBasicControls();
        void UpdateBackdrop();

        void ControlButtonClicked(UiDefines::ControlButtonType type);

        // Internal handling of settings widget
        void ToggleSettingsVisibility(bool visible);
        void ToggleTeleportVisibility(bool visible);
        void CheckSettingsButtonStyle();
        void CheckTeleportButtonStyle();

    private:
        AnchorLayoutManager *layout_manager_;
        BackdropWidget *backdrop_widget_;

        QList<ControlPanelButton *> control_buttons_;
        QMap<UiDefines::ControlButtonType, ControlPanelButton *> backdrop_area_buttons_map_;

        QMap<UiDefines::ControlButtonType, UiServices::UiAction *> action_map_;

        // Contolled core widgets
        SettingsWidget *settings_widget_;
        BindingWidget *binding_widget_;
        PersonalWidget *personal_widget_;
        LanguageWidget* language_widget_;
        TeleportWidget* teleport_widget_;
        ChangeThemeWidget* changetheme_widget_;
        CacheSettingsWidget *cache_settings_widget_;
    };
}

#endif