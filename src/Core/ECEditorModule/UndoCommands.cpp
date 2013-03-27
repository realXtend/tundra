// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"

#include "UndoCommands.h"
#include "EntityIdChangeTracker.h"

#include "Scene.h"
#include "SceneAPI.h"
#include "IComponent.h"
#include "IAttribute.h"
#include "UiAPI.h"
#include "UiMainWindow.h"
#include "Entity.h"
#include "EC_DynamicComponent.h"
#include "Transform.h"
#include "EC_Placeable.h"

#include "MemoryLeakCheck.h"

/// Merges two EditAttributeCommand<Color> objects, since editing 'Color' triggers two changes
template<> bool EditAttributeCommand<Color>::mergeWith(const QUndoCommand *other)
{
    if (id() != other->id())
        return false;

    const EditAttributeCommand<Color> *otherCommand = dynamic_cast<const EditAttributeCommand<Color> *>(other);
    if (!otherCommand)
        return false;

    return (oldValue_ == otherCommand->oldValue_);
}

AddAttributeCommand::AddAttributeCommand(IComponent * comp, const QString &typeName, const QString &name, QUndoCommand * parent) :
    entity_(comp->ParentEntity()->shared_from_this()),
    componentName_(comp->Name()),
    componentType_(comp->TypeName()),
    attributeTypeName_(typeName),
    attributeName_(name),
    QUndoCommand(parent)
{
    setText("add attribute of type " + typeName);
}

int AddAttributeCommand::id() const
{
    return Id;
}

void AddAttributeCommand::undo()
{
    EntityPtr ent = entity_.lock();
    if (!ent.get())
        return;

    ComponentPtr comp = ent->GetComponent(componentType_, componentName_);
    if (comp.get())
    {
        EC_DynamicComponent *dynComp = dynamic_cast<EC_DynamicComponent *>(comp.get());
        if (dynComp)
        {
            if (dynComp->ContainsAttribute(attributeName_))
            {
                dynComp->RemoveAttribute(attributeName_);
                dynComp->ComponentChanged(AttributeChange::Default);
            }
        }
    }
}

void AddAttributeCommand::redo()
{
    EntityPtr ent = entity_.lock();
    if (!ent.get())
        return;

    ComponentPtr comp = ent->GetComponent(componentType_, componentName_);
    if (comp.get())
    {
        EC_DynamicComponent *dynComp = dynamic_cast<EC_DynamicComponent *>(comp.get());
        if (dynComp)
            if (!dynComp->ContainsAttribute(attributeName_))
            {
                IAttribute * attr = dynComp->CreateAttribute(attributeTypeName_, attributeName_);
                if (attr)
                    dynComp->ComponentChanged(AttributeChange::Default);
                else
                    QMessageBox::information(dynComp->GetFramework()->Ui()->MainWindow(), QMessageBox::tr("Failed to create attribute"),
                    QMessageBox::tr("Failed to create %1 attribute \"%2\", please try again.").arg(attributeTypeName_).arg(attributeName_));
            }
    }
}


RemoveAttributeCommand::RemoveAttributeCommand(IAttribute *attr, QUndoCommand *parent) : 
    entity_(attr->Owner()->ParentEntity()->shared_from_this()),
    componentName_(attr->Owner()->Name()),
    componentType_(attr->Owner()->TypeName()),
    attributeTypeName_(attr->TypeName()),
    attributeName_(attr->Name()),
    value_(QString::fromStdString(attr->ToString())),
    QUndoCommand(parent)
{
    setText("remove attribute of type " + attributeTypeName_);
}

int RemoveAttributeCommand::id() const
{
    return Id;
}

void RemoveAttributeCommand::undo()
{
    EntityPtr ent = entity_.lock();
    if (!ent.get())
        return;

    ComponentPtr comp = ent->GetComponent(componentType_, componentName_);
    if (comp.get())
    {
        EC_DynamicComponent *dynComp = dynamic_cast<EC_DynamicComponent *>(comp.get());
        if (dynComp)
        {
            IAttribute * attr = dynComp->CreateAttribute(attributeTypeName_, attributeName_);
            attr->FromString(value_.toStdString(), AttributeChange::Default);
        }
    }
}

void RemoveAttributeCommand::redo()
{
    EntityPtr ent = entity_.lock();
    if (!ent.get())
        return;

    ComponentPtr comp = ent->GetComponent(componentType_, componentName_);
    if (comp.get())
    {
        EC_DynamicComponent *dynComp = dynamic_cast<EC_DynamicComponent *>(comp.get());
        if (dynComp)
        {
            dynComp->RemoveAttribute(attributeName_);
            dynComp->ComponentChanged(AttributeChange::Default);
        }
    }
}

AddComponentCommand::AddComponentCommand(const ScenePtr &scene, EntityIdChangeTracker * tracker, EntityIdList entities, const QString compType, const QString compName, bool sync, bool temp, QUndoCommand * parent) :
    scene_(scene),
    tracker_(tracker),
    entityIds_(entities),
    componentName_(compName),
    componentType_(compType),
    temp_(temp),
    sync_(sync),
    QUndoCommand(parent)
{
    setText("add component of type " + componentType_ + (entities.size() == 1 ? "" : " to multiple entities"));
}

int AddComponentCommand::id() const
{
    return Id;
}

void AddComponentCommand::undo()
{
    ScenePtr scene = scene_.lock();
    if (!scene.get())
        return;

    foreach (entity_id_t id, entityIds_)
    {
        EntityPtr ent = scene->EntityById(tracker_->RetrieveId(id));
        if (ent.get())
        {
            ComponentPtr comp = ent->GetComponent(componentType_, componentName_);
            if (comp.get())
            {
                sync_ = comp->IsReplicated();
                temp_ = comp->IsTemporary();

                ent->RemoveComponent(comp, AttributeChange::Default);
            }
        }
    }
}

void AddComponentCommand::redo()
{
    ScenePtr scene = scene_.lock();
    if (!scene.get())
        return;

    foreach (entity_id_t id, entityIds_)
    {
        EntityPtr ent = scene->EntityById(tracker_->RetrieveId(id));
        if (ent.get())
        {
            Framework *fw = scene->GetFramework();
            ComponentPtr comp = fw->Scene()->CreateComponentByName(scene.get(), componentType_, componentName_);
            if (comp)
            {
                comp->SetReplicated(sync_);
                comp->SetTemporary(temp_);
                ent->AddComponent(comp, AttributeChange::Default);
            }
        }
    }
}

EditXMLCommand::EditXMLCommand(const ScenePtr &scene, const QDomDocument oldDoc, const QDomDocument newDoc, QUndoCommand * parent) : 
    scene_(scene),
    oldState_(oldDoc),
    newState_(newDoc),
    QUndoCommand(parent)
{
    setText("editing XML");
}

int EditXMLCommand::id() const
{
    return Id;
}

void EditXMLCommand::undo()
{
    Deserialize(oldState_);
}

void EditXMLCommand::redo()
{
    Deserialize(newState_);
}

bool EditXMLCommand::mergeWith(const QUndoCommand *other)
{
    if (id() != other->id())
        return false;

    const EditXMLCommand *otherCommand = dynamic_cast<const EditXMLCommand *>(other);
    if (!otherCommand)
        return false;

    if ((oldState_.toString() != otherCommand->oldState_.toString()) && (newState_.toString() != otherCommand->newState_.toString()))
        return false;

    return true;
}

void EditXMLCommand::Deserialize(const QDomDocument docState)
{
    ScenePtr scene = scene_.lock();
    if (!scene.get())
        return;

    QDomElement entityElement;
    QDomElement entitiesElement = docState.firstChildElement("entities");
    if (!entitiesElement.isNull())
        entityElement = entitiesElement.firstChildElement("entity");
    else
        entityElement = docState.firstChildElement("entity");

    while(!entityElement.isNull())
    {
        entity_id_t id = (entity_id_t)entityElement.attribute("id").toInt();

        EntityPtr entity = scene->GetEntity(id);
        if (entity)
        {
            QDomElement componentElement = entityElement.firstChildElement("component");
            while(!componentElement.isNull())
            {
                QString typeName = componentElement.attribute("type");
                QString name = componentElement.attribute("name");
                ComponentPtr comp = entity->GetComponent(typeName, name);
                if (comp)
                    comp->DeserializeFrom(componentElement, AttributeChange::Default);

                componentElement = componentElement.nextSiblingElement("component");
            }
        }
        else
        {
            LogWarning("EcXmlEditorWidget::Save: Could not find entity " + QString::number(id) + " in scene!");
        }

        entityElement = entityElement.nextSiblingElement("entity");
    }
}

AddEntityCommand::AddEntityCommand(const ScenePtr &scene, EntityIdChangeTracker * tracker, const QString &name, bool sync, bool temp, QUndoCommand *parent) :
    scene_(scene),
    tracker_(tracker),
    entityName_(name),
    entityId_(0),
    sync_(sync),
    temp_(temp),
    QUndoCommand(parent)
{
    setText("add entity named " + (entityName_.isEmpty() ? "(no name)" : entityName_));
}

int AddEntityCommand::id() const
{
    return Id;
}

void AddEntityCommand::undo()
{
    ScenePtr scene = scene_.lock();
    if (!scene.get())
        return;

    entity_id_t newId = tracker_->RetrieveId(entityId_);
    EntityPtr ent = scene->EntityById(newId);
    if (ent.get())
        scene->RemoveEntity(newId, AttributeChange::Replicate);
}

void AddEntityCommand::redo()
{
    ScenePtr scene = scene_.lock();
    if (!scene.get())
        return;

    entity_id_t newId = sync_ ? scene->NextFreeId() : scene->NextFreeIdLocal();
    tracker_->AppendUnackedId(newId);
    if (entityId_)
        tracker_->TrackId(entityId_, newId);

    entityId_ = newId;
    AttributeChange::Type changeType = sync_ ? AttributeChange::Replicate : AttributeChange::LocalOnly;
    EntityPtr entity = scene->CreateEntity(entityId_, QStringList(), changeType, sync_);

    if (!entityName_.isEmpty())
        entity->SetName(entityName_);

    entity->SetTemporary(temp_);
}

RemoveCommand::RemoveCommand(const ScenePtr &scene, EntityIdChangeTracker * tracker, const QList<EntityWeakPtr> &entityList, const QList<ComponentWeakPtr> &componentList, QUndoCommand * parent) :
    scene_(scene),
    tracker_(tracker),
    QUndoCommand(parent)
{
    QString commandText;
    QString entityStr;
    QString andStr;
    QString componentStr;

    if (!entityList.isEmpty())
    {
        for (QList<EntityWeakPtr>::const_iterator i = entityList.begin(); i != entityList.end(); ++i)
        {
            entityList_ << (*i).lock()->Id();

            if (entityStr.isEmpty())
            {
                QString entityName = (*i).lock()->Name().isEmpty() ? "(no name)" : (*i).lock()->Name();
                entityStr = QString(" entity named %1 with id %2 ").arg(entityName).arg((*i).lock()->Id());
            }
        }

        if (entityList.count() > 1)
            entityStr = QString(" multiple entities ");
    }

    if (!componentList.isEmpty())
    {
        for (QList<ComponentWeakPtr>::const_iterator i = componentList.begin(); i != componentList.end(); ++i)
        {
            ComponentPtr comp = (*i).lock();
            if (comp.get())
            {
                if (entityList_.contains(comp->ParentEntity()->Id()))
                    continue;

                componentMap_[comp->ParentEntity()->Id()] << qMakePair(comp->TypeName(), comp->Name());

                if (componentStr.isEmpty())
                {
                    QString componentName = comp->Name().isEmpty() ? "(no name)" : comp->Name();
                    componentStr = QString(" component named %1 of type %2 ").arg(componentName).arg(comp->TypeName());
                }
            }
        }

        if (componentList.count() > 1)
            componentStr = QString(" multiple components ");
    }

    andStr = QString((!entityStr.isEmpty() && !componentStr.isEmpty()) ? "and" : "");
    commandText = QString("remove%1%2%3").arg(entityStr).arg(andStr).arg(componentStr);
    setText(commandText);
}

int RemoveCommand::id() const
{
    return Id;
}

void RemoveCommand::undo()
{
    ScenePtr scene = scene_.lock();
    if (!scene.get())
        return;

    if (!entitiesDocument_.isNull())
    {
        QDomElement sceneElement = entitiesDocument_.firstChildElement("scene");
        QDomElement entityElement = sceneElement.firstChildElement("entity");
        while (!entityElement.isNull())
        {
            entity_id_t id = entityElement.attribute("id").toInt();
            bool sync = ParseBool(entityElement.attribute("sync"));

            entity_id_t newId = sync ? scene->NextFreeId() : scene->NextFreeIdLocal();
            tracker_->TrackId(id, newId);

            QString newIdStr;
            newIdStr.setNum((int)newId);
            entityElement.setAttribute("id", newIdStr);

            entityElement = entityElement.nextSiblingElement();
        }

        scene->CreateContentFromXml(entitiesDocument_, true, AttributeChange::Default);
    }

    if (!componentsDocument_.isNull())
    {
        QDomElement entityElement = componentsDocument_.firstChildElement("entity");
        while (!entityElement.isNull())
        {
            entity_id_t entityId = entityElement.attribute("id").toInt();
            EntityPtr ent = scene->EntityById(tracker_->RetrieveId(entityId));
            if (ent.get())
            {
                QDomElement compElement = entityElement.firstChildElement("component");
                while (!compElement.isNull())
                {
                    ComponentPtr component;
                    QString type = compElement.attribute("type");
                    QString name = compElement.attribute("name");
                    QString sync = compElement.attribute("sync");
                    QString temp = compElement.attribute("temporary");

                    Framework *fw = ent->ParentScene()->GetFramework();
                    component = fw->Scene()->CreateComponentByName(ent->ParentScene(), type, name);

                    if (!sync.isEmpty())
                        component->SetReplicated(ParseBool(sync));
                    if (!temp.isEmpty())
                        component->SetTemporary(ParseBool(temp));

                    ent->AddComponent(component);
                    component->DeserializeFrom(compElement, AttributeChange::Default);

                    compElement = compElement.nextSiblingElement("component");
                }
            }

            entityElement = entityElement.nextSiblingElement("entity");
        }
    }
}

void RemoveCommand::redo()
{
    ScenePtr scene = scene_.lock();
    if (!scene.get())
        return;

    if (!componentMap_.isEmpty())
    {
        componentsDocument_ = QDomDocument();
        EntityIdList keys = componentMap_.keys();
        foreach (entity_id_t key, keys)
        {
            EntityPtr ent = scene->EntityById(tracker_->RetrieveId(key));
            if (ent.get())
            {
                QDomElement entityElem = componentsDocument_.createElement("entity");
                componentsDocument_.appendChild(entityElem);
                entityElem.setAttribute("id", ent->Id());
                entityElem.setAttribute("name", ent->Name());

                for (ComponentList::iterator i = componentMap_[key].begin(); i != componentMap_[key].end(); ++i)
                {
                    ComponentPtr comp = ent->GetComponent((*i).first, (*i).second);
                    if (comp.get())
                    {
                        comp->SerializeTo(componentsDocument_, entityElem, true);
                        ent->RemoveComponent(comp, AttributeChange::Replicate);
                    }
                }
            }
        }
    }

    if (!entityList_.isEmpty())
    {
        entitiesDocument_ = QDomDocument();
        QDomElement sceneElement = entitiesDocument_.createElement("scene");
        entitiesDocument_.appendChild(sceneElement);

        foreach(entity_id_t id, entityList_)
        {
            EntityPtr ent = scene->EntityById(tracker_->RetrieveId(id));
            if (ent.get())
            {
                ent->SerializeToXML(entitiesDocument_, sceneElement, true);
                scene->RemoveEntity(ent->Id(), AttributeChange::Replicate);
            }
        }
    }
}

RenameCommand::RenameCommand(EntityWeakPtr entity, EntityIdChangeTracker * tracker, const QString oldName, const QString newName, QUndoCommand * parent) : 
    tracker_(tracker),
    oldName_(oldName),
    newName_(newName),
    QUndoCommand(parent)
{
    setText("rename entity named " + (oldName.isEmpty() ? "(no name)" : oldName));
    scene_ = entity.lock()->ParentScene()->shared_from_this();
    entityId_ = entity.lock()->Id();
}

int RenameCommand::id() const
{
    return Id;
}

void RenameCommand::undo()
{
    ScenePtr scene = scene_.lock();
    if (!scene.get())
        return;

    EntityPtr entity = scene->EntityById(tracker_->RetrieveId(entityId_));
    if (entity.get())
        entity->SetName(oldName_);
}

void RenameCommand::redo()
{
    ScenePtr scene = scene_.lock();
    if (!scene.get())
        return;

    EntityPtr entity = scene->EntityById(tracker_->RetrieveId(entityId_));
    if (entity.get())
        entity->SetName(newName_);
}

ToggleTemporaryCommand::ToggleTemporaryCommand(const QList<EntityWeakPtr> &entities, EntityIdChangeTracker * tracker, bool temporary, QUndoCommand *parent) :
    tracker_(tracker),
    temporary_(temporary),
    QUndoCommand(parent)
{
    if (entities.size() > 1)
        setText("toggle temporary of multiple entities");
    else
    {
        QString name = entities.at(0).lock()->Name();
        setText("toggle temporary of entity named " + name);
    }

    scene_ = entities.at(0).lock()->ParentScene()->shared_from_this();

    for(QList<EntityWeakPtr>::const_iterator i = entities.begin(); i != entities.end(); ++i)
        entityIds_ << (*i).lock()->Id();
}

int ToggleTemporaryCommand::id() const
{
    return Id;
}

void ToggleTemporaryCommand::undo()
{
    ToggleTemporary(!temporary_);
}

void ToggleTemporaryCommand::redo()
{
    ToggleTemporary(temporary_);
}

void ToggleTemporaryCommand::ToggleTemporary(bool temporary)
{
    ScenePtr scene = scene_.lock();
    if (!scene.get())
        return;

    foreach(entity_id_t id, entityIds_)
    {
        EntityPtr entity = scene->EntityById(tracker_->RetrieveId(id));
        if (entity.get())
            entity->SetTemporary(temporary);
    }
}

TransformCommand::TransformCommand(const TransformAttributeWeakPtrList &attributes, int numberOfItems, Action action, const float3 & offset, QUndoCommand * parent) : 
    targets_(attributes),
    nItems_(numberOfItems),
    action_(action),
    offset_(offset),
    rotation_(float3x4::identity),
    QUndoCommand(parent)
{
    SetCommandText();
}

TransformCommand::TransformCommand(const TransformAttributeWeakPtrList &attributes, int numberOfItems, const float3x4 & rotation, QUndoCommand * parent) :
    targets_(attributes),
    nItems_(numberOfItems),
    action_(TransformCommand::Rotate),
    rotation_(rotation),
    offset_(float3::zero),
    QUndoCommand(parent)
{
    SetCommandText();
}

int TransformCommand::id() const
{
    return Id;
}

void TransformCommand::SetCommandText()
{
    QString text;
    switch(action_)
    {
        case Translate:
            text += "translate";
            break;
        case Rotate:
            text += "rotate";
            break;
        case Scale:
            text += "scale";
            break;
    }
    text += QString(" %1 ").arg(nItems_);
    text += (nItems_ == 1 ? "item" : "items");
    setText(text);
}

void TransformCommand::undo()
{
    switch(action_)
    {
        case Translate:
            DoTranslate(true);
            break;
        case Rotate:
            DoRotate(true);
            break;
        case Scale:
            DoScale(true);
            break;
    }
}

void TransformCommand::redo()
{
    switch(action_)
    {
        case Translate:
            DoTranslate(false);
            break;
        case Rotate:
            DoRotate(false);
            break;
        case Scale:
            DoScale(false);
            break;
    }
}

void TransformCommand::DoTranslate(bool isUndo)
{
    float3 offset = (isUndo ? offset = offset_.Neg() : offset = offset_);

    foreach(const TransformAttributeWeakPtr &attr, targets_)
    {
        Attribute<Transform> *transform = dynamic_cast<Attribute<Transform> *>(attr.Get());
        if (transform)
        {
            Transform t = transform->Get();
            // If we have parented transform, translate the changes to parent's world space.
            Entity *parentPlaceableEntity = attr.parentPlaceableEntity.lock().get();
            EC_Placeable *parentPlaceable = parentPlaceableEntity ? parentPlaceableEntity->GetComponent<EC_Placeable>().get() : 0;
            if (parentPlaceable)
                t.pos += parentPlaceable->WorldToLocal().MulDir(offset);
            else
                t.pos += offset;
            transform->Set(t, AttributeChange::Default);
        }
    }
}

void TransformCommand::DoRotate(bool isUndo)
{
    float3x4 rotation = (isUndo ? rotation_.Inverted() : rotation_);

    foreach(const TransformAttributeWeakPtr &attr, targets_)
    {
        Attribute<Transform> *transform = dynamic_cast<Attribute<Transform> *>(attr.Get());
        if (transform)
        {
            Transform t = transform->Get();
            // If we have parented transform, translate the changes to parent's world space.
            Entity *parentPlaceableEntity = attr.parentPlaceableEntity.lock().get();
            EC_Placeable* parentPlaceable = parentPlaceableEntity ? parentPlaceableEntity->GetComponent<EC_Placeable>().get() : 0;
            if (parentPlaceable)
                t.FromFloat3x4(parentPlaceable->WorldToLocal() * rotation * parentPlaceable->LocalToWorld() * t.ToFloat3x4());
            else
                t.FromFloat3x4(rotation * t.ToFloat3x4());

            transform->Set(t, AttributeChange::Default);
        }
    }
}

void TransformCommand::DoScale(bool isUndo)
{
    float3 offset = (isUndo ? offset = offset_.Neg() : offset = offset_);

    foreach(const TransformAttributeWeakPtr &attr, targets_)
    {
        Attribute<Transform> *transform = dynamic_cast<Attribute<Transform> *>(attr.Get());
        if (transform)
        {
            Transform t = transform->Get();
            t.scale += offset;
            transform->Set(t, AttributeChange::Default);
        }
    }
}

bool TransformCommand::mergeWith(const QUndoCommand * other)
{
    if (id() != other->id())
        return false;

    const TransformCommand * otherCommand = dynamic_cast<const TransformCommand*> (other);
    if (!otherCommand)
        return false;

    if (action_ != otherCommand->action_)
        return false;

    if (targets_ != otherCommand->targets_)
        return false;

    if (action_ != Rotate)
        offset_ += otherCommand->offset_;
    else
        rotation_ = otherCommand->rotation_.Mul(rotation_);

    return true;
}


/*
PasteCommand::PasteCommand(QUndoCommand *parent) :
    QUndoCommand(parent)
{
}

int PasteCommand::id() const
{
    return Id;
}

void PasteCommand::undo()
{
}

void PasteCommand::redo()
{
}
*/
