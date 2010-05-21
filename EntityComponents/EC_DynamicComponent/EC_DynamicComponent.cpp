// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_DynamicComponent.h"
#include "ModuleInterface.h"
#include "ModuleManager.h"
#include "Entity.h"

#define LogError(msg) Poco::Logger::get("EC_DynamicComponent").error(std::string("Error: ") + msg);
#define LogInfo(msg) Poco::Logger::get("EC_DynamicComponent").information(msg);

EC_DynamicComponent::EC_DynamicComponent(Foundation::ModuleInterface *module) :
  Foundation::ComponentInterface(module->GetFramework()),
    position_(this, "position", 0)
{
  //QObject::connect(this, SIGNAL(OnChanged()), this, SLOT(UpdateWidgetAndCanvas()));
}

EC_DynamicComponent::~EC_DynamicComponent()
{
}

//void EC_3DCanvasSource::UpdateWidgetAndCanvas()

