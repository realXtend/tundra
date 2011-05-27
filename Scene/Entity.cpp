// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Entity.h"
#include "Scene.h"
#include "EC_Name.h"
#include "SceneAPI.h"

#include "Framework.h"
#include "IComponent.h"
#include "CoreStringUtils.h"
#include "LoggingFunctions.h"

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

Entity::Entity(Framework* framework, uint id, Scene* scene) :
    framework_(framework),
    id_(id),
    scene_(scene),
    temporary_(false)
{
}

Entity::~Entity()
{
    // If components still alive, they become free-floating
    for(size_t i=0 ; i<components_.size() ; ++i)
        components_[i]->SetParentEntity(0);
   
    components_.clear();
    qDeleteAll(actions_);
}

void Entity::AddComponent(const ComponentPtr &component, AttributeChange::Type change)
{
    // Must exist and be free
    if (component && component->GetParentEntity() == 0)
    {
        QString componentTypeName = component->TypeName();
        componentTypeName.replace(0, 3, "");
        componentTypeName = componentTypeName.toLower();
        if(!property(componentTypeName.toStdString().c_str()).isValid())
        {
            QVariant var = QVariant::fromValue<QObject*>(component.get());
            setProperty(componentTypeName.toStdString().c_str(), var);
        }

        component->SetParentEntity(this);
        components_.push_back(component);
        
        if (change != AttributeChange::Disconnected)
            emit ComponentAdded(component.get(), change == AttributeChange::Default ? component->GetUpdateMode() : change);
        if (scene_)
            scene_->EmitComponentAdded(this, component.get(), change);
    }
}

void Entity::RemoveComponent(const ComponentPtr &component, AttributeChange::Type change)
{
    if (component)
    {
        ComponentVector::iterator iter = std::find(components_.begin(), components_.end(), component);
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
                emit ComponentRemoved((*iter).get(), change == AttributeChange::Default ? component->GetUpdateMode() : change);
            if (scene_)
                scene_->EmitComponentRemoved(this, (*iter).get(), change);

            (*iter)->SetParentEntity(0);
            components_.erase(iter);
        }
        else
        {
            LogWarning("Failed to remove component: " + component->TypeName() + " from entity: " + QString::number(GetId()));
        }
    }
}

void Entity::RemoveComponentRaw(QObject* comp)
{
    IComponent* compPtr = dynamic_cast<IComponent*>(comp);
    if (compPtr)
    {
        ComponentPtr ptr = GetComponent(compPtr->TypeName(), compPtr->Name()); //the shared_ptr to this component
        RemoveComponent(ptr);
    }
}

ComponentPtr Entity::GetOrCreateComponent(const QString &type_name, AttributeChange::Type change, bool syncEnabled)
{
    ComponentPtr new_comp = GetComponent(type_name);
    if (new_comp)
        return new_comp;

    return CreateComponent(type_name, change, syncEnabled);
}

ComponentPtr Entity::GetOrCreateComponent(const QString &type_name, const QString &name, AttributeChange::Type change, bool syncEnabled)
{
    ComponentPtr new_comp = GetComponent(type_name, name);
    if (new_comp)
        return new_comp;

    return CreateComponent(type_name, name, change, syncEnabled);
}

ComponentPtr Entity::GetOrCreateComponent(u32 typeId, AttributeChange::Type change)
{
    ComponentPtr new_comp = GetComponent(typeId);
    if (new_comp)
        return new_comp;

    return CreateComponent(typeId, change);
}

ComponentPtr Entity::GetOrCreateComponent(u32 typeId, const QString &name, AttributeChange::Type change)
{
    ComponentPtr new_comp = GetComponent(typeId, name);
    if (new_comp)
        return new_comp;

    return CreateComponent(typeId, name, change);
}

ComponentPtr Entity::CreateComponent(const QString &type_name, AttributeChange::Type change, bool syncEnabled)
{
    ComponentPtr new_comp = framework_->Scene()->CreateComponentByName(scene_, type_name);
    if (!new_comp)
    {
        LogError("Failed to create a component of type \"" + type_name + "\" to " + ToString());
        return ComponentPtr();
    }

    new_comp->SetNetworkSyncEnabled(syncEnabled);
    AddComponent(new_comp, change);
    return new_comp;
}

ComponentPtr Entity::CreateComponent(const QString &type_name, const QString &name, AttributeChange::Type change, bool syncEnabled)
{
    ComponentPtr new_comp = framework_->Scene()->CreateComponentByName(scene_, type_name, name);
    if (!new_comp)
    {
        LogError("Failed to create a component of type \"" + type_name + "\" and name \"" + name + "\" to " + ToString());
        return ComponentPtr();
    }

    new_comp->SetNetworkSyncEnabled(syncEnabled);
    AddComponent(new_comp, change);
    return new_comp;
}

ComponentPtr Entity::CreateComponent(u32 typeId, AttributeChange::Type change)
{
    ComponentPtr new_comp = framework_->Scene()->CreateComponentById(scene_, typeId);
    if (!new_comp)
    {
        LogError("Failed to create a component of type id " + QString::number(typeId) + " to " + ToString());
        return ComponentPtr();
    }

    AddComponent(new_comp, change);
    return new_comp;
}

ComponentPtr Entity::CreateComponent(u32 typeId, const QString &name, AttributeChange::Type change)
{
    ComponentPtr new_comp = framework_->Scene()->CreateComponentById(scene_, typeId, name);
    if (!new_comp)
    {
        LogError("Failed to create a component of type id " + QString::number(typeId) + " and name \"" + name + "\" to " + ToString());
        return ComponentPtr();
    }

    AddComponent(new_comp, change);
    return new_comp;
}


ComponentPtr Entity::GetComponent(const QString &type_name) const
{
    for(size_t i=0 ; i<components_.size() ; ++i)
        if (components_[i]->TypeName() == type_name)
            return components_[i];

    return ComponentPtr();
}

ComponentPtr Entity::GetComponent(u32 typeId) const
{
    for(size_t i = 0;  i <components_.size(); ++i)
        if (components_[i]->TypeId() == typeId)
            return components_[i];

    return ComponentPtr();
}

Entity::ComponentVector Entity::GetComponents(const QString &type_name) const
{
    ComponentVector ret;
    for(size_t i = 0; i < components_.size() ; ++i)
        if (components_[i]->TypeName() == type_name)
            ret.push_back(components_[i]);
    return ret;
}

ComponentPtr Entity::GetComponent(const QString &type_name, const QString& name) const
{
    for(size_t i=0 ; i<components_.size() ; ++i)
        if (components_[i]->TypeName() == type_name && components_[i]->Name() == name)
            return components_[i];

    return ComponentPtr();
}

ComponentPtr Entity::GetComponent(u32 typeId, const QString& name) const
{
    for(size_t i = 0; i < components_.size(); ++i)
        if (components_[i]->TypeId() == typeId && components_[i]->Name() == name)
            return components_[i];

    return ComponentPtr();
}

QObjectList Entity::GetComponentsRaw(const QString &type_name) const
{
    QObjectList ret;
    if (type_name.isNull())
        for(size_t i = 0; i < components_.size() ; ++i)
            ret.push_back(components_[i].get());
    else
        for(size_t i = 0; i < components_.size() ; ++i)
            if (components_[i]->TypeName() == type_name)
                ret.push_back(components_[i].get());
    return ret;
}

IAttribute *Entity::GetAttribute(const std::string &name) const
{
    for(size_t i = 0; i < components_.size() ; ++i)
    {
        IAttribute *attr = components_[i]->GetAttribute(QString(name.c_str()));
        if (attr)
            return attr;
    }
    return 0;
}

void Entity::SerializeToBinary(kNet::DataSerializer &dst) const
{
    dst.Add<u32>(GetId());
    uint num_serializable = 0;
    foreach(const ComponentPtr &comp, Components())
        if (!comp->IsTemporary())
            num_serializable++;
    dst.Add<u32>(num_serializable);
    foreach(const ComponentPtr &comp, Components())
        if (!comp->IsTemporary())
        {
            dst.Add<u32>(comp->TypeId()); ///\todo VLE this!
            dst.AddString(comp->Name().toStdString());
            dst.Add<u8>(comp->GetNetworkSyncEnabled() ? 1 : 0);
            
            // Write each component to a separate buffer, then write out its size first, so we can skip unknown components
            QByteArray comp_bytes;
            // Assume 64KB max per component for now
            comp_bytes.resize(64 * 1024);
            kNet::DataSerializer comp_dest(comp_bytes.data(), comp_bytes.size());
            comp->SerializeToBinary(comp_dest);
            comp_bytes.resize(comp_dest.BytesFilled());
            
            dst.Add<u32>(comp_bytes.size());
            dst.AddArray<u8>((const u8*)comp_bytes.data(), comp_bytes.size());
        }
}

/* Disabled for now, since have to decide how entityID conflicts are handled.
void Entity::DeserializeFromBinary(kNet::DataDeserializer &src, AttributeChange::Type change)
{
}*/

void Entity::SerializeToXML(QDomDocument &doc, QDomElement &base_element) const
{
    QDomElement entity_elem = doc.createElement("entity");
    
    QString id_str;
    id_str.setNum((int)GetId());
    entity_elem.setAttribute("id", id_str);

    foreach(const ComponentPtr c, Components())
        if (!c->IsTemporary())
            c->SerializeTo(doc, entity_elem);

    base_element.appendChild(entity_elem);
}

/* Disabled for now, since have to decide how entityID conflicts are handled.
void Entity::DeserializeFromXML(QDomElement& element, AttributeChange::Type change)
{
}*/

QString Entity::SerializeToXMLString() const
{
    QDomDocument scene_doc("Scene");
    QDomElement scene_elem = scene_doc.createElement("scene");

    SerializeToXML(scene_doc, scene_elem);
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

AttributeVector Entity::GetAttributes(const std::string &name) const
{
    std::vector<IAttribute *> ret;
    for(size_t i = 0; i < components_.size() ; ++i)
    {
        IAttribute *attr = components_[i]->GetAttribute(QString(name.c_str()));
        if (attr)
            ret.push_back(attr);
    }
    return ret;
}

EntityPtr Entity::Clone(bool local, bool temporary) const
{
    // Craft XML
    QDomDocument doc("Scene");
    QDomElement sceneElem = doc.createElement("scene");
    QDomElement entityElem = doc.createElement("entity");
    entityElem.setAttribute("id", QString::number((int) local ? scene_->GetNextFreeIdLocal() : scene_->GetNextFreeId()));
    foreach(const ComponentPtr &c, Components())
        c->SerializeTo(doc, entityElem);
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

QString Entity::GetName() const
{
    boost::shared_ptr<EC_Name> name = GetComponent<EC_Name>();
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

QString Entity::GetDescription() const
{
    boost::shared_ptr<EC_Name> name = GetComponent<EC_Name>();
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

void Entity::Exec(EntityAction::ExecutionTypeField type, const QString &action, const QString &p1, const QString &p2, const QString &p3)
{
    Exec(type, action, QStringList(QStringList() << p1 << p2 << p3));
}

void Entity::Exec(EntityAction::ExecutionTypeField type, const QString &action, const QStringList &params)
{
    EntityAction *act = Action(action);
    // Commented out to allow sending actions with receiver only on the server, for example
/*
    if (!HasReceivers(act))
        return;
*/

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

    if (GetScene())
        GetScene()->EmitActionTriggered(this, action, params, type);
}

void Entity::Exec(EntityAction::ExecutionTypeField type, const QString &action, const QVariantList &params)
{
    QStringList stringParams;
    foreach(QVariant var, params)
        stringParams << var.toString();
    Exec(type, action, stringParams);
}

bool Entity::HasReceivers(EntityAction *action)
{
    int receivers = action->receivers(SIGNAL(Triggered(QString, QString, QString, QStringList)));
    if (receivers == 0)
    {
//            LogInfo("No receivers found for action \"" + action->Name().toStdString() + "\" removing the action.");
        actions_.remove(action->Name());
        SAFE_DELETE(action);
        return false;
    }

    return true;
}

void Entity::EmitEntityRemoved(AttributeChange::Type change)
{
    if (change == AttributeChange::Disconnected)
        return;
    emit EntityRemoved(this, change);
}

void Entity::EmitEnterView()
{
    emit EnterView();
}

void Entity::EmitLeaveView()
{
    emit LeaveView();
}

void Entity::SetTemporary(bool enable)
{
    temporary_ = enable;
}

QString Entity::ToString() const
{
    QString name = GetName();
    if (name.trimmed().isEmpty())
        return QString("Entity ID ") + QString::number(GetId());
    else
        return QString("Entity \"") + name + "\" (ID: " + QString::number(GetId()) + ")";
}
