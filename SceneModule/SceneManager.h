// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_SceneSceneManager_h
#define incl_SceneSceneManager_h

#include "SceneManagerServiceInterface.h"

namespace Foundation
{
    class Framework;
}

namespace Scene
{
    class SceneModule;

    //! Manages scenes. 
    /*!
        Contains all scenes in a generic fashion.
        Acts as a factory for scenes.
    */
    class SceneManager : public Foundation::SceneManagerServiceInterface
    {
        friend class SceneModule;
    private:
        //! default constructor not applicable
        SceneManager();
        //! constructor that takes a framework
        SceneManager(SceneModule *module) : SceneManagerServiceInterface(), module_(module) {}

    public:
        virtual ~SceneManager() {}

        //! Creates new empty scene
        /*!
            \param name name of the new scene
            \return empty scene
        */
        virtual Foundation::ScenePtr CreateScene(const std::string &name);

        //! Deletes a scene and all entities in it
        /*! Since entities may be shared between scenes, not all entities may get deleted.
            
            Precondition: HasScene(name)
            Postcondition: !HasScene(name)

            \param name name of the scene to delete
        */
        virtual void DeleteScene(const std::string &name);

        //! Clones a scene. The new scene will contain the same entities as the old one.
        /*!
            Precondition:   HasScene(name)
                            !HasScene(cloneName)
            Postcondition:  HasScene(cloneName)

            \param name Name of the scene to clone
            \param cloneName name of the new scene
            \return Cloned scene
        */
        virtual Foundation::ScenePtr CloneScene(const std::string &name, const std::string &cloneName);
        
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

        //! parent module
        SceneModule *module_;
    };
}

#endif
