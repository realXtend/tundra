// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Scene_SceneAPI_h
#define incl_Scene_SceneAPI_h

#include "SceneFwd.h"
#include "SceneInteract.h"

#include <QObject>
#include <QString>

typedef std::map<QString, Scene::ScenePtr> SceneMap;
namespace Foundation { class Framework; }

/**
<table class="header"><tr><td>
<h2>SceneAPI</h2>

Manages the scenes for the viewer and server. With this API you can create, remove, get and query scenes.
You can also get and set the default scene. You can also receive Qt signals about scene events from this API.

Owned by Foundation::Framework.

<b>Qt signals emitted by SceneAPI object:</b>
<ul>
<li>SceneAdded(const QString&); - Emitted when a scene is added.
<div>Parameters: Added scenes name.</div>
<li>SceneRemoved(const QString&); - Emitted when a scene is removed.
<div>Parameters: Removed scenes name.</div>
<li>DefaultWorldSceneChanged(Scene::SceneManager*); - Emitted when a new default world scene is set.
<div>Parameters: The new default Scene::SceneManager ptr.</div>
</ul>

</td></tr></table>
*/

/*! 
    \todo Change typedef Scene::ScenePtr from boost::shared_ptr<Scene::SceneManager> to 
    QPointer/QSharedPointer/QWeakPointer<Scene::SceneManager> to get rid of *Raw() functions for scripts.
*/

class SceneAPI : public QObject
{
    Q_OBJECT

    friend class Foundation::Framework;

public:
    /// Destructor.
    ~SceneAPI();

signals:
    //! Emitted after new scene has been added to framework.
    /// \param name new scene name.
    void SceneAdded(const QString &name);

    //! Emitted after scene has been removed from the framework.
    /// \param name removed scene name.
    void SceneRemoved(const QString &name);

    /// Emitted when default world scene changes.
    /// \param scene new default world scene object.
    void DefaultWorldSceneChanged(Scene::SceneManager *scene);

public slots:
    //! Get Scene Interact weak pointer.
    SceneInteractWeakPtr GetSceneIteract() const;

    //! Returns true if specified scene exists, false otherwise
    bool HasScene(const QString &name) const;

    //! Sets the default world scene, for convinient retrieval with GetDefaultWorldScene().
    void SetDefaultScene(const QString &name);

    //! Sets the default world scene, for convinient retrieval with GetDefaultWorldScene().
    void SetDefaultScene(const Scene::ScenePtr &scene);
    
    //! Returns the default scene shared ptr.
    /// \todo remove this function when we move to QPointer/QSharedPointer/QWeakPointer<Scene::SceneManager> rename GetDefaultSceneRaw() to GetDefaultScene().
    const Scene::ScenePtr &GetDefaultScene() const;

    //! Returns the default scene ptr.
    Scene::SceneManager* GetDefaultSceneRaw() const;

    //! Returns a pointer to a scene
    /*! Manage the pointer carefully, as scenes may not get deleted properly if
        references to the pointer are left alive.

        \note Returns a shared pointer, but it is preferable to use a weak pointer, Scene::SceneWeakPtr,
              to avoid dangling references that prevent scenes from being properly destroyed.

        \param name Name of the scene to return
        \return The scene, or empty pointer if the scene with the specified name could not be found 
    */
    /// \todo remove this function when we move to QPointer/QSharedPointer/QWeakPointer<Scene::SceneManager> rename GetSceneRaw() to GetScene().
    Scene::ScenePtr GetScene(const QString &name) const;

    //! Returns a scene by name
    Scene::SceneManager* GetSceneRaw(const QString& name) const;

    //! Creates new empty scene.
    /*! \param name name of the new scene
        \param viewenabled Whether the scene is view enabled
        \return The new scene, or empty pointer if scene with the specified name already exists.
    */
    Scene::ScenePtr CreateScene(const QString &name, bool viewenabled);

    //! Removes a scene with the specified name.
    /*! The scene may not get deleted since there may be dangling references to it.
        If the scene does get deleted, removes all entities which are not shared with
        another existing scene.

        Does nothing if scene with the specified name doesn't exist.

        \param name name of the scene to delete
    */
    void RemoveScene(const QString &name);

    //! Returns the scene map for self reflection / introspection.
    const SceneMap &GetSceneMap() const;

private:
    //! Constructor. Framework takes ownership of this object.
    /// \param framework Foundation::Framework ptr.
    explicit SceneAPI(Foundation::Framework *framework);

    //! Frees all known scene and the scene interact object.
    /// \note This function is called by our fried class Foundation::Framework in its UnloadModules() function.
    void Reset();

    //! Registers the scene relaetd events to event manager.
    /// \note This function is called by our fried class Foundation::Framework when EvenManager is ready.
    void RegisterSceneEvents() const;
    
    //! Initialize the scene interact object. Needs framework->Input() to be valid.
    /// \note This function is called by our fried class Foundation::Framework when InputAPI is ready.
    void Initialise();

    //! PostInitialize the scene interact object as RenderServiceInterface is now available.
    /// \note This function is called by our fried class Foundation::Framework when modules have loaded and RenderServiceInterface is ready.
    void PostInitialize();

    //! Framework ptr.
    Foundation::Framework *framework_;

    //! Map of scenes.
    SceneMap scenes_;

    //! Current 'default' scene.
    Scene::ScenePtr defaultScene_;

    //! Scene interact shared ptr.
    QSharedPointer<SceneInteract> sceneInteract_;

    //! Scene events category name.
    std::string sceneCatergoryName_;

};

#endif