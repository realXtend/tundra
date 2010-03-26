// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_OgreSkeletonResource_h
#define incl_OgreRenderer_OgreSkeletonResource_h

#include "AssetInterface.h"
#include "ResourceInterface.h"
#include "OgreModuleApi.h"

#include <OgreSkeleton.h>

namespace OgreRenderer
{
    class OgreSkeletonResource;
    typedef boost::shared_ptr<OgreSkeletonResource> OgreSkeletonResourcePtr;

    //! An Ogre-specific skeleton resource, contains bone structure and skeletal animations
    /*! \ingroup OgreRenderingModuleClient
     */
    class OGRE_MODULE_API OgreSkeletonResource : public Foundation::ResourceInterface
    {
    public:
        //! Generates an empty unloaded skeleton resource.
        /*! \param id The resource ID that is associated to this skeleton.
         */
        explicit OgreSkeletonResource(const std::string& id);
        
        //! constructor
        /*! \param id material id
            \param source asset data to construct skeleton from
        */
        OgreSkeletonResource(const std::string& id, Foundation::AssetPtr source);

        //! destructor
        virtual ~OgreSkeletonResource();

        //! returns resource type in text form
        virtual const std::string& GetType() const;

        //! returns whether resource valid
        virtual bool IsValid() const;

        //! returns Ogre skeleton
        /*! may be null if no data successfully set yet
         */
        Ogre::SkeletonPtr GetSkeleton() const { return ogre_skeleton_; }

        //! sets contents from asset data
        /*! \param source asset data to construct the skeleton from
            \return true if successful
        */
        bool SetData(Foundation::AssetPtr source);

        //! returns resource type in text form (static)
        static const std::string& GetTypeStatic();

    private:
        Ogre::SkeletonPtr ogre_skeleton_;
        
        //! Deinitializes the skeleton and frees all Ogre-side structures as well.
        void RemoveSkeleton();
        
    };
}

#endif
