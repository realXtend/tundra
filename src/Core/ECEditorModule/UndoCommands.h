/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   UndoCommands.h
    @brief  A collection of classes which apply to the operations in EC Editor and Scene Structure windows. */

#pragma once

#include "ECEditorModuleApi.h"
#include "SceneFwd.h"
#include "Entity.h"
#include "IComponent.h"
#include "IAttribute.h"
#include "Color.h"
#include "Math/float3.h"
#include "Math/float3x4.h"

#include <QDomDocument>
#include <QDomElement>
#include <QUndoCommand>

typedef QList<entity_id_t> EntityIdList;
typedef QList<TransformAttributeWeakPtr> TransformAttributeWeakPtrList;

class EntityIdChangeTracker;

/// EditAttributeCommandBase exposes basi
class IEditAttributeCommand : public QUndoCommand
{
public:
    /// Internal QUndoCommand unique ID
    enum { Id = 100 };
   
    IEditAttributeCommand(QUndoCommand *parent) : QUndoCommand(parent) {}

    AttributeWeakPtr attribute;
    QString attributeName;
    QString attributeTypeName;
    entity_id_t parentId;
};

/// EditAttributeCommand representing an "Edit" operation to the attributes
template <typename T>
class EditAttributeCommand : public IEditAttributeCommand
{
public:
    /** Current value of the attribute will be read as the undo() applied value.
        When redo() is triggered this command does nothing and it will read the redo value once undo() is applied.
        @param attr The attribute that is being edited.
        @param parent The parent command of this command (optional). */
    EditAttributeCommand(IAttribute *attr, QUndoCommand *parent = 0);
    
    /** Current value of the attribute will be read as the undo() applied value.
        Given valueToApply will be applied immediately when redo() is triggered (eg. added to a QUndoStack).
        @param attr The attribute that is being edited.
        @param valueToApply The new attribute value. Gets applied immediately when this actions redo() it triggered.
        @param parent The parent command of this command (optional). */
    EditAttributeCommand(IAttribute *attr, const T &valueToApply, QUndoCommand *parent = 0);

    /// Returns this command's ID
    int id() const { return Id; }

    /// QUndoCommand override
    void undo()
    {
        if (!attribute.Expired())
        {
            redoValue = static_cast<Attribute<T> *>(attribute.Get())->Get();
            static_cast<Attribute<T> *>(attribute.Get())->Set(undoValue, AttributeChange::Default);
        }
    }

    /// QUndoCommand override
    void redo()
    {
        if (noAutoRedo)
            noAutoRedo = false;
        else if (!attribute.Expired())
            static_cast<Attribute<T> *>(attribute.Get())->Set(redoValue, AttributeChange::Default);
    }

    /// QUndoCommand override
    /** @todo Should we make a specialization for certain types of attributes that their 'Edit attribute' commands are merged,
            or should we keep each atomic change to the attributes in the stack?
            (e.g. if 'Transform' attribute for each of the components of each pos, rot, and scale float3s are edited, it 
            will push 9 commands into the undo stack) */
    bool mergeWith(const QUndoCommand * UNUSED_PARAM(other))
    {
        // Don't merge commands yet. This is only for the 'Color' attribute specialization
        return false;

        /*
        if (id() != other->id())
            return false;

        const EditAttributeCommand<T> *otherCommand = dynamic_cast<const EditAttributeCommand<T> *>(other);
        if (!otherCommand)
            return false;

        return true;
        */
    }

private:
    void Initialize(IAttribute *attr, bool autoRedo)
    {
        attribute = AttributeWeakPtr(attr->Owner()->shared_from_this(), attr);
        attributeName = attr->Name();
        attributeTypeName = attr->TypeName(),
        parentId = attr->Owner()->ParentEntity()->Id();
        noAutoRedo = false;
        setText("Edited " + attr->Name());
    }
    
    T redoValue;
    T undoValue;
    bool noAutoRedo;
};

#include "UndoCommands.inl"

/// Represents adding an attribute to a dynamic component
class ECEDITOR_MODULE_API AddAttributeCommand : public QUndoCommand
{
public:
    /// Internal QUndoCommand unique ID
    enum { Id = 101 };

    /// Constructor
    /* @param comp A raw pointer to the component that an attribute is added to
       @param typeName The type of the attribute being added
       @param name The name of the attribute being added
       @param parent The parent command of this command (optional) */
    AddAttributeCommand(IComponent * comp, const QString &typeName, const QString &name, QUndoCommand * parent = 0);

    /// Returns this command's ID
    int id() const;
    /// QUndoCommand override
    void undo();
    /// QUndoCommand override
    void redo();

    EntityWeakPtr entity_; ///< A weak pointer to this attribute's parent entity
    const QString componentName_; ///< Name of this attribute's parent component
    const QString componentType_; ///< Typename of this attribute's parent component
    const QString attributeTypeName_; ///< Typename of this attribute
    const QString attributeName_; ///< Name of this attribute
};

/// Represents removing an existing attribute of a dynamic component
class ECEDITOR_MODULE_API RemoveAttributeCommand : public QUndoCommand
{
public:
    /// Internal QUndoCommand unique ID
    enum { Id = 102 };

    /// Constructor
    /* @param attr A raw pointer to the attribute being removed
       @param parent The parent command of this command (optional) */
    RemoveAttributeCommand(IAttribute * attr, QUndoCommand * parent = 0);
    
    /// Returns this command's ID
    int id () const;
    /// QUndoCommand override
    void undo();
    /// QUndoCommand override
    void redo();

    EntityWeakPtr entity_; ///< A weak pointer to this attribute's parent entity
    const QString componentName_; ///< Name of this attribute's parent component
    const QString componentType_; ///< Typename of this attribute's parent component
    const QString attributeTypeName_; ///< Typename of this attribute
    const QString attributeName_; ///< Name of this attribute
    QString value_; ///< Value of this attribute represented as string
};

/// Represents adding a component to an entity or more entities
class ECEDITOR_MODULE_API AddComponentCommand : public QUndoCommand
{
public:
    /// Internal QUndoCommand unique ID
    enum { Id = 103 };

    /// Constructor
    /* @param scene Scene of which entities we're tracking.
       @param tracker Pointer to the EntityIdChangeTracker object
       @param entities A list of IDs of entities that a component is being added to
       @param compType Type name of the component being added
       @param compName Name of the component being added
       @param sync Sync state of the component being added
       @param temp Temporary state of the component being added
       @param parent The parent command of this command (optional) */
    AddComponentCommand(const ScenePtr &scene, EntityIdChangeTracker * tracker, const EntityIdList &entities,
        const QString &compType, const QString &compName, bool sync, bool temp, QUndoCommand * parent = 0);

    /// Returns this command's ID
    int id () const;
    /// QUndoCommand override
    void undo();
    /// QUndoCommand override
    void redo();

    SceneWeakPtr scene_; ///< A weak pointer to the main camera scene
    EntityIdChangeTracker * tracker_; ///< Pointer to the tracker object, taken from an undo manager
    EntityIdList entityIds_; ///< List of IDs of entities that this component is being added to
    QString componentName_; ///< Name of the component being added
    QString componentType_; ///< Typename of the component being added
    bool sync_; ///< Sync state of the component
    bool temp_; ///< Temporary state of the component
};

/// Represents editing entities and/or components as XML
class ECEDITOR_MODULE_API EditXMLCommand : public QUndoCommand
{
public:
    /// Internal QUndoCommand unique ID
    enum { Id = 104 };

    EditXMLCommand(const ScenePtr &scene, const QDomDocument &oldDoc, const QDomDocument &newDoc, QUndoCommand * parent = 0);

    /// Returns this command's ID
    int id () const;
    /// QUndoCommand override
    void undo();
    /// QUndoCommand override
    void redo();
    /// QUndoCommand override
    bool mergeWith(const QUndoCommand *other);

    /// Deserializes a document state
    /* @param docState The document state to be deserialized */
    void Deserialize(const QDomDocument docState);

    SceneWeakPtr scene_; ///< A weak pointer to the main camera scene
    QDomDocument oldState_; ///< Old state of the XML document
    QDomDocument newState_; ///< New state of the XML document
};

/// Represents adding an entity to the scene
class ECEDITOR_MODULE_API AddEntityCommand : public QUndoCommand
{
public:
    /// Internal QUndoCommand unique ID
    enum { Id = 105 };

    /// Constructor
    /* @param scene Scene of which entities we're tracking.
       @param tracker Pointer to the EntityIdChangeTracker object
       @param name The desired name of the entity being created
       @param sync The desired sync state of the entity being created
       @param temp The desired temporary state of the entity being created
       @param parent The parent command of this command (optional) */
    AddEntityCommand(const ScenePtr &scene, EntityIdChangeTracker * tracker, const QString &name, bool sync, bool temp, QUndoCommand * parent = 0);

    /// Returns this command's ID
    int id () const;
    /// QUndoCommand override
    void undo();
    /// QUndoCommand override
    void redo();

    SceneWeakPtr scene_; ///< A weak pointer to the main camera scene
    EntityIdChangeTracker * tracker_; ///< Pointer to the tracker object, taken from an undo manager
    QString entityName_; ///< Name of the entity being created
    entity_id_t entityId_; ///< ID of the entity
    bool sync_; ///< sync state of the entity
    bool temp_; ///< Temporary state of the entity
};

class ECEDITOR_MODULE_API RemoveCommand : public QUndoCommand
{
public:
    /// Internal QUndoCommand unique ID
    enum { Id = 106 };

    /// Constructor for both entity and component removal.
    /* @param scene Scene of which entities we're tracking.
       @param tracker Pointer to the EntityIdChangeTracker object.
       @param entities List of the entities about to be removed.
       @param components List of component about to be removed.
       @param parent The parent command of this command (optional). */
    RemoveCommand(const ScenePtr &scene, EntityIdChangeTracker * tracker, const QList<EntityWeakPtr> &entities, const QList<ComponentWeakPtr> &components, QUndoCommand * parent = 0);
    /// Constructor for removal of entities.
    RemoveCommand(const ScenePtr &scene, EntityIdChangeTracker *tracker, const QList<EntityWeakPtr> &entities, QUndoCommand *parent = 0);
    /// Constructor for removal of components.
    RemoveCommand(const ScenePtr &scene, EntityIdChangeTracker *tracker, const QList<ComponentWeakPtr> &components, QUndoCommand *parent = 0);
    /// Constructor for a single entity removal.
    RemoveCommand(const ScenePtr &scene, EntityIdChangeTracker *tracker, const EntityWeakPtr &entity, QUndoCommand *parent = 0);
    /// Constructor for a single component removal.
    RemoveCommand(const ScenePtr &scene, EntityIdChangeTracker *tracker, const ComponentWeakPtr &component, QUndoCommand *parent = 0);

    /// @todo Remove command for Scene.

    /// Returns this command's ID
    int id () const;
    /// QUndoCommand override
    void undo();
    /// QUndoCommand override
    void redo();

private:
    void Initialize(const QList<EntityWeakPtr> &entities, const QList<ComponentWeakPtr> &components);

    EntityIdList entityList_; ///< Entity ID list of the entities being removed
    typedef QList<QPair<QString, QString> > ComponentList; ///< A typedef for QList containing QPair of component typenames and component names
    typedef QMap<entity_id_t, ComponentList> ParentEntityOfComponentMap; ///< A typedef for QMap with entity ID as key and a ComponentList as value
    ParentEntityOfComponentMap componentMap_; ///< Map that links QPairs of component typenames and names with their parent entity IDs

    SceneWeakPtr scene_; ///< A weak pointer to the main camera scene
    EntityIdChangeTracker * tracker_; ///< Pointer to the tracker object, taken from an undo manager
    QDomDocument entitiesDocument_; ///< XML document containing data about the entities to be removed
    QDomDocument componentsDocument_; ///< XML document containing data about the components to be removed
};

/// Represents a rename operation over an entity or a component
class ECEDITOR_MODULE_API RenameCommand : public QUndoCommand
{
public:
    /// Internal QUndoCommand unique ID
    enum { Id = 107 };

    /// Constructor 
    /* @param entity A weak pointer to the entity being renamed
       @param tracker Pointer to the EntityIdChangeTracker object
       @param oldName The old name of the entity
       @param newName The new name of the entity
       @param parent The parent command of this command (optional) */
    RenameCommand(EntityWeakPtr entity, EntityIdChangeTracker * tracker, const QString oldName, const QString newName, QUndoCommand * parent = 0);

    /// Returns this command's ID
    int id () const;
    /// QUndoCommand override
    void undo();
    /// QUndoCommand override
    void redo();

    SceneWeakPtr scene_; ///< A weak pointer to the main camera scene
    EntityIdChangeTracker * tracker_; ///< Pointer to the tracker object, taken from an undo manager
    entity_id_t entityId_; ///< Entity ID of the entity being renamed
    QString oldName_; ///< Old entity name
    QString newName_; ///< New entity name
};

/// Represents toggling the temporary state of entity / entities
class ECEDITOR_MODULE_API ToggleTemporaryCommand : public QUndoCommand
{
public:
    /// Internal QUndoCommand unique ID
    enum { Id = 108 };

    /// Constructor
    /* @param entities A list of weak pointers to entities being toggled
       @param tracker Pointer to the EntityIdChangeTracker object
       @param temporary The desired temporary state to be set
       @param parent The parent command of this command (optional) */
    ToggleTemporaryCommand(const QList<EntityWeakPtr> &entities, EntityIdChangeTracker * tracker, bool temporary, QUndoCommand * parent = 0);

    /// Returns this command's ID
    int id () const;
    /// QUndoCommand override
    void undo();
    /// QUndoCommand override
    void redo();
    void ToggleTemporary(bool temporary);

    SceneWeakPtr scene_; ///< A weak pointer to the main camera scene
    EntityIdChangeTracker * tracker_; ///< Pointer to the tracker object, taken from an undo manager
    EntityIdList entityIds_; ///< List of target entity IDs
    bool temporary_; ///< Temporary state
};

class ECEDITOR_MODULE_API TransformCommand : public QUndoCommand
{
public:
    /// Internal QUnodCommand unique ID
    enum { Id = 109 };

    enum Action
    {
        Translate = 0,  // Translate on multiple axes
        TranslateX,     // Translate X-axis
        TranslateY,     // Translate Y-axis
        TranslateZ,     // Translate Z-axis
        Rotate,         // Rotate on multiple axes
        RotateX,        // Rotate X-axis
        RotateY,        // Rotate Y-axis
        RotateZ,        // Rotate Z-axis
        Scale,          // Scale on multiple axes
        ScaleX,         // Scale X-axis
        ScaleY,         // Scale Y-axis
        ScaleZ          // Scale Z-axis
    };

    TransformCommand(const TransformAttributeWeakPtrList &attributes, int numberOfItems, Action action, const float3 &offset, QUndoCommand *parent = 0);
    TransformCommand(const TransformAttributeWeakPtrList &attributes, int numberOfItems, Action action, const float3x4 &rotation, QUndoCommand *parent = 0);

    /// Returns this command's ID
    int id() const;
    /// QUndoCommand override
    void undo();
    /// QUndoCommand override
    void redo();
    /// QUndoCommand override
    bool mergeWith(const QUndoCommand *other);

    void SetCommandText();

    void DoTranslate(bool isUndo);
    void DoRotate(bool isUndo);
    void DoScale(bool isUndo);

    TransformAttributeWeakPtrList targets_;
    Action action_;
    float3 offset_;
    float3x4 rotation_;
    int nItems_;
};

/*
class ECEDITOR_MODULE_API PasteCommand : public QUndoCommand
{
public:
    /// Internal QUndoCommand unique ID
    enum { Id = 109 };

    PasteCommand(QUndoCommand * parent = 0);

    /// Returns this command's ID
    int id () const;
    /// QUndoCommand override
    void undo();
    /// QUndoCommand override
    void redo();
};
*/
