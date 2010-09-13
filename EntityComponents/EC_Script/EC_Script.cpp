// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_Script.h"
#include "IScriptInstance.h"

#include "AttributeInterface.h"
#include "Entity.h"

EC_Script::~EC_Script()
{
    SAFE_DELETE(scriptInstance_);
}

void EC_Script::SetScriptInstance(IScriptInstance *instance)
{
    // If we already have a script instance, unload and delete it.
    if (scriptInstance_)
    {
        scriptInstance_->Stop();
        scriptInstance_->Unload();
        SAFE_DELETE(scriptInstance_);
    }

    scriptInstance_ = instance;
}

void EC_Script::Run()
{
    if (scriptInstance_)
        scriptInstance_->Run();
}

void EC_Script::Stop()
{
    if (scriptInstance_)
        scriptInstance_->Stop();
}

EC_Script::EC_Script(Foundation::ModuleInterface *module):
    Foundation::ComponentInterface(module->GetFramework()),
    scriptRef(this, "Script ref"),
    type(this, "Type"),
    scriptInstance_(0)
{
    connect(this, SIGNAL(OnAttributeChanged(AttributeInterface*, AttributeChange::Type)),
        SLOT(HandleAttributeChanged(AttributeInterface*, AttributeChange::Type)));
    connect(this, SIGNAL(ParentEntitySet()), SLOT(RegisterActions()));
}

void EC_Script::HandleAttributeChanged(AttributeInterface* attribute, AttributeChange::Type change)
{
    if (attribute->GetNameString() == scriptRef.GetNameString())
        emit ScriptRefChanged(scriptRef.Get());
}

void EC_Script::Run(const QString &name)
{
}

void EC_Script::Stop(const QString &name)
{
}

void EC_Script::RegisterActions()
{
    Scene::Entity *entity = GetParentEntity();
    assert(entity);
    if (entity)
    {
        entity->ConnectAction("Run", this, SLOT(Run(const QString &)));
        entity->ConnectAction("Stop", this, SLOT(Stop(const QString &)));
    }
}

