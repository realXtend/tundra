// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_Script.h"
#include "AttributeInterface.h"
#include "IScriptInstance.h"

EC_Script::~EC_Script()
{
}

void EC_Script::SetScriptInstance(IScriptInstance *instance)
{
    // If we already have a script instance, unload and delete it.
    if (scriptInstance_)
    {
        scriptInstance_->Unload();
        SAFE_DELETE(scriptInstance_);
    }

    scriptInstance_ = instance;
}

EC_Script::EC_Script(Foundation::ModuleInterface *module):
    Foundation::ComponentInterface(module->GetFramework()),
    scriptRef(this, "Script ref"),
    type(this, "Type"),
    scriptInstance_(0)
{
    connect(this, SIGNAL(OnAttributeChanged(AttributeInterface*, AttributeChange::Type)),
        SLOT(HandleAttributeChanged(AttributeInterface*, AttributeChange::Type)));
}

void EC_Script::HandleAttributeChanged(AttributeInterface* attribute, AttributeChange::Type change)
{
    if (attribute->GetNameString() == scriptRef.GetNameString())
        emit ScriptRefChanged(scriptRef.Get());
}
