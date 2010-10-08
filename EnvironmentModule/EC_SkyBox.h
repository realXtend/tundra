// For conditions of distribution and use, see copyright notice in license.txt

#ifndef EC_SKYBOX_H_
#define EC_SKYBOX_H_

#include "IComponent.h"
#include "IAttribute.h"
#include "Declare_EC.h"
#include "CoreTypes.h"
#include "RexTypes.h"
#include "Quaternion.h"

#include <QVariant>


/// Makes the entity a SkyBox

/**

<table class="header">
<tr>
<td>
<h2>SkyBox plane</h2>

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
    class EC_SkyBox : public IComponent
    {
        Q_OBJECT
        DECLARE_EC(EC_SkyBox);
    
        public:
            virtual ~EC_SkyBox();

            virtual bool IsSerializable() const { return true; }
            
            /// Name of sky material 
            DEFINE_QPROPERTY_ATTRIBUTE(QString, materialAttr);
            Q_PROPERTY(QString  materialAttr READ getmaterialAttr WRITE setmaterialAttr); 
            
            /// Defines sky material texture
            DEFINE_QPROPERTY_ATTRIBUTE(QVariantList, textureAttr);
            Q_PROPERTY(QVariantList textureAttr READ gettextureAttr WRITE settextureAttr); 
            
            DEFINE_QPROPERTY_ATTRIBUTE(float, distanceAttr);
            Q_PROPERTY(float distanceAttr READ getdistanceAttr WRITE setdistanceAttr); 
            
             DEFINE_QPROPERTY_ATTRIBUTE(Quaternion, orientationAttr);
            Q_PROPERTY(Quaternion orientationAttr READ getorientationAttr WRITE setorientationAttr); 
            
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
            explicit EC_SkyBox(IModule *module);
        
           /**
            * Helper function which is used to update sky plane state. 
            *
            */
            void ChangeSkyBox(IAttribute* attribute);

            void CreateSky();
           
            
            void SetTextures();
         
            QString lastMaterial_;
            float lastDistance_;
            bool lastDrawFirst_;
            Quaternion lastOrientation_;
            QVariantList lastTextures_;

            /// Renderer
            OgreRenderer::RendererWeakPtr renderer_;

    };



}

#endif // EC_SKYBOX_H_
