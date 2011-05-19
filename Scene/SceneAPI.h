// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Scene_SceneAPI_h
#define incl_Scene_SceneAPI_h

#include "SceneFwd.h"
#include "CoreTypes.h"
#include "CoreStringUtils.h"

#include <QObject>
#include <QString>

class Framework;
class SceneInteract;

/// Scene core API.
/**
<table class="header"><tr><td>
<h2>SceneAPI</h2>

Manages the scenes for the viewer and server. With this API you can create, remove, get and query scenes.
You can also get and set the default scene. You can also receive Qt signals about scene events from this API.

Owned by Framework.

<b>Qt signals emitted by SceneAPI object:</b>
<ul>
<li>SceneAdded(const QString&); - Emitted when a scene is added.
<div>Parameters: Added scenes name.</div>
<li>SceneRemoved(const QString&); - Emitted when a scene is removed.
<div>Parameters: Removed scenes name.</div>
<li>DefaultWorldSceneChanged(SceneManager*); - Emitted when a new default world scene is set.
<div>Parameters: The new default SceneManager ptr.</div>
</ul>

</td></tr></table>
*/
class SceneAPI : public QObject
{
    Q_OBJECT

    friend class Framework;

public:
    /// Destructor.
    ~SceneAPI();

    /// Creates new component of the type @c T.
    /** @param newComponentName Name for the component (optional).
    */
    template<typename T>
    boost::shared_ptr<T> CreateComponent(const QString &newComponentName = "")
    {
        return boost::dynamic_pointer_cast<T>(CreateComponentById(T::TypeIdStatic(), newComponentName));
    }

signals:
    /// Emitted after new scene has been added to framework.
    /// @param name new scene name.
    void SceneAdded(const QString &name);

    /// Emitted after scene has been removed from the framework.
    /// @param name removed scene name.
    void SceneRemoved(const QString &name);

    /// Emitted when default world scene changes.
    /// @param scene new default world scene object.
    ///\todo Delete this function and the concept of 'default scene' or 'current scene'. There should be neither. -jj.
    void DefaultWorldSceneChanged(SceneManager *scene);

public slots:
    /// Get Scene Interact weak pointer.
    ///\todo Remove this - move to its own plugin - should not have hardcoded application logic running on each scene. -jj.
    SceneInteract *GetSceneInteract() const;

    /// Returns true if specified scene exists, false otherwise
    bool HasScene(const QString &name) const;

    /// Sets the default world scene, for convinient retrieval with GetDefaultWorldScene().
    ///\todo Delete this function and the concept of 'default scene' or 'current scene'. There should be neither. -jj.
    void SetDefaultScene(const QString &name);

    /// Sets the default world scene, for convinient retrieval with GetDefaultWorldScene().
    ///\todo Delete this function and the concept of 'default scene' or 'current scene'. There should be neither. -jj.
    void SetDefaultScene(const ScenePtr &scene);
    
    /// Returns the default scene shared ptr.
    /// \todo remove this function when we move to QPointer/QSharedPointer/QWeakPointer<SceneManager> rename GetDefaultSceneRaw() to GetDefaultScene().
    ///\todo Delete this function and the concept of 'default scene' or 'current scene'. There should be neither. -jj.
    const ScenePtr &GetDefaultScene() const;

    /// Returns the default scene ptr.
    ///\todo Delete this function and the concept of 'default scene' or 'current scene'. There should be neither. -jj.
    SceneManager* GetDefaultSceneRaw() const;

    /// Returns a pointer to a scene
    /** Manage the pointer carefully, as scenes may not get deleted properly if
        references to the pointer are left alive.

        \note Returns a shared pointer, but it is preferable to use a weak pointer, SceneWeakPtr,
              to avoid dangling references that prevent scenes from being properly destroyed.

        @param name Name of the scene to return
        @return The scene, or empty pointer if the scene with the specified name could not be found 
    */
    /// \todo remove this function when we move to QPointer/QSharedPointer/QWeakPointer<SceneManager> rename GetSceneRaw() to GetScene().
    ScenePtr GetScene(const QString &name) const;

    /// Creates new empty scene.
    /** @param name name of the new scene
        @param viewEnabled Whether the scene is view enabled
        @return The new scene, or empty pointer if scene with the specified name already exists.
    */
    ScenePtr CreateScene(const QString &name, bool viewEnabled);

    /// Removes a scene with the specified name.
    /** The scene may not get deleted since there may be dangling references to it.
        If the scene does get deleted, removes all entities which are not shared with
        another existing scene.

        Does nothing if scene with the specified name doesn't exist.

        @param name name of the scene to delete
    */
    void RemoveScene(const QString &name);

    /// Returns the scene map for self reflection / introspection.
    const SceneMap &Scenes() const;

    /// Registers a new factory to create new components of type 'componentTypename' and id 'componentTypeid'.
    void RegisterComponentFactory(ComponentFactoryPtr factory);

    /// Creates a new component instance by specifying the typename of the new component to create.
    ComponentPtr CreateComponentByName(const QString &componentTypename, const QString &newComponentName = "");

    /// Creates a new component instance by specifying the typeid of the new component to create.
    ComponentPtr CreateComponentById(u32 componentTypeid, const QString &newComponentName = "");

    /// Looks up the given type id and returns the type name string for that id.
    QString GetComponentTypeName(u32 componentTypeid);

    /// Looks up the given type name and returns the type id for that component type.
    u32 GetComponentTypeId(const QString &componentTypename);

    /// Creates a clone of the specified component. The new component will be detached, i.e. it has no parent entity.
    ///\todo Implement this.
//    ComponentPtr CloneComponent(const ComponentPtr &component, const QString &newComponentName);

    /// Create new attribute for spesific component.
    IAttribute *CreateAttribute(IComponent *owner, const QString &attributeTypename, const QString &newAttributeName);

    /// Returns a list of all attribute typenames that can be used in the CreateAttribute function to create an attribute.
    QStringList GetAttributeTypes() const;

    /// Returns a list of all component typenames that can be used in the CreateComponentByName function to create a component.
    QStringList GetComponentTypes() const;

private:
    /// Constructor. Framework takes ownership of this object.
    /// @param framework Framework ptr.
    explicit SceneAPI(Framework *framework);

    /// Frees all known scene and the scene interact object.
    /// \note This function is called by our fried class Framework in its UnloadModules() function.
    void Reset();
    
    /// Initialize the scene interact object. Needs framework->Input() to be valid.
    /// \note This function is called by our fried class Framework when InputAPI is ready.
    void Initialise();

    ComponentFactoryPtr GetFactory(const QString &typeName);
    ComponentFactoryPtr GetFactory(u32 typeId);

    typedef boost::shared_ptr<IComponentFactory> ComponentFactoryPtr;
    typedef std::map<QString, ComponentFactoryPtr, QStringLessThanNoCase> ComponentFactoryMap;
    typedef std::map<u32, boost::weak_ptr<IComponentFactory> > ComponentFactoryWeakMap;

    ComponentFactoryMap componentFactories;
    ComponentFactoryWeakMap componentFactoriesByTypeid;
    Framework *framework_; ///< Framework.
    SceneMap scenes_; ///< All currently created scenes.
    ScenePtr defaultScene_; ///< Current 'default' scene. \todo Delete this.
    SceneInteract *sceneInteract; ///< Scene interact. \todo Remove this - move to its own plugin - should not have hardcoded application logic running on each scene. -jj.
};

#endif
