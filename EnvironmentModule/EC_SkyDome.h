// For conditions of distribution and use, see copyright notice in license.txt

#ifndef EC_SKYDOME_H_
#define EC_SKYDOME_H_

#include "IComponent.h"
#include "IAttribute.h"
#include "Declare_EC.h"
#include "CoreTypes.h"
#include "RexTypes.h"
#include "Quaternion.h"

#include <QVariant>


/// Makes the entity a SkyDome

/**

<table class="header">
<tr>
<td>
<h2>SkyDome plane</h2>

Registered by Enviroment::EnvironmentModule. 

<b>Attributes</b>:
<ul>
<li> QString : materialAttr.
<div>  </div>
<li> QString : textureAttr.
<div>  </div>
<li> Quaternion : orientationAttr
<div> </div>
<li> float : distanceAttr.
<div>  </div>
<li> bool : drawFirstAttr.
<div>  </div>

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

namespace OgreRenderer
{
    class Renderer;
    typedef boost::shared_ptr<Renderer> RendererPtr;
    typedef boost::weak_ptr<Renderer> RendererWeakPtr;
}


namespace Environment
{
    class EC_SkyDome : public IComponent
    {
        Q_OBJECT
        DECLARE_EC(EC_SkyDome);
    
        public:
            virtual ~EC_SkyDome();

            virtual bool IsSerializable() const { return true; }
           

            /// Name of sky material 
            DEFINE_QPROPERTY_ATTRIBUTE(QString, materialAttr);
            Q_PROPERTY(QString  materialAttr READ getmaterialAttr WRITE setmaterialAttr); 
            
            /// Defines sky material texture
            DEFINE_QPROPERTY_ATTRIBUTE(QString, textureAttr);
            Q_PROPERTY(QString textureAttr READ gettextureAttr WRITE settextureAttr); 
            
            DEFINE_QPROPERTY_ATTRIBUTE(float, distanceAttr);
            Q_PROPERTY(float distanceAttr READ getdistanceAttr WRITE setdistanceAttr); 

			DEFINE_QPROPERTY_ATTRIBUTE(float, curvatureAttr);
            Q_PROPERTY(float curvatureAttr READ getcurvatureAttr WRITE setcurvatureAttr); 

		    DEFINE_QPROPERTY_ATTRIBUTE(float, tilingAttr);
            Q_PROPERTY(float tilingAttr READ gettilingAttr WRITE settilingAttr); 

			DEFINE_QPROPERTY_ATTRIBUTE(int, xSegmentsAttr);
            Q_PROPERTY(int xSegmentsAttr READ getxSegmentsAttr WRITE setxSegmentsAttr); 

			DEFINE_QPROPERTY_ATTRIBUTE(int, ySegmentsAttr);
            Q_PROPERTY(int ySegmentsAttr READ getySegmentsAttr WRITE setySegmentsAttr); 
            
            DEFINE_QPROPERTY_ATTRIBUTE(Quaternion, orientationAttr);
            Q_PROPERTY(Quaternion orientationAttr READ getorientationAttr WRITE setorientationAttr); 
           	
            DEFINE_QPROPERTY_ATTRIBUTE(int, ySegmentsKeepAttr);
            Q_PROPERTY(int ySegmentsKeepAttr READ getySegmentsKeepAttr WRITE setySegmentsKeepAttr); 
            
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
            explicit EC_SkyDome(IModule *module);
        
           /**
            * Helper function which is used to update sky plane state. 
            *
            */
            void ChangeSkyDome(IAttribute* attribute);

            void CreateSky(); 
            void SetTexture();
         
            QString lastMaterial_;
            float lastDistance_;
            bool lastDrawFirst_;
            Quaternion lastOrientation_;
            QString lastTexture_;
            float lastTiling_;
            float lastCurvature_;
            int lastxSegments_;
            int lastySegments_;
            int lastySegmentsKeep_;

            /// Renderer
            OgreRenderer::RendererWeakPtr renderer_;

    };



}

#endif // EC_SkyDome_H_
