// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_DynamicComponent.h"
#include "ModuleInterface.h"
#include "ModuleManager.h"
#include "Entity.h"

#include <qvariant.h>

#define LogError(msg) Poco::Logger::get("EC_DynamicComponent").error(std::string("Error: ") + msg);
#define LogInfo(msg) Poco::Logger::get("EC_DynamicComponent").information(msg);

EC_DynamicComponent::EC_DynamicComponent(Foundation::ModuleInterface *module) :
  Foundation::ComponentInterface(module->GetFramework()),
    x_(this, "x", 0)
{
  //QObject::connect(this, SIGNAL(OnChanged()), this, SLOT(UpdateWidgetAndCanvas()));
}

EC_DynamicComponent::~EC_DynamicComponent()
{
}

void EC_DynamicComponent::AddAttribute()
{
    LogInfo("AddAttribute");
}

QVariant EC_DynamicComponent::GetAttribute()
{
    LogInfo("GetAttribute");
    QVariant v(x_.Get());
    return v;
}

void EC_DynamicComponent::SetAttribute(float new_x)
{
    LogInfo("SetAttribute");
    x_.Set(new_x, Foundation::Local);
    ComponentChanged(Foundation::Local);
}
