// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_AbstractLogin_h
#define incl_UiModule_AbstractLogin_h

//#include <EventHandlers/LoginHandler.h>
#include <QWidget>

namespace RexLogic
{
    class AbstractLoginHandler;
}

namespace CoreUi
{
    class LoginContainer;

    class AbstractLogin : public QWidget
    {
        Q_OBJECT

    public:
        AbstractLogin(LoginContainer *controller)
            : QWidget(),
              controller_(controller),
              login_handler_(0) {}

        virtual void InitWidget() = 0;

        LoginContainer *controller_;
        RexLogic::AbstractLoginHandler *login_handler_;
    };
}

#endif // incl_UiModule_AbstractLogin_h
