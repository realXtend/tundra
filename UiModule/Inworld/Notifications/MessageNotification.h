// For conditions of distribution and use, see copyright notice in license.txt

#include "NotificationBaseWidget.h"
#include "UiModuleApi.h"

namespace UiServices
{
    class UI_MODULE_API MessageNotification : public CoreUi::NotificationBaseWidget
    {

    Q_OBJECT

    public:
        MessageNotification(QString message, int hide_in_msec = 10000);

    };
}