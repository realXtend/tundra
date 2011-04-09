// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "SceneAPI.h"
#include "SceneManager.h"

#include "EventManager.h"

#include "MemoryLeakCheck.h"

SceneAPI::SceneAPI(Foundation::Framework *framework) :
    QObject(framework),
    framework_(framework)
{
    sceneInteract = QSharedPointer<SceneInteract>(new SceneInteract());
    framework->RegisterDynamicObject("sceneinteract", sceneInteract.data());

    defaultScene_.reset();
    scenes_.clear();
}

SceneAPI::~SceneAPI()
{
}

void SceneAPI::Reset()
{
    sceneInteract.clear();
    defaultScene_.reset();
    scenes_.clear();
}

void SceneAPI::Initialise()
{
    sceneInteract->Initialize(framework_);
}

void SceneAPI::PostInitialize()
{
    sceneInteract->PostInitialize();
}

SceneInteractWeakPtr SceneAPI::GetSceneInteract() const
{
    return SceneInteractWeakPtr(sceneInteract);
}

bool SceneAPI::HasScene(const QString &name) const
{
    return scenes_.find(name) != scenes_.end();
}

void SceneAPI::SetDefaultScene(const QString &name)
{
    Scene::ScenePtr scene = GetScene(name);
    if(scene != defaultScene_)
    {
        defaultScene_ = scene;
        emit DefaultWorldSceneChanged(defaultScene_.get());
    }
}

void SceneAPI::SetDefaultScene(const Scene::ScenePtr &scene)
{
    if (scene != defaultScene_)
    {
        defaultScene_ = scene;
        emit DefaultWorldSceneChanged(defaultScene_.get());
    }
}

const Scene::ScenePtr &SceneAPI::GetDefaultScene() const
{
    return defaultScene_;
}

Scene::SceneManager* SceneAPI::GetDefaultSceneRaw() const
{
    return defaultScene_.get();
}

Scene::ScenePtr SceneAPI::GetScene(const QString &name) const
{
    SceneMap::const_iterator scene = scenes_.find(name);
    if (scene != scenes_.end())
        return scene->second;
    return Scene::ScenePtr();  
}

Scene::ScenePtr SceneAPI::CreateScene(const QString &name, bool viewenabled)
{
    if (HasScene(name))
        return Scene::ScenePtr();

    Scene::ScenePtr newScene = Scene::ScenePtr(new Scene::SceneManager(name, framework_, viewenabled));
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
        // If default scene is being removed. Reset our ref so it does not keep ref count alive.
        if (defaultScene_ == sceneIter->second)
            defaultScene_.reset();
        scenes_.erase(sceneIter);

        // Emit signal about removed scene
        emit SceneRemoved(name);
    }
}

const SceneMap &SceneAPI::GetSceneMap() const
{
    return scenes_;
}
