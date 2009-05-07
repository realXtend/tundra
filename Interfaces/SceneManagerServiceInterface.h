// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_SceneManagerServiceInterface_h
#define incl_Interfaces_SceneManagerServiceInterface_h

#include "ServiceInterface.h"
#include "SceneInterface.h"

namespace Foundation
{
    //! A manager for scenes, offered as a service.
    /*! With this service, one can create, delete and clone scenes.
        Generally scenes are independent from each other, but may contain
        same entities.

        \ingroup Services_group
        \ingroup Scene_group
    */
    class SceneManagerServiceInterface : public ServiceInterface
    {
    public:
        typedef Core::AnyIterator<ScenePtr> iterator;
        typedef Core::AnyIterator<const ScenePtr> const_iterator;

        //! default constructor
        SceneManagerServiceInterface() {}

        //! destructor
        virtual ~SceneManagerServiceInterface() {}

        //! Creates new empty scene.
        /*! If a new scene with the specified name already exists, overrides it. The old scene
            may get deleted if no references are held to it by other classes.

            \param name name of the new scene
            \return empty scene
        */
        virtual Foundation::ScenePtr CreateScene(const std::string &name) = 0;

        //! Deletes a scene and all entities in it
        /*! Since entities may be shared between scenes, not all entities may get deleted.
            Also because of dangling references the scene may not get deleted immediatelly.
            
            Precondition: HasScene(name)
            Postcondition: !HasScene(name)

            \param name name of the scene to delete
        */
        virtual void DeleteScene(const std::string &name) = 0;

        //! Clones a scene. The new scene will contain the same entities as the old one.
        /*!
            Precondition:   HasScene(name)
                            !HasScene(cloneName)
            Postcondition:  HasScene(cloneName)

            \param name Name of the scene to clone
            \param cloneName name of the new scene
            \return Cloned scene
        */
        virtual Foundation::ScenePtr CloneScene(const std::string &name, const std::string &cloneName) = 0;
        
        //! Returns a scene
        /*!
            \todo Return weak pointer? -cm

            \param name name of the scene
            \return Scene with the specified name, or null pointer if the scene could not be found.
        */
        virtual Foundation::ScenePtr GetScene(const std::string &name) const = 0;

        //! Returns true if a scene with the specified name is contained within this manager, false otherwise.
        virtual bool HasScene(const std::string &name) const = 0;

        typedef boost::shared_ptr<Core::AnyIteratorInterface <ScenePtr> > SceneIteratorImplPtr;
        typedef boost::shared_ptr<Core::AnyIteratorInterface <const ScenePtr> > ConstSceneIteratorImplPtr;

        iterator Begin() { return iterator(SceneIteratorBegin()); }
        iterator End() { return iterator(SceneIteratorEnd()); }
        const_iterator Begin() const { return const_iterator(ConstSceneIteratorBegin()); }
        const_iterator End() const { return const_iterator(ConstSceneIteratorEnd()); }

    private:
        virtual SceneIteratorImplPtr SceneIteratorBegin() = 0;
        virtual SceneIteratorImplPtr SceneIteratorEnd() = 0;
        virtual ConstSceneIteratorImplPtr ConstSceneIteratorBegin() const = 0;
        virtual ConstSceneIteratorImplPtr ConstSceneIteratorEnd() const = 0;
    };

    typedef SceneManagerServiceInterface SceneManager;
    typedef boost::shared_ptr<SceneManager> SceneManagerPtr;
}

#endif

