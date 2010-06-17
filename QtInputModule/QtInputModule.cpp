// For conditions of distribution and use, see copyright notice in license.txt

#include "ServiceManager.h"
#include "InputEvents.h"
#include "Framework.h"
#include "EventManager.h"
#include "QtInputModule.h"

#include <QGraphicsItem>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QApplication>

#include <boost/make_shared.hpp>

QtInputModule::QtInputModule() 
:ModuleInterfaceImpl("QtInputModule")
{
}

QtInputModule::~QtInputModule()
{
}

void QtInputModule::Initialize()
{
    inputService = boost::make_shared<QtInputService>(framework_);
    framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Input, inputService);
}

static const std::string moduleName("QtInputModule");

const std::string &QtInputModule::NameStatic()
{
    return moduleName;
}

void QtInputModule::Update(f64 frametime)
{
    inputService->Update(frametime);

    RESETPROFILER;
}
