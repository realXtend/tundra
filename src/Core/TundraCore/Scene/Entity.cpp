// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Entity.h"
#include "Scene/Scene.h"
#include "EC_Name.h"
#include "SceneAPI.h"

#include "Framework.h"
#include "IComponent.h"
#include "CoreStringUtils.h"
#include "LoggingFunctions.h"
#include "Profiler.h"

#include <QDomDocument>

#include <kNet/DataSerializer.h>
#include <kNet/DataDeserializer.h>

#include "MemoryLeakCheck.h"

Entity::Entity(Framework* framework, Scene* scene) :
    framework_(framework),
    scene_(scene),
    temporary_(false)
{
}

Entity::Entity(Framework* framework, entity_id_t id, Scene* scene) :
    framework_(framework),
    id_(id),
    scene_(scene),
    temporary_(false)
{
    connect(this, SIGNAL(TemporaryStateToggled(Entity *)), scene_, SIGNAL(EntityTemporaryStateToggled(Entity *)));
}

Entity::~Entity()
{
    // If components still alive, they become free-floating
    for (ComponentMap::const_iterator i = components_.begin(); i != components_.end(); ++i)
        i->second->SetParentEntity(0);
   
    components_.clear();
    qDeleteAll(actions_);
}

void Entity::ChangeComponentId(component_id_t old_id, component_id_t new_id)
{
    if (old_id == new_id)
        return;
    
    ComponentPtr old_comp = GetComponentById(old_id);
    if (!old_comp)
        return;
    
    if (GetComponentById(new_id))
    {
        LogWarning("Purged component " + QString::number(new_id) + " to make room for a ChangeComponentId request. This should not happen.");
        RemoveComponentById(new_id, AttributeChange::LocalOnly);
    }
    
    old_comp->SetNewId(new_id);
    components_.erase(old_id);
    components_[new_id] = old_comp;
}

void Entity::AddComponent(const ComponentPtr &component, AttributeChange::Type change)
{
    AddComponent(0, component, change);
}

void Entity::AddComponent(component_id_t id, const ComponentPtr &component, AttributeChange::Type change)
{
    // Must exist and be free
    if (component && component->ParentEntity() == 0)
    {
        if (!id)
        {
            bool authority = true;
            if (scene_)
                authority = scene_->IsAuthority();
            // Loop until we find a free ID
            for (;;)
            {
                if (authority)
                    id = component->IsReplicated() ? idGenerator_.AllocateReplicated() : idGenerator_.AllocateLocal();
                else
                    id = component->IsReplicated() ? idGenerator_.AllocateUnacked() : idGenerator_.AllocateLocal();
                if (components_.find(id) == components_.end())
                    break;
            }
        }
        else
        {
            component->SetReplicated(id < UniqueIdGenerator::FIRST_LOCAL_ID);
            // If component ID is specified manually, but it already exists, it is an error. Do not add the component in that case.
            if (components_.find(id) != components_.end())
            {
                LogError("Can not add component: a component with id " + QString::number(id) + " already exists in entity " + ToString());
                return;
            }
            // Whenever a manual replicated ID is assigned, reset the ID generator to the highest value to avoid unnecessary free ID probing in the future
            if (id < UniqueIdGenerator::FIRST_LOCAL_ID)
                idGenerator_.ResetReplicatedId(std::max(id, idGenerator_.id));
        }
        
        QString componentTypeName = component->TypeName();
        componentTypeName.replace(0, 3, "");
        componentTypeName = componentTypeName.toLower();
        // We already have 'name' property in Entity, so ignore "EC_Name" ("name") here.
        if (componentTypeName != "name" && !property(componentTypeName.toStdString().c_str()).isValid())
        {
            QVariant var = QVariant::fromValue<QObject*>(component.get());
            setProperty(componentTypeName.toStdString().c_str(), var);
        }
        
        component->SetNewId(id);
        component->SetParentEntity(this);
        components_[id] = component;
        
        if (change != AttributeChange::Disconnected)
            emit ComponentAdded(component.get(), change == AttributeChange::Default ? component->UpdateMode() : change);
        if (scene_)
            scene_->EmitComponentAdded(this, component.get(), change);
    }
}

void Entity::RemoveComponent(const ComponentPtr &component, AttributeChange::Type change)
{
    if (component)
    {
        ComponentMap::iterator iter = components_.find(component->Id());
        if (iter != components_.end())
        {
            QString componentTypeName = component->TypeName();
            componentTypeName.replace(0, 3, "");
            componentTypeName = componentTypeName.toLower();
            
            if(property(componentTypeName.toStdString().c_str()).isValid())
            {
                QObject *obj = property(componentTypeName.toStdString().c_str()).value<QObject*>();
                //Make sure that QObject is inherited by the IComponent.
                if (obj && dynamic_cast<IComponent*>(obj))
                {
                    //Make sure that name is matching incase there are many of same type of components in entity.
                    if (dynamic_cast<IComponent*>(obj)->Name() == component->Name())
                        setProperty(componentTypeName.toStdString().c_str(), QVariant());
                }
            }
            
            if (change != AttributeChange::Disconnected)
                emit ComponentRemoved(iter->second.get(), change == AttributeChange::Default ? component->UpdateMode() : change);
            if (scene_)
                scene_->EmitComponentRemoved(this, iter->second.get(), change);

            iter->second->SetParentEntity(0);
            components_.erase(iter);
        }
        else
        {
            LogWarning("Failed to remove component: " + component->TypeName() + " from entity: " + QString::number(Id()));
        }
    }
}

void Entity::RemoveComponentById(component_id_t id, AttributeChange::Type change)
{
    ComponentPtr comp = GetComponentById(id);
    if (comp)
        RemoveComponent(comp, change);
}

void Entity::RemoveComponentRaw(QObject* comp)
{
    LogWarning("Entity::RemoveComponentRaw: This function is deprecated and will be removed. Use RemoveComponent or RemoveComponentById instead.");
    IComponent* compPtr = dynamic_cast<IComponent*>(comp);
    if (compPtr)
    {
        ComponentPtr ptr = GetComponent(compPtr->TypeName(), compPtr->Name()); //the shared_ptr to this component
        RemoveComponent(ptr);
    }
}

ComponentPtr Entity::GetOrCreateComponent(const QString &type_name, AttributeChange::Type change, bool replicated)
{
    ComponentPtr existing = GetComponent(type_name);
    if (existing)
        return existing;

    return CreateComponent(type_name, change, replicated);
}

ComponentPtr Entity::GetOrCreateComponent(const QString &type_name, const QString &name, AttributeChange::Type change, bool replicated)
{
    ComponentPtr existing = GetComponent(type_name, name);
    if (existing)
        return existing;

    return CreateComponent(type_name, name, change, replicated);
}

ComponentPtr Entity::GetOrCreateComponent(u32 typeId, AttributeChange::Type change, bool replicated)
{
    ComponentPtr existing = GetComponent(typeId);
    if (existing)
        return existing;

    return CreateComponent(typeId, change, replicated);
}

ComponentPtr Entity::GetOrCreateComponent(u32 typeId, const QString &name, AttributeChange::Type change, bool replicated)
{
    ComponentPtr new_comp = GetComponent(typeId, name);
    if (new_comp)
        return new_comp;

    return CreateComponent(typeId, name, change, replicated);
}

ComponentPtr Entity::GetOrCreateLocalComponent(const QString& type_name)
{
    return GetOrCreateComponent(type_name, AttributeChange::LocalOnly, false);
}

ComponentPtr Entity::GetOrCreateLocalComponent(const QString& type_name, const QString& name)
{
    return GetOrCreateComponent(type_name, name, AttributeChange::LocalOnly, false);
}

ComponentPtr Entity::CreateComponent(const QString &type_name, AttributeChange::Type change, bool replicated)
{
    ComponentPtr new_comp = framework_->Scene()->CreateComponentByName(scene_, type_name);
    if (!new_comp)
    {
        LogError("Failed to create a component of type \"" + type_name + "\" to " + ToString());
        return ComponentPtr();
    }

    // If changemode is default, and new component requests to not be replicated by default, honor that
    if (change != AttributeChange::Default || new_comp->IsReplicated())
        new_comp->SetReplicated(replicated);
    
    AddComponent(new_comp, change);
    return new_comp;
}

ComponentPtr Entity::CreateComponent(const QString &type_name, const QString &name, AttributeChange::Type change, bool replicated)
{
    ComponentPtr new_comp = framework_->Scene()->CreateComponentByName(scene_, type_name, name);
    if (!new_comp)
    {
        LogError("Failed to create a component of type \"" + type_name + "\" and name \"" + name + "\" to " + ToString());
        return ComponentPtr();
    }

    // If changemode is default, and new component requests to not be replicated by default, honor that
    if (change != AttributeChange::Default || new_comp->IsReplicated())
        new_comp->SetReplicated(replicated);
    
    AddComponent(new_comp, change);
    return new_comp;
}

ComponentPtr Entity::CreateComponent(u32 typeId, AttributeChange::Type change, bool replicated)
{
    ComponentPtr new_comp = framework_->Scene()->CreateComponentById(scene_, typeId);
    if (!new_comp)
    {
        LogError("Failed to create a component of type id " + QString::number(typeId) + " to " + ToString());
        return ComponentPtr();
    }

    // If changemode is default, and new component requests to not be replicated by default, honor that
    if (change != AttributeChange::Default || new_comp->IsReplicated())
        new_comp->SetReplicated(replicated);
    
    AddComponent(new_comp, change);
    return new_comp;
}

ComponentPtr Entity::CreateComponent(u32 typeId, const QString &name, AttributeChange::Type change, bool replicated)
{
    ComponentPtr new_comp = framework_->Scene()->CreateComponentById(scene_, typeId, name);
    if (!new_comp)
    {
        LogError("Failed to create a component of type id " + QString::number(typeId) + " and name \"" + name + "\" to " + ToString());
        return ComponentPtr();
    }

    // If changemode is default, and new component requests to not be replicated by default, honor that
    if (change != AttributeChange::Default || new_comp->IsReplicated())
        new_comp->SetReplicated(replicated);
    
    AddComponent(new_comp, change);
    return new_comp;
}

ComponentPtr Entity::CreateComponentWithId(component_id_t compId, u32 typeId, const QString &name, AttributeChange::Type change)
{
    ComponentPtr new_comp = framework_->Scene()->CreateComponentById(scene_, typeId, name);
    if (!new_comp)
    {
        LogError("Failed to create a component of type id " + QString::number(typeId) + " and name \"" + name + "\" to " + ToString());
        return ComponentPtr();
    }

    // If this overload is called with id 0, it must come from SyncManager (server). In that case make sure we do not allow the component to be created as local
    if (!compId)
        new_comp->SetReplicated(true);

    AddComponent(compId, new_comp, change);
    return new_comp;
}

ComponentPtr Entity::CreateLocalComponent(const QString &type_name)
{
    return CreateComponent(type_name, AttributeChange::LocalOnly, false);
}

ComponentPtr Entity::CreateLocalComponent(const QString &type_name, const QString &name)
{
    return CreateComponent(type_name, name, AttributeChange::LocalOnly, false);
}

ComponentPtr Entity::GetComponentById(component_id_t id) const
{
    ComponentMap::const_iterator i = components_.find(id);
    if (i != components_.end())
        return i->second;
    else
        return ComponentPtr();
}

ComponentPtr Entity::GetComponent(const QString &type_name) const
{
    for (ComponentMap::const_iterator i = components_.begin(); i != components_.end(); ++i)
        if (i->second->TypeName() == type_name)
            return i->second;

    return ComponentPtr();
}

ComponentPtr Entity::GetComponent(u32 typeId) const
{
    for (ComponentMap::const_iterator i = components_.begin(); i != components_.end(); ++i)
        if (i->second->TypeId() == typeId)
            return i->second;

    return ComponentPtr();
}

Entity::ComponentVector Entity::GetComponents(const QString &type_name) const
{
    ComponentVector ret;
    for (ComponentMap::const_iterator i = components_.begin(); i != components_.end(); ++i)
        if (i->second->TypeName() == type_name)
            ret.push_back(i->second);
    return ret;
}

ComponentPtr Entity::GetComponent(const QString &type_name, const QString& name) const
{
    for (ComponentMap::const_iterator i = components_.begin(); i != components_.end(); ++i)
        if (i->second->TypeName() == type_name && i->second->Name() == name)
            return i->second;

    return ComponentPtr();
}

ComponentPtr Entity::GetComponent(u32 typeId, const QString& name) const
{
    for (ComponentMap::const_iterator i = components_.begin(); i != components_.end(); ++i)
        if (i->second->TypeId() == typeId && i->second->Name() == name)
            return i->second;

    return ComponentPtr();
}

QObjectList Entity::GetComponentsRaw(const QString &type_name) const
{
    LogWarning("Entity::GetComponentsRaw: This function is deprecated and will be removed. Use GetComponents or Components instead.");
    QObjectList ret;
    if (type_name.isNull())
        for (ComponentMap::const_iterator i = components_.begin(); i != components_.end(); ++i)
            ret.push_back(i->second.get());
    else
        for (ComponentMap::const_iterator i = components_.begin(); i != components_.end(); ++i)
            if (i->second->TypeName() == type_name)
                ret.push_back(i->second.get());
    return ret;
}

IAttribute *Entity::GetAttribute(const QString &name) const
{
    for (ComponentMap::const_iterator i = components_.begin(); i != components_.end(); ++i)
    {
        IAttribute *attr = i->second->GetAttribute(name);
        if (attr)
            return attr;
    }
    return 0;
}

void Entity::SerializeToBinary(kNet::DataSerializer &dst) const
{
    dst.Add<u32>(Id());
    dst.Add<u8>(IsReplicated() ? 1 : 0);
    uint num_serializable = 0;
    for (ComponentMap::const_iterator i = components_.begin(); i != components_.end(); ++i)
        if (!i->second->IsTemporary())
            num_serializable++;
    dst.Add<u32>(num_serializable);
    for (ComponentMap::const_iterator i = components_.begin(); i != components_.end(); ++i)
        if (!i->second->IsTemporary())
        {
            dst.Add<u32>(i->second->TypeId()); ///\todo VLE this!
            dst.AddString(i->second->Name().toStdString());
            dst.Add<u8>(i->second->IsReplicated() ? 1 : 0);
            
            // Write each component to a separate buffer, then write out its size first, so we can skip unknown components
            QByteArray comp_bytes;
            // Assume 64KB max per component for now
            comp_bytes.resize(64 * 1024);
            kNet::DataSerializer comp_dest(comp_bytes.data(), comp_bytes.size());
            i->second->SerializeToBinary(comp_dest);
            comp_bytes.resize(comp_dest.BytesFilled());
            
            dst.Add<u32>(comp_bytes.size());
            dst.AddArray<u8>((const u8*)comp_bytes.data(), comp_bytes.size());
        }
}

/* Disabled for now, since have to decide how entityID conflicts are handled.
void Entity::DeserializeFromBinary(kNet::DataDeserializer &src, AttributeChange::Type change)
{
}*/

void Entity::SerializeToXML(QDomDocument &doc, QDomElement &base_element, bool serializeTemporary) const
{
    QDomElement entity_elem = doc.createElement("entity");
    entity_elem.setAttribute("id", QString::number(Id()));
    entity_elem.setAttribute("sync", BoolToString(IsReplicated()));
    if (serializeTemporary)
        entity_elem.setAttribute("temporary", BoolToString(IsTemporary()));

    for (ComponentMap::const_iterator i = components_.begin(); i != components_.end(); ++i)
            i->second->SerializeTo(doc, entity_elem, serializeTemporary);

    base_element.appendChild(entity_elem);
}

/* Disabled for now, since have to decide how entityID conflicts are handled.
void Entity::DeserializeFromXML(QDomElement& element, AttributeChange::Type change)
{
}*/

QString Entity::SerializeToXMLString(bool serializeTemporary) const
{
    QDomDocument scene_doc("Scene");
    QDomElement scene_elem = scene_doc.createElement("scene");

    SerializeToXML(scene_doc, scene_elem, serializeTemporary);
    return scene_doc.toString();
}

/* Disabled for now, since have to decide how entityID conflicts are handled.
bool Entity::DeserializeFromXMLString(const QString &src, AttributeChange::Type change)
{
    QDomDocument entityDocument("Entity");
    if (!entityDocument.setContent(src))
    {
        LogError("Parsing entity XML from text failed.");
        return false;
    }

    return CreateContentFromXml(entityDocument, replaceOnConflict, change);
}*/

AttributeVector Entity::GetAttributes(const QString &name) const
{
    std::vector<IAttribute *> ret;
    for (ComponentMap::const_iterator i = components_.begin(); i != components_.end(); ++i)
    {
        IAttribute *attr = i->second->GetAttribute(name);
        if (attr)
            ret.push_back(attr);
    }
    return ret;
}

EntityPtr Entity::Clone(bool local, bool temporary) const
{
    QDomDocument doc("Scene");
    QDomElement sceneElem = doc.createElement("scene");
    QDomElement entityElem = doc.createElement("entity");
    entityElem.setAttribute("sync", BoolToString(!local));
    entityElem.setAttribute("id", local ? scene_->NextFreeIdLocal() : scene_->NextFreeId());
    for (ComponentMap::const_iterator i = components_.begin(); i != components_.end(); ++i)
        i->second->SerializeTo(doc, entityElem);

    sceneElem.appendChild(entityElem);
    doc.appendChild(sceneElem);

    QList<Entity *> entities = scene_->CreateContentFromXml(doc, true, AttributeChange::Default);
    if (entities.size() && entities.first())
    {
        entities.first()->SetTemporary(temporary);
        return entities.first()->shared_from_this();
    }
    else
        return EntityPtr();
}

void Entity::SetName(const QString &name)
{
    ComponentPtr comp = GetOrCreateComponent(EC_Name::TypeNameStatic(), AttributeChange::Default, true);
    EC_Name * ecName = checked_static_cast<EC_Name*>(comp.get());
    ecName->name.Set(name, AttributeChange::Default);
}

QString Entity::Name() const
{
    shared_ptr<EC_Name> name = GetComponent<EC_Name>();
    if (name)
        return name->name.Get();
    else
        return QString();
}

void Entity::SetDescription(const QString &desc)
{
    ComponentPtr comp = GetOrCreateComponent(EC_Name::TypeNameStatic(), AttributeChange::Default, true);
    EC_Name * ecName = checked_static_cast<EC_Name*>(comp.get());
    ecName->description.Set(desc, AttributeChange::Default);
}

QString Entity::Description() const
{
    shared_ptr<EC_Name> name = GetComponent<EC_Name>();
    if (name)
        return name->description.Get();
    else
        return QString();
}

EntityAction *Entity::Action(const QString &name)
{
    foreach(EntityAction *action, actions_)
        if (action->Name().compare(name, Qt::CaseInsensitive) == 0)
            return action;

    EntityAction *action = new EntityAction(name);
    actions_.insert(name, action);
    return action;
}

void Entity::RemoveAction(const QString &name)
{
    ActionMap::iterator iter = actions_.find(name);
    if (iter != actions_.end())
    {
        (*iter)->deleteLater();
        actions_.erase(iter);
    }
}

void Entity::ConnectAction(const QString &name, const QObject *receiver, const char *member)
{
    EntityAction *action = Action(name);
    assert(action);
    connect(action, SIGNAL(Triggered(QString, QString, QString, QStringList)), receiver, member, Qt::UniqueConnection);
}

void Entity::Exec(EntityAction::ExecTypeField type, const QString &action, const QString &p1, const QString &p2, const QString &p3)
{
    Exec(type, action, QStringList(QStringList() << p1 << p2 << p3));
}

void Entity::Exec(EntityAction::ExecTypeField type, const QString &action, const QStringList &params)
{
    PROFILE(Entity_ExecEntityAction);
    
    EntityAction *act = Action(action);
    if ((type & EntityAction::Local) != 0)
    {
        if (params.size() == 0)
            act->Trigger();
        else if (params.size() == 1)
            act->Trigger(params[0]);
        else if (params.size() == 2)
            act->Trigger(params[0], params[1]);
        else if (params.size() == 3)
            act->Trigger(params[0], params[1], params[2]);
        else if (params.size() >= 4)
            act->Trigger(params[0], params[1], params[2], params.mid(3));
    }

    if (ParentScene())
        ParentScene()->EmitActionTriggered(this, action, params, type);
}

void Entity::Exec(EntityAction::ExecTypeField type, const QString &action, const QVariantList &params)
{
    QStringList stringParams;
    foreach(QVariant var, params)
        stringParams << var.toString();
    Exec(type, action, stringParams);
}

void Entity::EmitEntityRemoved(AttributeChange::Type change)
{
    if (change == AttributeChange::Disconnected)
        return;
    emit EntityRemoved(this, change);
}

void Entity::EmitEnterView(IComponent* camera)
{
    emit EnterView(camera);
}

void Entity::EmitLeaveView(IComponent* camera)
{
    emit LeaveView(camera);
}

void Entity::SetTemporary(bool enable)
{
    if (enable != temporary_)
    {
        temporary_ = enable;
        emit TemporaryStateToggled(this);
    }
}

QString Entity::ToString() const
{
    QString name = Name();
    if (name.trimmed().isEmpty())
        return QString("Entity ID ") + QString::number(Id());
    else
        return QString("Entity \"") + name + "\" (ID: " + QString::number(Id()) + ")";
}

QObjectList Entity::ComponentsList() const
{
    LogWarning("Entity::ComponentsLis: this function is deprecated and will be removed. Use Entity::Components instead");
    QObjectList compList;
    for (ComponentMap::const_iterator i = components_.begin(); i != components_.end(); ++i)
        if (i->second.get())
            compList << i->second.get();
    return compList;
}
