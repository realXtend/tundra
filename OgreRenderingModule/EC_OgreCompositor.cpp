// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_OgreCompositor.h"
#include "OgreRenderingModule.h"
#include "CompositionHandler.h"

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("EC_OgreCompositor");


EC_OgreCompositor::EC_OgreCompositor(IModule* module) :
    IComponent(module->GetFramework()),
    compositorref(this, "Compositor ref", ""),
    priority(this, "Priority", -1),
    owner_(checked_static_cast<OgreRenderer::OgreRenderingModule*>(module)),
    handler_(owner_->GetRenderer()->GetCompositionHandler())
{
    assert (handler_ && "No CompositionHandler.");

    QObject::connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)),
            SLOT(AttributeUpdated(IAttribute*)));
}

EC_OgreCompositor::~EC_OgreCompositor()
{
}

void EC_OgreCompositor::AttributeUpdated(IAttribute* attribute)
{
    if (attribute == &compositorref)
    {
        UpdateCompositor();
    }

    if (attribute == &priority)
    {
        UpdateCompositor();
    }
}

void EC_OgreCompositor::UpdateCompositor()
{
        if (!previous_ref_.isEmpty())
            handler_->RemoveCompositorFromViewport(previous_ref_.toStdString());

        if (!compositorref.Get().isEmpty())
        {
            if (priority.Get() == -1)
                handler_->AddCompositorForViewport(compositorref.Get().toStdString());
            else
                handler_->AddCompositorForViewportPriority(compositorref.Get().toStdString(), priority.Get());
        }

        previous_ref_ = compositorref.Get();
}

