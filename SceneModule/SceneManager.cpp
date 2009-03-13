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

    Foundation::ScenePtr SceneManager::CloneScene(const std::string &name, const std::string &cloneName)
    {
        assert (scenes_.find(name) != scenes_.end());
        
        Generic *oldScene = static_cast<Generic*>(scenes_.find(name)->second.get());

        Foundation::ScenePtr scene = Foundation::ScenePtr(new Scene::Generic(*oldScene, cloneName));

        scenes_[cloneName] = scene;

        return scene;
    }
}

