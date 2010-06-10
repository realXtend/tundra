// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_UiSettingsServiceInterface_h
#define incl_Interfaces_UiSettingsServiceInterface_h

#include "ServiceInterface.h"
#include <QObject>

namespace Foundation
{
    class UiSettingsServiceInterface;
    typedef boost::shared_ptr<Foundation::UiSettingsServiceInterface> UiSettingServicePtr;

    // Can be used to fetch settings widget qpointers without link dependency
    // this can be used to track ui changes in real time or special emits that these widgets do
    // just check the widgets header and connect to the signals using the qobject pointer
    class UiSettingsServiceInterface : public QObject, public Foundation::ServiceInterface
    {

    Q_OBJECT

    public:
        UiSettingsServiceInterface() {}
        virtual ~UiSettingsServiceInterface() {}

    public slots:
        virtual QObject *GetMainSettingsWidget() const = 0;
        virtual QObject *GetPersonalWidget() const = 0;
        virtual QObject *GetTeleportWidget() const = 0;
        virtual QObject *GetLanguageWidget() const = 0;
        virtual QObject *GeBindingsWidget() const = 0;
        virtual QObject *GetCacheSettingsWidget() const = 0;

    };
}

#endif