// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_SceneManagerServiceInterface_h
#define incl_Interfaces_SceneManagerServiceInterface_h

#include "ServiceInterface.h"
#include "SceneInterface.h"

namespace Foundation
{
    //! Interface for scene managers
    class SceneManagerServiceInterface : public ServiceInterface
    {
    public:
        // Implementation details in an abstract interface, ouch. Less work than creating our own iterator though.
        typedef std::map<std::string, Foundation::ScenePtr> SceneMap;
        typedef SceneMap::iterator iterator;
        typedef SceneMap::const_iterator const_iterator;
        //typedef typename boost::shared_ptr<SceneManagerServiceInterface<iterator>> SceneManagerPtr;
    //    typedef typename class iterator;
       // typedef const_iterator;
    public:
        //! default constructor
        SceneManagerServiceInterface() {}

        //! destructor
        virtual ~SceneManagerServiceInterface() {}

        //! Creates new empty scene
        /*!
            \param name name of the new scene
            \return empty scene
        */
        virtual Foundation::ScenePtr CreateScene(const std::string &name) = 0;

        //! Deletes a scene and all entities in it
        /*! Since entities may be shared between scenes, not all entities may get deleted.
            
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
            Precondition: HasScene(name)

            \param name name of the scene
        */
        virtual Foundation::ScenePtr GetScene(const std::string &name) const = 0;

        //! Returns true if a scene with the specified name is contained within this manager
        virtual bool HasScene(const std::string &name) const = 0;

        virtual iterator Begin() = 0;
        virtual const_iterator Begin() const = 0;
        virtual iterator End() = 0;
        virtual const_iterator End() const = 0;
    };

    typedef SceneManagerServiceInterface SceneManager;
    typedef boost::shared_ptr<SceneManager> SceneManagerPtr;
}

#endif

