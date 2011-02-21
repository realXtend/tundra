// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_ControlPanelManager_h
#define incl_UiModule_ControlPanelManager_h

#include "UiModuleApi.h"
#include "UiTypes.h"

#include <QObject>
#include <QAction>
#include <QMap>

namespace UiServices { class UiAction; }

namespace CoreUi
{
    class AnchorLayoutManager;
    class BackdropWidget;
    class ControlPanelButton;

    class SettingsWidget;
    class LanguageWidget;
    class CacheSettingsWidget;
    class ChangeThemeWidget;

    class UI_MODULE_API ControlPanelManager : public QObject
    {
    
    Q_OBJECT

    public:
        ControlPanelManager(QObject *parent, AnchorLayoutManager *layout_manager);
        virtual ~ControlPanelManager();

    public slots:
        void SetHandler(UiServices::ControlButtonType type, UiServices::UiAction *action);
        ControlPanelButton *GetButtonForType(UiServices::ControlButtonType type) const;

        qreal GetContentHeight() const;
        qreal GetContentWidth() const;

        SettingsWidget *GetSettingsWidget()  const { return settings_widget_; }
		void ToggleSettingsVisibility(bool visible);

    private slots:
        void CreateBasicControls();
        void UpdateBackdrop();

        void ControlButtonClicked(UiServices::ControlButtonType type);

        // Internal handling of settings widget
        void CheckSettingsButtonStyle();

    private:
        AnchorLayoutManager *layout_manager_;
        BackdropWidget *backdrop_widget_;

        QList<ControlPanelButton *> control_buttons_;
        QMap<UiServices::ControlButtonType, ControlPanelButton *> backdrop_area_buttons_map_;
        QMap<UiServices::ControlButtonType, UiServices::UiAction *> action_map_;

        // Contolled core widgets
        SettingsWidget *settings_widget_;
        LanguageWidget* language_widget_;
        ChangeThemeWidget* changetheme_widget_;
        CacheSettingsWidget *cache_settings_widget_;

		//$ BEGIN_MOD $
		ControlPanelButton *previous_button;
		//$ END_MOD $
    };
}

#endif