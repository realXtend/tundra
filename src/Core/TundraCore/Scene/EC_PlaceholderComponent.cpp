// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EC_PlaceholderComponent.h"

#include "SceneAPI.h"

#include "Entity.h"
#include "LoggingFunctions.h"
#include "Scene/Scene.h"

#include <QDomDocument>

#include "MemoryLeakCheck.h"

EC_PlaceholderComponent::EC_PlaceholderComponent(Scene* scene):
    IComponent(scene)
{
}

EC_PlaceholderComponent::~EC_PlaceholderComponent()
{
}

void EC_PlaceholderComponent::SetTypeId(u32 newTypeId)
{
    typeId = newTypeId;
}

void EC_PlaceholderComponent::SetTypeName(const QString& newTypeName)
{
    typeName = newTypeName;
}

void EC_PlaceholderComponent::DeserializeFromBinary(kNet::DataDeserializer& source, AttributeChange::Type change)
{
    LogError("EC_PlaceholderComponent can not support deserialization from binary");
}

IAttribute *EC_PlaceholderComponent::CreateAttribute(const QString &typeName, const QString &id, const QString &name, AttributeChange::Type change)
{
    // Check for duplicate
    if (AttributeById(id))
        return IComponent::AttributeById(id);
    if (AttributeByName(name))
        return IComponent::AttributeByName(name);

    IAttribute *attribute = SceneAPI::CreateAttribute(typeName, id);
    if (!attribute)
    {
        LogError("Failed to create new attribute of type \"" + typeName + "\" with ID \"" + id + "\" to placeholder component \"" + Name() + "\".");
        return 0;
    }

    attribute->SetName(name);
    IComponent::AddAttribute(attribute);
    // Placeholder components are supposed to be similar to static-structured components (attributes filled on construction), so do not signal attribute being added

    return attribute;
}
