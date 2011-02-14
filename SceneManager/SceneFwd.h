/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneFwd.h
 *  @brief  Forward declarations and typedefs for Scene-related clasess.
 */

#ifndef incl_Scene_SceneFwd
#define incl_Scene_SceneFwd

#include <boost/smart_ptr.hpp>

namespace Scene
{
    class Entity;
    class SceneManager;

    typedef boost::shared_ptr<Entity> EntityPtr;
    typedef boost::shared_ptr<SceneManager> ScenePtr;

    typedef boost::weak_ptr<Entity> EntityWeakPtr;
    typedef boost::weak_ptr<SceneManager> SceneWeakPtr;
}

#endif
