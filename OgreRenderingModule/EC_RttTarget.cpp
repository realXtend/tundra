// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_RttTarget.h"
#include "OgreRenderingModule.h"
#include "Frame.h"
#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("EC_RttTarget");


EC_RttTarget::EC_RttTarget(IModule* module) :
    IComponent(module->GetFramework())
    //    owner_(checked_static_cast<OgreRenderer::OgreRenderingModule*>(module))
{
    QObject::connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)),
            SLOT(AttributeUpdated(IAttribute*)));

    if (ViewEnabled())
    {
        ScheduleRender();
    }
}

EC_RttTarget::~EC_RttTarget()
{
}

void EC_RttTarget::ScheduleRender()
{
    framework_->GetFrame()->DelayedExecute(0.1f, this, SLOT(UpdateRtt()));
}

void EC_RttTarget::AttributeUpdated(IAttribute* attribute)
{
}

void EC_RttTarget::UpdateRtt()
{
    LogInfo("Rtt update");
    ScheduleRender();
}

