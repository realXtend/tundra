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
    public:
        typedef std::map<std::string, Foundation::ScenePtr> SceneMap;
    private:
        //! default constructor not applicable
        SceneManager();
        //! constructor that takes a framework
        SceneManager(SceneModule *module) : SceneManagerServiceInterface(), module_(module) {}

    public:
        //! destructor
        virtual ~SceneManager() {}

        virtual Foundation::ScenePtr CreateScene(const std::string &name);
        virtual void DeleteScene(const std::string &name);
        virtual Foundation::ScenePtr CloneScene(const std::string &name, const std::string &cloneName);
        virtual Foundation::ScenePtr GetScene(const std::string &name) const;

        virtual bool HasScene(const std::string &name) const
        {
            return (scenes_.find(name) != scenes_.end());
        }

        const SceneMap &GetSceneMap() const { return scenes_; }

    private:
        virtual SceneIteratorImplPtr SceneIteratorBegin()
        { 
            return SceneIteratorImplPtr(new Core::MapIterator<SceneMap::iterator, Foundation::ScenePtr>(scenes_.begin()));
        }
        virtual SceneIteratorImplPtr SceneIteratorEnd()
        { 
            return SceneIteratorImplPtr(new Core::MapIterator<SceneMap::iterator, Foundation::ScenePtr>(scenes_.end()));
        }

        virtual ConstSceneIteratorImplPtr ConstSceneIteratorBegin() const
        { 
            return ConstSceneIteratorImplPtr(new Core::MapIterator<SceneMap::const_iterator, const Foundation::ScenePtr>(scenes_.begin()));
        }
        virtual ConstSceneIteratorImplPtr ConstSceneIteratorEnd() const
        { 
            return ConstSceneIteratorImplPtr(new Core::MapIterator<SceneMap::const_iterator, const Foundation::ScenePtr>(scenes_.end()));
        }

    private:
        //! container for entities managed by this scene manager
        SceneMap scenes_;

        //! parent module
        SceneModule *module_;
    };
}

#endif
