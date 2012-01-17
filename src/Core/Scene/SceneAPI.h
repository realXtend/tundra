// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "SceneFwd.h"
#include "CoreTypes.h"
#include "CoreStringUtils.h"

#include <QObject>

class Framework;
class SceneInteract;

/// Gives access to the scenes in the system.
/** With this API you can create, remove, query scenes and listen to scene additions and removals.
    Owned by Framework. */
class SceneAPI : public QObject
{
    Q_OBJECT

    friend class Framework;

public:
    /// Destructor.
    ~SceneAPI();

    /// Creates new component of the type @c T.
    /** @param newComponentName Name for the component (optional). */
    template<typename T>
    boost::shared_ptr<T> CreateComponent(Scene* parentScene, const QString &newComponentName = "") const
    {
        return boost::dynamic_pointer_cast<T>(CreateComponentById(parentScene, T::TypeIdStatic(), newComponentName));
    }

public slots:
    /// Get Scene Interact weak pointer.
    ///\todo Remove this - move to its own plugin - should not have hardcoded application logic running on each scene. -jj.
    SceneInteract *GetSceneInteract() const;

    /// Returns true if specified scene exists, false otherwise
    bool HasScene(const QString &name) const;

    /// Returns a pointer to a scene
    /** Manage the pointer carefully, as scenes may not get deleted properly if
        references to the pointer are left alive.

        @note Returns a shared pointer, but it is preferable to use a weak pointer, SceneWeakPtr,
              to avoid dangling references that prevent scenes from being properly destroyed.

        @param name Name of the scene to return
        @return The scene, or empty pointer if the scene with the specified name could not be found. */
    ScenePtr GetScene(const QString &name) const;

    /// Returns the Scene the current active main camera is in.
    /** If there is no active main camera, this function returns the first found scene.
        If no scenes have been created, returns 0. */
    Scene *MainCameraScene();

    /// Creates new empty scene.
    /** @param name name of the new scene
        @param viewEnabled Whether the scene is view enabled
        @param authority True for server & standalone scenes, false for network client scenes
        @return The new scene, or empty pointer if scene with the specified name already exists. */
    ScenePtr CreateScene(const QString &name, bool viewEnabled, bool authority);

    /// Removes a scene with the specified name.
    /** The scene may not get deleted since there may be dangling references to it.
        If the scene does get deleted, removes all entities which are not shared with
        another existing scene.

        Does nothing if scene with the specified name doesn't exist.

        @param name name of the scene to delete */
    void RemoveScene(const QString &name);

    /// Returns the scene map for self reflection / introspection.
    SceneMap &Scenes();
    const SceneMap &Scenes() const;

    /// Return if a component factory has been registered for a type name.
    bool IsComponentFactoryRegistered(const QString &typeName) const;

    /// Registers a new factory to create new components of type 'componentTypename' and id 'componentTypeid'.
    void RegisterComponentFactory(ComponentFactoryPtr factory);

    /// Creates a new component instance by specifying the typename of the new component to create, and the scene where to create.
    ComponentPtr CreateComponentByName(Scene* scene, const QString &componentTypename, const QString &newComponentName = "") const;

    /// Creates a new component instance by specifying the typeid of the new component to create, and the scene where to create.
    ComponentPtr CreateComponentById(Scene* scene, u32 componentTypeid, const QString &newComponentName = "") const;

    /// Looks up the given type id and returns the type name string for that id.
    QString GetComponentTypeName(u32 componentTypeid) const;

    /// Looks up the given type name and returns the type id for that component type.
    u32 GetComponentTypeId(const QString &componentTypename) const;

    /// Looks up the attribute type name for an attribute type id
    static QString GetAttributeTypeName(u32 attributeTypeid);

    /// Looks up the type id for an attribute type name, or zero if not found
    static u32 GetAttributeTypeId(const QString &attributeTypename);

    /// Creates a clone of the specified component. The new component will be detached, i.e. it has no parent entity.
    ///\todo Implement this.
//    ComponentPtr CloneComponent(const ComponentPtr &component, const QString &newComponentName);

    /// Create a new dynamic attribute without attaching it to a component and return it. Return null if illegal typeid.
    static IAttribute* CreateAttribute(u32 attributeTypeid, const QString& newAttributeName);

    /// Create a new dynamic attribute without attaching it to a component and return it. Return null if illegal type.
    static IAttribute* CreateAttribute(const QString &attributeTypename, const QString& newAttributeName);

    /// Returns a list of all attribute type names that can be used in the CreateAttribute function to create an attribute.
    static QStringList AttributeTypes();

    /// Returns a list of all component type names that can be used in the CreateComponentByName function to create a component.
    QStringList ComponentTypes() const;

signals:
    /// Emitted after new scene has been added to framework.
    /** @param name new scene name. */
    void SceneAdded(const QString &name);

    /// Emitted after scene has been removed from the framework.
    /** @param name removed scene name. */
    void SceneRemoved(const QString &name);

private:
    /// Constructor. Framework takes ownership of this object.
    /** @param owner Owner Framework. */
    explicit SceneAPI(Framework *owner);

    /// Frees all known scene and the scene interact object.
    /** Called by Framework during application shutdown. */
    void Reset();

    ComponentFactoryPtr GetFactory(const QString &typeName) const;
    ComponentFactoryPtr GetFactory(u32 typeId) const;

    typedef std::map<QString, ComponentFactoryPtr, QStringLessThanNoCase> ComponentFactoryMap;
    typedef std::map<u32, boost::weak_ptr<IComponentFactory> > ComponentFactoryWeakMap;

    ComponentFactoryMap componentFactories;
    ComponentFactoryWeakMap componentFactoriesByTypeid;
    Framework *framework;
    SceneMap scenes; ///< All currently created scenes.
    SceneInteract *sceneInteract; ///< Scene interact. \todo Remove this - move to its own plugin - should not have hardcoded application logic running on each scene. -jj.
    static QStringList attributeTypeNames;
};
