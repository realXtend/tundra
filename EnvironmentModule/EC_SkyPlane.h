// For conditions of distribution and use, see copyright notice in license.txt

#ifndef EC_SKYPLANE_H_
#define EC_SKYPLANE_H_

#include "IComponent.h"
#include "IAttribute.h"
#include "Declare_EC.h"
#include "CoreTypes.h"
#include "RexTypes.h"

/// Makes the entity a SkyPlane

/**

<table class="header">
<tr>
<td>
<h2>Skyplane</h2>

Registered by Enviroment::EnvironmentModule. 

<b>Attributes</b>:
<ul>
<li> QString : materialAttr.
<div> Defines sky material </div>
<li> QString : textureAttr.
<div> Sky texture which is used in given material </div>
<li> float : tilingAttr.
<div> How many times to tile the texture(s) across the plane. </div>
<li> float : scaleAttr.
<div> The scaling applied to the sky plane - higher values mean a bigger sky plane  </div>
<li> float : bowAttr.
<div> If zero, the plane will be completely flat (like previous versions. If above zero, the plane will be curved, allowing the sky to appear below camera level. 
      Curved sky planes are simular to skydomes, but are more compatable with fog.  </div>
<li> float : distanceAttr.
<div> Plane distance in world coordinates from the camera </div>
<li> int : xSegmentsAttr.
<div> Defines how many vertices is used in drawing in x - axis.  </div>
<li> int : ySegmentsAttr.
<div> Defines how many vertices is used in drawing in y - axis.  </div>
<li> bool : drawFirstAttr.
<div> If true, the plane is drawn before all other geometry in the scene. </div>

</ul>

</table>

*/

namespace OgreRenderer
{
    class Renderer;
    typedef boost::shared_ptr<Renderer> RendererPtr;
    typedef boost::weak_ptr<Renderer> RendererWeakPtr;
}


namespace Environment
{
    class EC_SkyPlane : public IComponent
    {
        Q_OBJECT
        DECLARE_EC(EC_SkyPlane);
    
        public:
            virtual ~EC_SkyPlane();

            virtual bool IsSerializable() const { return true; }
            
            /// Name of sky material 
            DEFINE_QPROPERTY_ATTRIBUTE(QString, materialAttr);
            Q_PROPERTY(QString  materialAttr READ getmaterialAttr WRITE setmaterialAttr); 
            
            /// Defines sky material texture
            DEFINE_QPROPERTY_ATTRIBUTE(QString, textureAttr);
            Q_PROPERTY(QString textureAttr READ gettextureAttr WRITE settextureAttr); 

            DEFINE_QPROPERTY_ATTRIBUTE(float, tilingAttr);
            Q_PROPERTY(float tilingAttr READ gettilingAttr WRITE settilingAttr); 

            DEFINE_QPROPERTY_ATTRIBUTE(float, scaleAttr);
            Q_PROPERTY(float scaleAttr READ getscaleAttr WRITE setscaleAttr); 

            /// Defines how much sky bows
            DEFINE_QPROPERTY_ATTRIBUTE(float, bowAttr);
            Q_PROPERTY(float bowAttr READ getbowAttr WRITE setbowAttr); 
            
            DEFINE_QPROPERTY_ATTRIBUTE(float, distanceAttr);
            Q_PROPERTY(float distanceAttr READ getdistanceAttr WRITE setdistanceAttr); 
            
            /// Defines how many vertices is used in drawing in x - axis.
            DEFINE_QPROPERTY_ATTRIBUTE(int, xSegmentsAttr);
            Q_PROPERTY(int xSegmentsAttr READ getxSegmentsAttr WRITE setxSegmentsAttr); 
            
            /// Defines how many vertices is used in drawing in y - axis.
            DEFINE_QPROPERTY_ATTRIBUTE(int, ySegmentsAttr);
            Q_PROPERTY(int ySegmentsAttr READ getySegmentsAttr WRITE setySegmentsAttr); 

             /// Defines that is sky drawn first
            DEFINE_QPROPERTY_ATTRIBUTE(bool, drawFirstAttr);
            Q_PROPERTY(bool drawFirstAttr READ getdrawFirstAttr WRITE setdrawFirstAttr); 
           
        public slots: 
            
             /// Called If some of the attributes has been changed.
            void AttributeUpdated(IAttribute* attribute, AttributeChange::Type change);
            void DisableSky();
    
        private:
            

           /** 
            * Constuctor.
            * @param module Module where component belongs.
            **/
            explicit EC_SkyPlane(IModule *module);
        
           /**
            * Helper function which is used to update sky plane state. 
            *
            */
            void ChangeSkyPlane(IAttribute* attribute);

            void CreateSky();
           
            
            void SetTexture();
         
            QString lastMaterial_;
            float lastTiling_;
            float lastScale_;
            float lastBow_;
            float lastDistance_;
            int lastxSegments_;
            int lastySegments_;
            bool lastDrawFirst_;

            /// Renderer
            OgreRenderer::RendererWeakPtr renderer_;

    };



}

#endif // EC_SKYPLANE_H_
