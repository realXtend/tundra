// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "EC_OgreCompositor.h"
#include "OgreRenderingModule.h"
#include "CompositionHandler.h"

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("EC_OgreCompositor");


EC_OgreCompositor::EC_OgreCompositor(IModule* module) :
    IComponent(module->GetFramework()),
    enabled(this, "Enabled", true),
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

QStringList EC_OgreCompositor::AvailableCompositors() const
{
    return handler_->GetAvailableCompositors().toList();
}

void EC_OgreCompositor::AttributeUpdated(IAttribute* attribute)
{
    if (attribute == &enabled)
    {
        handler_->SetCompositorEnabled(compositorref.Get().toStdString(), enabled.Get());
        UpdateCompositor(compositorref.Get());
        UpdateCompositorParams(compositorref.Get());
    }

    if (attribute == &compositorref)
    {
        UpdateCompositor(compositorref.Get());
    }

    if (attribute == &priority)
    {
        UpdateCompositor(compositorref.Get());
    }

    if (attribute == &parameters)
    {
        UpdateCompositorParams(compositorref.Get());
    }
}

void EC_OgreCompositor::UpdateCompositor(const QString &compositor)
{
    if (ViewEnabled() && enabled.Get())
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

void EC_OgreCompositor::UpdateCompositorParams(const QString &compositor)
{
    if (ViewEnabled() && enabled.Get())
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
                    Ogre::Vector4 value;
                    QStringList valueList = sepParams[1].split(" ", QString::SkipEmptyParts);
                    if (valueList.size() > 0) value.x = boost::lexical_cast<Ogre::Real>(valueList[0].toStdString());
                    if (valueList.size() > 1) value.y = boost::lexical_cast<Ogre::Real>(valueList[1].toStdString());
                    if (valueList.size() > 2) value.z = boost::lexical_cast<Ogre::Real>(valueList[2].toStdString());
                    if (valueList.size() > 3) value.w = boost::lexical_cast<Ogre::Real>(valueList[3].toStdString());
                    std::string name = sepParams[0].toStdString();

                    programParams.push_back(std::make_pair(name, value));
                } catch(boost::bad_lexical_cast &) {}
            }
        }
        handler_->SetCompositorParameter(compositorref.Get().toStdString(), programParams);
        handler_->SetCompositorEnabled(compositorref.Get().toStdString(), false);
        handler_->SetCompositorEnabled(compositorref.Get().toStdString(), true);
    }
}

