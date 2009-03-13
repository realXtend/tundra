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

        //! Clones a scene. The new scene will contain the same entities as the old one.
        /*!
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
    };

    typedef boost::shared_ptr<SceneManagerServiceInterface> SceneManagerPtr;
}

#endif

