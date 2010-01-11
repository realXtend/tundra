// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_WebLoginWidget_h
#define incl_UiModule_WebLoginWidget_h

//#include "StableHeaders.h"
#include "UiModuleApi.h"
#include "WebLogin.h"
#include "AbstractLogin.h"

#include <EventHandlers/LoginHandler.h>
#include <NetworkEvents.h>

#include <QtGui>
#include <QUiLoader>
#include <QFile>

namespace CoreUi
{
    class WebLoginWidget : public AbstractLogin
    {

	Q_OBJECT

    public:
        WebLoginWidget(LoginContainer *controller, RexLogic::TaigaLoginHandler *taiga_login_handler);
        virtual ~WebLoginWidget(void);

	private:
        void InitWidget();
		void SetLoginHandler(RexLogic::TaigaLoginHandler *taiga_login_handler);

		WebLogin *web_login_widget_;

    };
}

#endif // incl_UiModule_WebLoginWidget_h