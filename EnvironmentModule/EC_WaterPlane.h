// For conditions of distribution and use, see copyright notice in license.txt

#ifndef EC_WATERPLANE_H_
#define EC_WATERPLANE_H_

#include "IComponent.h"
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
    class ColourValue;
}


namespace OgreRenderer
{
    class Renderer;
    typedef boost::shared_ptr<Renderer> RendererPtr;
    typedef boost::weak_ptr<Renderer> RendererWeakPtr;
}

/// Makes the entity a water plane.

/**

<table class="header">
<tr>
<td>
<h2>Water plane</h2>

Registered by Enviroment::EnvironmentModule. Water plane component defines into world actually water cube. Inside of that water cube scene fog is changed to correspond, given
water plane underwater fog properties. Water plane cannot visualize outside as a water cube (it still looks just plane). 

<b>Attributes</b>:
<ul>
<li> int : xSizeAttr.
<div> Water plane size in x-axis. </div>
<li> int : ySizeAttr. 
<div> Water plane size in y-axis. </div>
<li> int : depthAttr.
<div> Depth value defines that how much below from surface water fog colour is used. Meaning this attribute defines how "deep" is our ocean/pond. </div>
<li> Vector3df : positionAttr.
<div> Defines position of water plane in world coordinate system. </div>
<li> Quaternion : rotationAttr.
<div> Defines rotation of water plane in world coordinate system. </div>
<li> float : scaleUfactorAttr.
<div> Water plane texture factor which defines how many times the texture should be repeated in the u direction. Note current default value 
 is so small 0.002, so it does not show up correctly in EC-editor. </div>
<li> float : scaleVfactorAttr.
<div> Water plane texture factor which defines how many times the texture should be repeated in the v direction. Note current default value 
 is so small 0.002, so it does not show up correctly in EC-editor. </div>
<li> int : xSegmentsAttr.
<div> The number of segments to the plane in the x direction.  </div>
<li> int : ySegmentsAttr.
<div> The number of segments to the plane in the y direction.  </div>
<li> QString : materialNameAttr.
<div> Defines what material is used in creating plane. </div>
<li> Color : fogColorAttr.
<div> Defines what is fog color when camera is inside of water cube which this plane defines. </div>
<li> float : fogStartAttr.
<div> Underwater fog start distance (meters) </div>
<li> float : fogEndAttr.
<div> Underwater fog end distance (meters) </div>
<li> enum :  fogModeAttr.
<div> UnderWater fog mode, defines how Fog density increases. </div>
</ul>

<b> Exposes the following scriptable functions: </b>
<ul>
<li>...
</ul>

<b> Reacts on the following actions: </b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>Depends on the component OgrePlaceable</b>. The position in the OgrePlaceable component specifies the position in the world space where this water plane is by default is placed at. 
</table>

*/

namespace Environment
{

class EC_WaterPlane : public IComponent
{
    Q_OBJECT
    DECLARE_EC(EC_WaterPlane);
    
    public:
        virtual ~EC_WaterPlane();

        virtual bool IsSerializable() const { return true; }
       
        /// Water plane x-size
        DEFINE_QPROPERTY_ATTRIBUTE(int, xSizeAttr);
        Q_PROPERTY(int xSizeAttr READ getxSizeAttr WRITE setxSizeAttr); 
       
        /// Water plane y-size
        DEFINE_QPROPERTY_ATTRIBUTE(int, ySizeAttr);
        Q_PROPERTY(int ySizeAttr READ getySizeAttr WRITE setySizeAttr); 

      
        /**
         * Water plane "depth". This is used to define when we are below water. 
         * and inside of watercube. 
         */
        DEFINE_QPROPERTY_ATTRIBUTE(int, depthAttr);
        Q_PROPERTY(int depthAttr READ getdepthAttr WRITE setdepthAttr); 
        
        /// Water plane position (this is used if there is not EC_Placeable)
        DEFINE_QPROPERTY_ATTRIBUTE(Vector3df, positionAttr);
        Q_PROPERTY(Vector3df positionAttr READ getpositionAttr WRITE setpositionAttr); 
        
        /// Water plane rotation
        DEFINE_QPROPERTY_ATTRIBUTE(Quaternion, rotationAttr);
        Q_PROPERTY(Quaternion rotationAttr READ getrotationAttr WRITE setrotationAttr); 
        
        ///U Scale, factor which defines how many times the texture should be repeated in the u direction    
        DEFINE_QPROPERTY_ATTRIBUTE(float, scaleUfactorAttr);
        Q_PROPERTY(float scaleUfactorAttr READ getscaleUfactorAttr WRITE setscaleUfactorAttr); 
        
        ///V Scale, factor which defines how many times the texture should be repeated in the v direction   
        DEFINE_QPROPERTY_ATTRIBUTE(float, scaleVfactorAttr);
        Q_PROPERTY(float scaleVfactorAttr READ getscaleVfactorAttr WRITE setscaleVfactorAttr); 
       
        /// The number of segments to the plane in the x direction 
        DEFINE_QPROPERTY_ATTRIBUTE(int, xSegmentsAttr);
        Q_PROPERTY(int xSegmentsAttr READ getxSegmentsAttr WRITE setxSegmentsAttr); 

        /// The number of segments to the plane in the y direction 
        DEFINE_QPROPERTY_ATTRIBUTE(int, ySegmentsAttr);
        Q_PROPERTY(int ySegmentsAttr READ getySegmentsAttr WRITE setySegmentsAttr); 

        /// Material name
        DEFINE_QPROPERTY_ATTRIBUTE(QString, materialNameAttr);
        Q_PROPERTY(QString materialNameAttr READ getmaterialNameAttr WRITE setmaterialNameAttr); 
        
        /// Underwater fog color
        DEFINE_QPROPERTY_ATTRIBUTE(Color,  fogColorAttr);
        Q_PROPERTY(Color fogColorAttr READ getfogColorAttr WRITE setfogColorAttr); 
   
        /// Underwater fog start distance (meters)
        DEFINE_QPROPERTY_ATTRIBUTE(float, fogStartAttr);
        Q_PROPERTY(float fogStartAttr READ getfogStartAttr WRITE setfogStartAttr); 
        
        /// Underwater fog end distance (meters)
        DEFINE_QPROPERTY_ATTRIBUTE(float, fogEndAttr);
        Q_PROPERTY(float fogEndAttr READ getfogEndAttr WRITE setfogEndAttr); 
        
        /// UnderWater fog mode, defines how Fog density increases.
        DEFINE_QPROPERTY_ATTRIBUTE(int, fogModeAttr);
        Q_PROPERTY(int fogModeAttr READ getfogModeAttr WRITE setfogModeAttr); 
        
        /// Returns color value in Ogre format.
        Ogre::ColourValue GetFogColorAsOgreValue() const;
       
    public slots: 
        
        /** 
         * Returns true if camera is inside of watercube. 
         */
        bool IsUnderWater();

        /// When called creates new water plane into world and tries to attacht it.
        void CreateWaterPlane();
        /// When called removes water plane from world.
        void RemoveWaterPlane();
        
        //! Attach a new entity to scene node that world scene owns.
        void AttachEntity();

        //! Detach entity from the scene node.
        void DetachEntity();

        /// Called If some of the attributes has been changed.
        void AttributeUpdated(IAttribute* attribute, AttributeChange::Type change);

    private:
        /** 
         * Constuctor.
         * @param module Module where component belongs.
         **/
        explicit EC_WaterPlane(IModule *module);
        
        /**
         * Finds out that is EC_OgrePlaceable component connected to same entity where waterplane compontent is placed. 
         * @returns component pointer to EC_OgrePlaceable component.
         */
        ComponentPtr FindPlaceable() const;
        
        /**
         * Helper function which is used to update water plane state. 
         */
        void ChangeWaterPlane(IAttribute* attribute);
        
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