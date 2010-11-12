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
    parameters(this, "Parameters"),
    owner_(checked_static_cast<OgreRenderer::OgreRenderingModule*>(module)),
    handler_(owner_->GetRenderer()->GetCompositionHandler())
{
    assert (handler_ && "No CompositionHandler.");

    QObject::connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)),
            SLOT(AttributeUpdated(IAttribute*)));
}

EC_OgreCompositor::~EC_OgreCompositor()
{
    UpdateCompositor("");
}

void EC_OgreCompositor::AttributeUpdated(IAttribute* attribute)
{
    if (attribute == &compositorref)
    {
        UpdateCompositor(compositorref.Get());
    }

    if (attribute == &priority)
    {
        UpdateCompositor(compositorref.Get());
    }

    if (attribute == &parameters && ViewEnabled())
    {
        QList< std::pair<std::string, Ogre::Vector4> > programParams;
        foreach(QVariant keyvalue, parameters.Get())
        {
            QString params = keyvalue.toString();
            QStringList sepParams = params.split('=');
            if (sepParams.size() > 1)
            {
                try
                {
                    float value = boost::lexical_cast<float>(sepParams[1].toStdString());
                    std::string name = sepParams[0].toStdString();

                    programParams.push_back(std::make_pair(name, Ogre::Vector4(value, 0, 0, 0)));
                } catch (boost::bad_lexical_cast &)
                {
                    // no need to handle
                }
            }
        }
        handler_->SetCompositorParameter(compositorref.Get().toStdString(), programParams);
        handler_->SetEnableCompositor(compositorref.Get().toStdString(), false);
        handler_->SetEnableCompositor(compositorref.Get().toStdString(), true);
    }
}

void EC_OgreCompositor::UpdateCompositor(const QString &compositor)
{
    if (ViewEnabled())
    {
        if (!previous_ref_.isEmpty())
            handler_->RemoveCompositorFromViewport(previous_ref_.toStdString());

        if (!compositorref.Get().isEmpty())
        {
            if (priority.Get() == -1)
                handler_->AddCompositorForViewport(compositor.toStdString());
            else
                handler_->AddCompositorForViewportPriority(compositor.toStdString(), priority.Get());
        }

        previous_ref_ = compositor;
    }
}

