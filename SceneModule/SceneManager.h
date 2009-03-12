// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_SceneSceneManager_h
#define incl_SceneSceneManager_h

//#include "SceneServiceInterface.h"
#include "SceneManagerServiceInterface.h"

namespace Foundation
{
    class Framework;
}

namespace Scene
{
    //! Manages scenes. 
    /*!
        Contains all scenes in a generic fashion.
        Acts as a factory for scenes.
    */
    class SceneManager : public Foundation::SceneManagerServiceInterface
    {
    public:
        SceneManager(Foundation::Framework *framework) : SceneManagerServiceInterface(), framework_(framework) {}
        virtual ~SceneManager() {}

        //! Creates new empty scene
        /*!
            \param name name of the new scene
            \return empty scene
        */
        virtual Foundation::ScenePtr CreateScene(const std::string &name);
        
        //! Returns a scene
        /*!
            Precondition: HasScene(name)

            \param name name of the scene
        */
        virtual Foundation::ScenePtr GetScene(const std::string &name) const
        {
            SceneMap::const_iterator it = scenes_.find(name);
            if (it != scenes_.end())
                return it->second;

            //const std::string e(std::string("Failed to find scene: " + name);
            throw Core::Exception((std::string("Failed to find scene: " + name)).c_str());
        }

        //! Returns true if a scene with the specified name is contained within this manager
        virtual bool HasScene(const std::string &name) const
        {
            return (scenes_.find(name) != scenes_.end());
        }
    
    private:
        typedef std::map<std::string, Foundation::ScenePtr> SceneMap;

        //! container for entities managed by this scene manager
        SceneMap scenes_;

        //! framework
        Foundation::Framework *framework_;
    };
}

#endif
