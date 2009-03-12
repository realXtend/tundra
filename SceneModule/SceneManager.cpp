// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SceneManager.h"
#include "Generic.h"

namespace Scene
{
    Foundation::ScenePtr SceneManager::CreateScene(const std::string &name)
    {
        assert (framework_);
        Foundation::ScenePtr scene = Foundation::ScenePtr(new Scene::Generic(framework_));
        
        scenes_[name] = scene;

        return scene;
    }
}

