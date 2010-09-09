#include "StableHeaders.h"
#include "EC_Script.h"
#include "AttributeInterface.h"

EC_Script::EC_Script(Foundation::ModuleInterface *module):
    Foundation::ComponentInterface(module->GetFramework()),
    scriptRef_(this, "Script ref")
{
    connect(this, SIGNAL(OnAttributeChanged(AttributeInterface*, AttributeChange::Type)), this, SLOT(HandleAttributeChanged(AttributeInterface*, AttributeChange::Type)));
}

EC_Script::~EC_Script()
{
    
}

void EC_Script::HandleAttributeChanged(AttributeInterface* attribute, AttributeChange::Type change)
{
    if(attribute->GetNameString() == scriptRef_.GetNameString())
    {
        emit onScriptRefChanged(scriptRef_.Get());
    }
}