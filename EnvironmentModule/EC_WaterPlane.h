// For conditions of distribution and use, see copyright notice in license.txt

#ifndef EC_WATERPLANE_H_
#define EC_WATERPLANE_H_

#include "ComponentInterface.h"
#include "Declare_EC.h"
#include "CoreTypes.h"
#include "RexTypes.h"
#include <Color.h>
#include <QString>
#include <Quaternion.h>



namespace Ogre
{
    class Entity;
    class SceneNode;
}


namespace OgreRenderer
{
    class Renderer;
    typedef boost::shared_ptr<Renderer> RendererPtr;
    typedef boost::weak_ptr<Renderer> RendererWeakPtr;
}


namespace Environment
{ 

///HACK Because namespace problems with mock-compiler we need to do this. 
//using Foundation::AttributeInterface;

class EC_WaterPlane : public Foundation::ComponentInterface
{
    Q_OBJECT
    DECLARE_EC(EC_WaterPlane);
    
    public:
        virtual ~EC_WaterPlane();

        virtual bool IsSerializable() const { return true; }
       
        /// Water plane x-size
        Attribute<int> xSizeAttr_;
        /// Water plane y-size
        Attribute<int> ySizeAttr_;
       
        /**
         * Water plane "depth". This is used to define when we are below water. 
         * and inside of watercube. 
         */
        Attribute<int> depthAttr_;
        
        /// Water plane position (this is used if there is not EC_Placeable)
        Attribute<Vector3df> positionAttr_;
        /// Water plane rotation
        Attribute<Quaternion> rotationAttr_;
        
        ///todo U Scale 
        Attribute<float> scaleUfactorAttr_;
        ///todo V Scale
        Attribute<float> scaleVfactorAttr_;

        /// The number of segments to the plane in the x direction 
        Attribute<int> xSegmentsAttr_;
        /// The number of segments to the plane in the y direction 
        Attribute<int> ySegmentsAttr_;

        /// Material name
        Attribute<QString> materialNameAttr_;
        /// Underwater fog color
        Attribute<Color> fogColorAttr_;
        /// Underwater fog start distance (meters)
        Attribute<float> fogStartAttr_;
        /// Underwater fog end distance (meters)
        Attribute<float> fogEndAttr_;
        
         /** 
         * Returns true if camera is inside of watercube. 
         */
        bool IsUnderWater();
       
    public slots: 
        
        void CreateWaterPlane();
        void RemoveWaterPlane();
        
        //! Attach a new entity to scene node that world scene owns.
        void AttachEntity();

        //! Detach entity from the scene node.
        void DetachEntity();

        /// Called If some of the attributes has been changed.
        void AttributeUpdated(AttributeInterface* attribute, AttributeChange::Type change);

    private:
        /** 
         * Constuctor.
         * @param module Module where component belongs.
         **/
        explicit EC_WaterPlane(Foundation::ModuleInterface *module);
        
        /**
         * Finds out that is EC_OgrePlaceable component connected to same entity where waterplane compontent is placed. 
         * @returns component pointer to EC_OgrePlaceable component.
         */
        Foundation::ComponentPtr FindPlaceable() const;
        
        /**
         * Helper function which is used to update water plane state. 
         */
        void ChangeWaterPlane(AttributeInterface* attribute);
        
        /**
         * Changes water plane position, this function should be called only if 
         * entity where water plane is connected has not a EC_OgrePlaceable component. 
         * @note uses attribute @p positionAttr_ to for waterplane defining water plane posititon 
         */
        void SetPosition();
        
         /**
         * Changes water plane rotation, this function should be called only if 
         * entity where water plane is connected has not a EC_OgrePlaceable component. 
         * @note uses attribute @p rotationAttr_ to for waterplane defining water plane rotation
         */
        void SetOrientation();

        OgreRenderer::RendererWeakPtr renderer_;
        Ogre::Entity* entity_;
        Ogre::SceneNode* node_;
       
        bool attached_;

        int lastXsize_;
        int lastYsize_;

        
    
};

}

#endif // EC_WATERPLANE_H