// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AbstractLogin.h"
#include "LoginContainer.h"

#include <EventHandlers/LoginHandler.h>

#include <QVBoxLayout>

namespace CoreUi
{

AbstractLogin::AbstractLogin(LoginContainer *controller) :
    QWidget(),
    controller_(controller),
    login_handler_(0)
{
}

void AbstractLogin::SetLayout()
{
    setLayout(new QVBoxLayout(this));
    layout()->setMargin(0);
}

}
