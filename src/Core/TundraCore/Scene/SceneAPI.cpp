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

#include "Color.h"
#include "Math/Quat.h"
#include "Math/float2.h"
#include "Math/float3.h"
#include "Math/float4.h"
#include "Transform.h"
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

ScenePtr SceneAPI::CreateScene(const QString &name, bool viewenabled, bool authority)
{
    if (SceneByName(name))
        return ScenePtr();

    ScenePtr newScene = MAKE_SHARED(Scene, name, framework, viewenabled, authority);
    scenes[name] = newScene;

    // Emit signal of creation
    emit SceneAdded(newScene->Name());
    return newScene;
}

bool SceneAPI::RemoveScene(const QString &name)
{
    SceneMap::iterator sceneIter = scenes.find(name);
    if (sceneIter == scenes.end())
        return false;
    // Remove entities before the scene subsystems or worlds are erased by various modules
    sceneIter->second->RemoveAllEntities(false);
    
    // Emit signal about removed scene
    emit SceneRemoved(name);

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
    return componentFactories.find(typeName) != componentFactories.end();
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
        LogError("Cannot create component for typeid \"" + QString::number(componentTypeid) + "\" - no factory exists!");
        return ComponentPtr();
    }
    return factory->Create(scene, newComponentName);
}

QString SceneAPI::GetComponentTypeName(u32 componentTypeid) const
{
    ComponentFactoryPtr factory = GetFactory(componentTypeid);
    if (factory)
        return factory->TypeName();
    else
        return "";
}

u32 SceneAPI::GetComponentTypeId(const QString &componentTypename) const
{
    ComponentFactoryPtr factory = GetFactory(componentTypename);
    if (factory)
        return factory->TypeId();
    else
        return 0;
}

QString SceneAPI::GetAttributeTypeName(u32 attributeTypeid)
{
    attributeTypeid--; // Skip 0 which is illegal
    if (attributeTypeid < (u32)attributeTypeNames.size())
        return attributeTypeNames[attributeTypeid];
    else
        return "";
}

u32 SceneAPI::GetAttributeTypeId(const QString &attributeTypename)
{
    for (int i = 0; i < attributeTypeNames.size(); ++i)
    {
        if (!attributeTypeNames[i].compare(attributeTypename, Qt::CaseInsensitive))
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

IAttribute *SceneAPI::CreateAttribute(const QString &attributeTypename, const QString &newAttributeName)
{
    return CreateAttribute(GetAttributeTypeId(attributeTypename), newAttributeName);
}

IAttribute* SceneAPI::CreateAttribute(u32 attributeTypeid, const QString& newAttributeName)
{
    IAttribute *attribute = 0;
    switch(attributeTypeid)
    {
    case cAttributeString:
        attribute = new Attribute<QString>(0, newAttributeName.toStdString().c_str()); break;
    case cAttributeInt:
        attribute = new Attribute<int>(0, newAttributeName.toStdString().c_str()); break;
    case cAttributeReal:
        attribute = new Attribute<float>(0, newAttributeName.toStdString().c_str()); break;
    case cAttributeColor:
        attribute = new Attribute<Color>(0, newAttributeName.toStdString().c_str()); break;
    case cAttributeFloat2:
        attribute = new Attribute<float2>(0, newAttributeName.toStdString().c_str()); break;
    case cAttributeFloat3:
        attribute = new Attribute<float3>(0, newAttributeName.toStdString().c_str()); break;
    case cAttributeFloat4:
        attribute = new Attribute<float4>(0, newAttributeName.toStdString().c_str()); break;
    case cAttributeBool:
        attribute = new Attribute<bool>(0, newAttributeName.toStdString().c_str()); break;
    case cAttributeUInt:
        attribute = new Attribute<uint>(0, newAttributeName.toStdString().c_str()); break;
    case cAttributeQuat:
        attribute = new Attribute<Quat>(0, newAttributeName.toStdString().c_str()); break;
    case cAttributeAssetReference:
        attribute = new Attribute<AssetReference>(0, newAttributeName.toStdString().c_str());break;
    case cAttributeAssetReferenceList:
        attribute = new Attribute<AssetReferenceList>(0, newAttributeName.toStdString().c_str());break;
    case cAttributeEntityReference:
        attribute = new Attribute<EntityReference>(0, newAttributeName.toStdString().c_str());break;
    case cAttributeQVariant:
        attribute = new Attribute<QVariant>(0, newAttributeName.toStdString().c_str()); break;
    case cAttributeQVariantList:
        attribute = new Attribute<QVariantList>(0, newAttributeName.toStdString().c_str()); break;
    case cAttributeTransform:
        attribute = new Attribute<Transform>(0, newAttributeName.toStdString().c_str()); break;
    case cAttributeQPoint:
        attribute = new Attribute<QPoint>(0, newAttributeName.toStdString().c_str()); break;
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

QStringList SceneAPI::ComponentTypes() const
{
    QStringList componentTypes;
    for(ComponentFactoryMap::const_iterator iter = componentFactories.begin(); iter != componentFactories.end(); ++iter)
        componentTypes << iter->first;
    return componentTypes;
}

ComponentFactoryPtr SceneAPI::GetFactory(const QString &typeName) const
{
    ComponentFactoryMap::const_iterator factory = componentFactories.find(typeName);
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
