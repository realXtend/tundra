// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_UiSettingsService_h
#define incl_UiModule_UiSettingsService_h

#include "UiSettingsServiceInterface.h"

namespace CoreUi 
{ 
    class ControlPanelManager; 
}

namespace UiServices
{
    class UiSettingsService : public Foundation::UiSettingsServiceInterface
    {

    Q_OBJECT

    public:
        explicit UiSettingsService(CoreUi::ControlPanelManager *control_panel_manager);
        virtual ~UiSettingsService();

    public slots:
        virtual QObject *GetMainSettingsWidget() const;
        virtual QObject *GetPersonalWidget() const;
        virtual QObject *GetTeleportWidget() const;
        virtual QObject *GetLanguageWidget() const;
        virtual QObject *GeBindingsWidget() const;
        virtual QObject *GetCacheSettingsWidget() const;

    private:
        CoreUi::ControlPanelManager *control_panel_manager_;
    };
}

#endif