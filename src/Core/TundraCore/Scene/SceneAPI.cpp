// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "SceneAPI.h"
#include "Scene/Scene.h"
#include "IComponentFactory.h"
#include "IComponent.h"
#include "IRenderer.h"
#include "AssetReference.h"
#include "EntityReference.h"
#include "LoggingFunctions.h"
#include "Framework.h"
#include "Color.h"
#include "Math/Quat.h"
#include "Math/float2.h"
#include "Math/float3.h"
#include "Math/float4.h"
#include "Transform.h"
#include "EC_PlaceholderComponent.h"

#include <QDomElement>

#include "MemoryLeakCheck.h"

const QStringList SceneAPI::attributeTypeNames(
    QStringList() << cAttributeStringTypeName << cAttributeIntTypeName << cAttributeRealTypeName <<
    cAttributeColorTypeName << cAttributeFloat2TypeName << cAttributeFloat3TypeName << cAttributeFloat4TypeName <<
    cAttributeBoolTypeName << cAttributeUIntTypeName << cAttributeQuatTypeName << cAttributeAssetReferenceTypeName <<
    cAttributeAssetReferenceListTypeName << cAttributeEntityReferenceTypeName << cAttributeQVariantTypeName <<
    cAttributeQVariantListTypeName << cAttributeTransformTypeName << cAttributeQPointTypeName);

SceneAPI::SceneAPI(Framework *owner) :
    QObject(owner),
    framework(owner)
{
}

SceneAPI::~SceneAPI()
{
    Reset();
}

void SceneAPI::Reset()
{
    scenes.clear();
    componentFactories.clear();
    componentFactoriesByTypeid.clear();
}

ScenePtr SceneAPI::SceneByName(const QString &name) const
{
    SceneMap::const_iterator scene = scenes.find(name);
    if (scene != scenes.end())
        return scene->second;
    return ScenePtr();
}

Scene *SceneAPI::MainCameraScene()
{
    if (!framework || !framework->Renderer())
        return 0;
    return framework->Renderer()->MainCameraScene();
}

ScenePtr SceneAPI::CreateScene(const QString &name, bool viewEnabled, bool authority, AttributeChange::Type change)
{
    if (SceneByName(name))
        return ScenePtr();

    ScenePtr newScene = MAKE_SHARED(Scene, name, framework, viewEnabled, authority);
    scenes[name] = newScene;

    // Emit signal of creation
    if (change != AttributeChange::Disconnected)
    {
        emit SceneCreated(newScene.get(), change);
        emit SceneAdded(newScene->Name());
    }

    return newScene;
}

bool SceneAPI::RemoveScene(const QString &name, AttributeChange::Type change)
{
    SceneMap::iterator sceneIter = scenes.find(name);
    if (sceneIter == scenes.end())
        return false;

    // Remove entities before the scene subsystems or worlds are erased by various modules
    sceneIter->second->RemoveAllEntities(false, change);

    // Emit signal about removed scene
    if (change != AttributeChange::Disconnected)
    {
        emit SceneAboutToBeRemoved(sceneIter->second.get(), change);
        emit SceneRemoved(name);
    }

    scenes.erase(sceneIter);
    return true;
}

const SceneMap &SceneAPI::Scenes() const
{
    return scenes;
}

SceneMap &SceneAPI::Scenes()
{
    return scenes;
}

bool SceneAPI::IsComponentFactoryRegistered(const QString &typeName) const
{
    return componentFactories.find(IComponent::EnsureTypeNameWithPrefix(typeName)) != componentFactories.end();
}

bool SceneAPI::IsPlaceholderComponentRegistered(const QString &typeName) const
{
    return placeholderComponentTypeIds.find(IComponent::EnsureTypeNameWithPrefix(typeName)) != placeholderComponentTypeIds.end();
}

bool SceneAPI::IsComponentTypeRegistered(const QString& typeName) const
{
    QString nameWithPrefix = IComponent::EnsureTypeNameWithPrefix(typeName);
    return componentFactories.find(nameWithPrefix) != componentFactories.end() ||
        placeholderComponentTypeIds.find(nameWithPrefix) != placeholderComponentTypeIds.end();
}

void SceneAPI::RegisterComponentFactory(const ComponentFactoryPtr &factory)
{
    if (factory->TypeName().trimmed() != factory->TypeName() || factory->TypeName().isEmpty() || factory->TypeId() == 0)
    {
        LogError("Cannot add a new ComponentFactory for component typename \"" + factory->TypeName() + "\" and typeid " + QString::number(factory->TypeId()) + ". Invalid input!");
        return;
    }

    ComponentFactoryMap::iterator existing = componentFactories.find(factory->TypeName());
    ComponentFactoryWeakMap::iterator existing2 = componentFactoriesByTypeid.find(factory->TypeId());
    ComponentFactoryPtr existingFactory;
    if (existing != componentFactories.end())
        existingFactory = existing->second;
    if (!existingFactory && existing2 != componentFactoriesByTypeid.end())
        existingFactory = existing2->second.lock();

    if (existingFactory)
    {
        LogError("Cannot add a new ComponentFactory for component typename \"" + factory->TypeName() + "\" and typeid " + QString::number(factory->TypeId()) + ". Conflicting type factory with typename " + existingFactory->TypeName() + " and typeid " + QString::number(existingFactory->TypeId()) + " already exists!");
        return;
    }

    componentFactories[factory->TypeName()] = factory;
    componentFactoriesByTypeid[factory->TypeId()] = factory;
}

ComponentPtr SceneAPI::CreateComponentByName(Scene* scene, const QString &componentTypename, const QString &newComponentName) const
{
    ComponentFactoryPtr factory = GetFactory(componentTypename);
    if (!factory)
    {
        // If no actual factory, try creating a placeholder component
        PlaceholderComponentTypeIdMap::const_iterator i = placeholderComponentTypeIds.find(IComponent::EnsureTypeNameWithPrefix(componentTypename));
        if (i != placeholderComponentTypeIds.end())
            return CreatePlaceholderComponentById(scene, i->second, newComponentName);

        LogError("Cannot create component for type \"" + componentTypename + "\" - no factory exists!");
        return ComponentPtr();
    }
    return factory->Create(scene, newComponentName);
}

ComponentPtr SceneAPI::CreateComponentById(Scene* scene, u32 componentTypeid, const QString &newComponentName) const
{
    ComponentFactoryPtr factory = GetFactory(componentTypeid);
    if (!factory)
    {
        // If no actual factory, try creating a placeholder component
        PlaceholderComponentTypeMap::const_iterator i = placeholderComponentTypes.find(componentTypeid);
        if (i != placeholderComponentTypes.end())
            return CreatePlaceholderComponentById(scene, componentTypeid, newComponentName);

        LogError("Cannot create component for typeid \"" + QString::number(componentTypeid) + "\" - no factory exists!");
        return ComponentPtr();
    }
    return factory->Create(scene, newComponentName);
}

QString SceneAPI::ComponentTypeNameForTypeId(u32 componentTypeid) const
{
    ComponentFactoryPtr factory = GetFactory(componentTypeid);
    if (factory)
        return factory->TypeName();
    else
    {
        // Check also placeholder types
        PlaceholderComponentTypeMap::const_iterator i = placeholderComponentTypes.find(componentTypeid);
        if (i != placeholderComponentTypes.end())
            return i->second.typeName;
        else
            return "";
    }
}

u32 SceneAPI::ComponentTypeIdForTypeName(const QString &componentTypename) const
{
    ComponentFactoryPtr factory = GetFactory(componentTypename);
    if (factory)
        return factory->TypeId();
    else
    {
        // Check also placeholder types
        PlaceholderComponentTypeIdMap::const_iterator i = placeholderComponentTypeIds.find(IComponent::EnsureTypeNameWithPrefix(componentTypename));
        if (i != placeholderComponentTypeIds.end())
            return i->second;
        else
            return 0;
    }
}

QString SceneAPI::AttributeTypeNameForTypeId(u32 attributeTypeid)
{
    attributeTypeid--; // Skip 0 which is illegal
    if (attributeTypeid < (u32)attributeTypeNames.size())
        return attributeTypeNames[attributeTypeid];
    else
        return "";
}

u32 SceneAPI::AttributeTypeIdForTypeName(const QString &attributeTypename)
{
    for (int i = 0; i < attributeTypeNames.size(); ++i)
    {
        if (attributeTypeNames[i].compare(attributeTypename, Qt::CaseInsensitive) == 0)
            return i + 1; // 0 is illegal, actual types start from 1
    }
    return 0;
}

/*
ComponentPtr SceneAPI::CloneComponent(const ComponentPtr &component, const QString &newComponentName)
{
    ComponentFactoryPtr factory = GetFactory(component->TypeId());
    if (!factory)
        return ComponentPtr();

    return factory->Clone(component.get(), newComponentName);
}
*/

IAttribute *SceneAPI::CreateAttribute(const QString &attributeTypename, const QString &newAttributeId)
{
    return CreateAttribute(GetAttributeTypeId(attributeTypename), newAttributeId);
}

IAttribute* SceneAPI::CreateAttribute(u32 attributeTypeid, const QString& newAttributeId)
{
    IAttribute *attribute = 0;
    switch(attributeTypeid)
    {
    case cAttributeString:
        attribute = new Attribute<QString>(0, newAttributeId.toStdString().c_str()); break;
    case cAttributeInt:
        attribute = new Attribute<int>(0, newAttributeId.toStdString().c_str()); break;
    case cAttributeReal:
        attribute = new Attribute<float>(0, newAttributeId.toStdString().c_str()); break;
    case cAttributeColor:
        attribute = new Attribute<Color>(0, newAttributeId.toStdString().c_str()); break;
    case cAttributeFloat2:
        attribute = new Attribute<float2>(0, newAttributeId.toStdString().c_str()); break;
    case cAttributeFloat3:
        attribute = new Attribute<float3>(0, newAttributeId.toStdString().c_str()); break;
    case cAttributeFloat4:
        attribute = new Attribute<float4>(0, newAttributeId.toStdString().c_str()); break;
    case cAttributeBool:
        attribute = new Attribute<bool>(0, newAttributeId.toStdString().c_str()); break;
    case cAttributeUInt:
        attribute = new Attribute<uint>(0, newAttributeId.toStdString().c_str()); break;
    case cAttributeQuat:
        attribute = new Attribute<Quat>(0, newAttributeId.toStdString().c_str()); break;
    case cAttributeAssetReference:
        attribute = new Attribute<AssetReference>(0, newAttributeId.toStdString().c_str());break;
    case cAttributeAssetReferenceList:
        attribute = new Attribute<AssetReferenceList>(0, newAttributeId.toStdString().c_str());break;
    case cAttributeEntityReference:
        attribute = new Attribute<EntityReference>(0, newAttributeId.toStdString().c_str());break;
    case cAttributeQVariant:
        attribute = new Attribute<QVariant>(0, newAttributeId.toStdString().c_str()); break;
    case cAttributeQVariantList:
        attribute = new Attribute<QVariantList>(0, newAttributeId.toStdString().c_str()); break;
    case cAttributeTransform:
        attribute = new Attribute<Transform>(0, newAttributeId.toStdString().c_str()); break;
    case cAttributeQPoint:
        attribute = new Attribute<QPoint>(0, newAttributeId.toStdString().c_str()); break;
    default:
        LogError("Cannot create attribute of type \"" + QString::number(attributeTypeid) + "\"! This type is not known to SceneAPI::CreateAttribute!");
        break;
    }

    if (attribute)
        attribute->dynamic = true;
    return attribute;
}

const QStringList &SceneAPI::AttributeTypes()
{
    return attributeTypeNames;
}

void SceneAPI::RegisterPlaceholderComponentType(QDomElement& element, AttributeChange::Type change)
{
    ComponentDesc desc;
    if (!element.hasAttribute("type"))
    {
        LogError("Component XML element is missing type attribute, can not register placeholder component type");
        return;
    }

    desc.typeId = ParseUInt(element.attribute("typeId"), 0xffffffff);
    desc.typeName = IComponent::EnsureTypeNameWithPrefix(element.attribute("type"));
    desc.name = element.attribute("name");

    QDomElement child = element.firstChildElement("attribute");
    while(!child.isNull())
    {
        AttributeDesc attr;
        attr.id = child.attribute("id");
        // Fallback if ID is not defined
        if (attr.id.isEmpty())
            attr.id = child.attribute("name");
        attr.name = child.attribute("name");
        attr.typeName = child.attribute("type");
        attr.value = child.attribute("value");
        
        // Older scene content does not have attribute typenames, these can not be used
        if (!attr.typeName.isEmpty())
            desc.attributes.push_back(attr);
        else
            LogWarning("Can not store placeholder component attribute " + attr.name + ", no type specified");

        child = child.nextSiblingElement("attribute");
    }

    RegisterPlaceholderComponentType(desc, change);
}

void SceneAPI::RegisterPlaceholderComponentType(ComponentDesc desc, AttributeChange::Type change)
{
    // If no typeid defined, generate from the name
    // (eg. if script is registering a type, do not require it to invent a typeID)
    if (desc.typeId == 0 || desc.typeId == 0xffffffff)
        desc.typeId = (qHash(desc.typeName) & 0xffff) | 0x10000;

    desc.typeName = IComponent::EnsureTypeNameWithPrefix(desc.typeName);

    if (GetFactory(desc.typeId))
    {
        LogError("Component factory for component typeId " + QString::number(desc.typeId) + " already exists, can not register placeholder component type");
        return;
    }
    if (desc.typeName.isEmpty())
    {
        LogError("Empty typeName in placeholder component description, can not register");
        return;
    }
    if (placeholderComponentTypes.find(desc.typeId) != placeholderComponentTypes.end())
    {
        LogWarning("Placeholder component type " + desc.typeName + " already registered, re-registering skipped");
        return;
    }

    placeholderComponentTypes[desc.typeId] = desc;
    placeholderComponentTypeIds[desc.typeName] = desc.typeId;
    LogInfo("Registered placeholder component type " + desc.typeName);

    emit PlaceholderComponentTypeRegistered(desc.typeId, desc.typeName, change);
}

void SceneAPI::RegisterComponentType(const QString& typeName, IComponent* component)
{
    if (!component)
        return;

    ComponentDesc desc;
    desc.typeName = typeName;
    desc.typeId = 0xffffffff; // Calculate from hash in RegisterPlaceholderComponentType()
    const AttributeVector& attrs = component->Attributes();
    for (size_t i = 0; i < attrs.size(); ++i)
    {
        IAttribute* attr = attrs[i];
        if (!attr)
            continue;
        AttributeDesc attrDesc;
        attrDesc.id = attr->Id();
        attrDesc.name = attr->Name();
        attrDesc.typeName = attr->TypeName();
        desc.attributes.push_back(attrDesc);
    }

    RegisterPlaceholderComponentType(desc);
}

ComponentPtr SceneAPI::CreatePlaceholderComponentById(Scene* scene, u32 componentTypeid, const QString &newComponentName) const
{
    PlaceholderComponentTypeMap::const_iterator i = placeholderComponentTypes.find(componentTypeid);
    if (i == placeholderComponentTypes.end())
    {
        LogError("Unknown placeholder component type " + QString::number(componentTypeid) + ", can not create placeholder component");
        return ComponentPtr();
    }

    const ComponentDesc& desc = i->second;

    EC_PlaceholderComponent* component = new EC_PlaceholderComponent(scene);
    component->SetTypeId(componentTypeid);
    component->SetTypeName(desc.typeName);
    component->SetName(newComponentName);

    for (int j = 0; j < desc.attributes.length(); ++j)
    {
        const AttributeDesc& attr = desc.attributes[j];
        component->CreateAttribute(attr.typeName, attr.id, attr.name);
    }

    return ComponentPtr(component);
}

QStringList SceneAPI::ComponentTypes() const
{
    QStringList componentTypes;
    for(ComponentFactoryMap::const_iterator iter = componentFactories.begin(); iter != componentFactories.end(); ++iter)
        componentTypes << iter->first;
    return componentTypes;
}

ComponentFactoryPtr SceneAPI::GetFactory(const QString &typeName) const
{
    ComponentFactoryMap::const_iterator factory = componentFactories.find(IComponent::EnsureTypeNameWithPrefix(typeName));
    if (factory == componentFactories.end())
        return ComponentFactoryPtr();
    else
        return factory->second;
}

ComponentFactoryPtr SceneAPI::GetFactory(u32 typeId) const
{
    ComponentFactoryWeakMap::const_iterator factory = componentFactoriesByTypeid.find(typeId);
    if (factory == componentFactoriesByTypeid.end())
        return ComponentFactoryPtr();
    else
        return factory->second.lock();
}
