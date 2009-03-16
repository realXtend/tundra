// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SceneManager.h"
#include "Generic.h"

namespace Scene
{
    Foundation::ScenePtr SceneManager::CreateScene(const std::string &name)
    {
        assert (module_);
        Foundation::ScenePtr scene = Foundation::ScenePtr(new Scene::Generic(name, module_));
        
        scenes_[name] = scene;

        return scene;
    }

    void SceneManager::DeleteScene(const std::string &name)
    {
        assert (HasScene(name) == true);

        SceneMap::iterator it = scenes_.find(name);
        it->second.reset();
        scenes_.erase(it);

        assert (HasScene(name) == false);
    }

    Foundation::ScenePtr SceneManager::CloneScene(const std::string &name, const std::string &cloneName)
    {
        assert (HasScene(name));
        assert (HasScene(cloneName) == false);
        
        Generic *oldScene = static_cast<Generic*>(scenes_.find(name)->second.get());

        Foundation::ScenePtr scene = Foundation::ScenePtr(new Scene::Generic(*oldScene, cloneName));

        scenes_[cloneName] = scene;

        assert (HasScene(cloneName));

        return scene;
    }
}

