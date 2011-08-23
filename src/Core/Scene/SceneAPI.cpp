// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "SceneAPI.h"
#include "Scene.h"
#include "IComponentFactory.h"
#include "IComponent.h"
#include "AssetReference.h"
#include "EntityReference.h"
#include "SceneInteract.h"

#include "Color.h"
#include "Math/Quat.h"
#include "Math/float2.h"
#include "Math/float3.h"
#include "Math/float4.h"
#include "Transform.h"
#include "MemoryLeakCheck.h"

QStringList SceneAPI::attributeTypeNames(QStringList() << "string" << "int" << "real" << "color" << "float2" << "float3" << "float4" << "bool" << "uint" << "quat" <<
        "assetreference" << "assetreferencelist" << "entityreference" << "qvariant" << "qvariantlist" << "transform");

SceneAPI::SceneAPI(Framework *framework) :
    QObject(framework),
    framework_(framework)
{
    sceneInteract = new SceneInteract();
    framework->RegisterDynamicObject("sceneinteract", sceneInteract);

    defaultScene_.reset();
    scenes_.clear();
}

SceneAPI::~SceneAPI()
{
    Reset();
}

void SceneAPI::Reset()
{
    SAFE_DELETE(sceneInteract);
    defaultScene_.reset();
    scenes_.clear();
    componentFactories.clear();
    componentFactoriesByTypeid.clear();
}

void SceneAPI::Initialise()
{
    sceneInteract->Initialize(framework_);
}

SceneInteract *SceneAPI::GetSceneInteract() const
{
    return sceneInteract;
}

bool SceneAPI::HasScene(const QString &name) const
{
    return scenes_.find(name) != scenes_.end();
}

void SceneAPI::SetDefaultScene(const QString &name)
{
    ScenePtr scene = GetScene(name);
    if(scene != defaultScene_)
    {
        defaultScene_ = scene;
        emit DefaultWorldSceneChanged(defaultScene_.get());
    }
}

void SceneAPI::SetDefaultScene(const ScenePtr &scene)
{
    if (scene != defaultScene_)
    {
        defaultScene_ = scene;
        emit DefaultWorldSceneChanged(defaultScene_.get());
    }
}

const ScenePtr &SceneAPI::GetDefaultScene() const
{
    return defaultScene_;
}

Scene* SceneAPI::GetDefaultSceneRaw() const
{
    return defaultScene_.get();
}

ScenePtr SceneAPI::GetScene(const QString &name) const
{
    SceneMap::const_iterator scene = scenes_.find(name);
    if (scene != scenes_.end())
        return scene->second;
    return ScenePtr();
}

ScenePtr SceneAPI::CreateScene(const QString &name, bool viewenabled, bool authority)
{
    if (HasScene(name))
        return ScenePtr();

    ScenePtr newScene = ScenePtr(new Scene(name, framework_, viewenabled, authority));
    if (newScene.get())
    {
        scenes_[name] = newScene;

        // Emit signal of creation
        emit SceneAdded(newScene->Name());
    }
    return newScene;
}

void SceneAPI::RemoveScene(const QString &name)
{
    SceneMap::iterator sceneIter = scenes_.find(name);
    if (sceneIter != scenes_.end())
    {
        // Remove entities before the scene subsystems or worlds are erased by various modules
        sceneIter->second->RemoveAllEntities(false);
        
        // Emit signal about removed scene
        emit SceneRemoved(name);
        
        // If default scene is being removed. Reset our ref so it does not keep ref count alive.
        if (defaultScene_ == sceneIter->second)
            defaultScene_.reset();
        scenes_.erase(sceneIter);
    }
}

const SceneMap &SceneAPI::Scenes() const
{
    return scenes_;
}

bool SceneAPI::IsComponentFactoryRegistered(const QString &typeName)
{
    ComponentFactoryMap::iterator existing = componentFactories.find(typeName);
    return (existing != componentFactories.end() ? true : false);
}

void SceneAPI::RegisterComponentFactory(ComponentFactoryPtr factory)
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

ComponentPtr SceneAPI::CreateComponentByName(Scene* scene, const QString &componentTypename, const QString &newComponentName)
{
    ComponentFactoryPtr factory = GetFactory(componentTypename);
    if (!factory)
    {
        LogError("Cannot create component for type \"" + componentTypename + "\" - no factory exists!");
        return ComponentPtr();
    }
    return factory->Create(scene, newComponentName);
}

ComponentPtr SceneAPI::CreateComponentById(Scene* scene, u32 componentTypeid, const QString &newComponentName)
{
    ComponentFactoryPtr factory = GetFactory(componentTypeid);
    if (!factory)
    {
        LogError("Cannot create component for typeid \"" + QString::number(componentTypeid) + "\" - no factory exists!");
        return ComponentPtr();
    }
    return factory->Create(scene, newComponentName);
}

QString SceneAPI::GetComponentTypeName(u32 componentTypeid)
{
    ComponentFactoryPtr factory = GetFactory(componentTypeid);
    if (factory)
        return factory->TypeName();
    else
        return "";
}

u32 SceneAPI::GetComponentTypeId(const QString &componentTypename)
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
    if (attributeTypeid == cAttributeString)
        attribute = new Attribute<QString>(0, newAttributeName.toStdString().c_str());
    else if (attributeTypeid == cAttributeInt)
        attribute = new Attribute<int>(0, newAttributeName.toStdString().c_str());
    else if (attributeTypeid == cAttributeReal)
        attribute = new Attribute<float>(0, newAttributeName.toStdString().c_str());
    else if (attributeTypeid == cAttributeColor)
        attribute = new Attribute<Color>(0, newAttributeName.toStdString().c_str());
    else if (attributeTypeid == cAttributeFloat2)
        attribute = new Attribute<float2>(0, newAttributeName.toStdString().c_str());
    else if (attributeTypeid == cAttributeFloat3)
        attribute = new Attribute<float3>(0, newAttributeName.toStdString().c_str());
    else if (attributeTypeid == cAttributeFloat4)
        attribute = new Attribute<float4>(0, newAttributeName.toStdString().c_str());
    else if (attributeTypeid == cAttributeBool)
        attribute = new Attribute<bool>(0, newAttributeName.toStdString().c_str());
    else if (attributeTypeid == cAttributeUInt)
        attribute = new Attribute<uint>(0, newAttributeName.toStdString().c_str());
    else if (attributeTypeid == cAttributeQuat)
        attribute = new Attribute<Quat>(0, newAttributeName.toStdString().c_str());
    else if (attributeTypeid == cAttributeAssetReference)
        attribute = new Attribute<AssetReference>(0, newAttributeName.toStdString().c_str());
    else if (attributeTypeid == cAttributeAssetReferenceList)
        attribute = new Attribute<AssetReferenceList>(0, newAttributeName.toStdString().c_str());
    else if (attributeTypeid == cAttributeEntityReference)
        attribute = new Attribute<EntityReference>(0, newAttributeName.toStdString().c_str());
    else if (attributeTypeid == cAttributeQVariant)
        attribute = new Attribute<QVariant>(0, newAttributeName.toStdString().c_str());
    else if (attributeTypeid == cAttributeQVariantList)
        attribute = new Attribute<QVariantList>(0, newAttributeName.toStdString().c_str());
    else if (attributeTypeid == cAttributeTransform)
        attribute = new Attribute<Transform>(0, newAttributeName.toStdString().c_str());
    else
        LogError("Cannot create attribute of type \"" + ToString<int>(attributeTypeid) + "\"! This type is not known to SceneAPI::CreateAttribute!");
    if (attribute)
        attribute->dynamic = true;
    return attribute;
}

QStringList SceneAPI::AttributeTypes()
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

ComponentFactoryPtr SceneAPI::GetFactory(const QString &typeName)
{
    ComponentFactoryMap::iterator factory = componentFactories.find(typeName);
    if (factory == componentFactories.end())
        return ComponentFactoryPtr();
    else
        return factory->second;
}

ComponentFactoryPtr SceneAPI::GetFactory(u32 typeId)
{
    ComponentFactoryWeakMap::iterator factory = componentFactoriesByTypeid.find(typeId);
    if (factory == componentFactoriesByTypeid.end())
        return ComponentFactoryPtr();
    else
        return factory->second.lock();
}
