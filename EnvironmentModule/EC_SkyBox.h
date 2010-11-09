// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EnvironmentModule_EC_SkyBox_h
#define incl_EnvironmentModule_EC_SkyBox_h

#include "IComponent.h"
#include "IAttribute.h"
#include "Declare_EC.h"
#include "CoreTypes.h"
#include "RexTypes.h"
#include "Quaternion.h"
#include "AssetReference.h"
#include "OgreModuleFwd.h"

#include <QVariant>

namespace Environment
{
/// Makes the entity a SkyBox
/**
<table class="header">
<tr>
<td>
<h2>SkyBox plane</h2>

Registered by Enviroment::EnvironmentModule. 

<h3> Using component to syncronize sky in Taiga </h3>

Currently (not in Tundra) EC_SkyBox component can be used to syncronize sky in Taiga worlds. This can be done
so that user creates entity and sets entity EC_Name-component. If this component name is set as "SkyEnvironment" our current implementation
will create automagically a EC_SkyBox-component on it. This component is now usable for every users and all changes on it will be transfered 
to all users. This syncronized sky plane component can also edit through environment editor (in world tools). Currently Caelum must be disabled 
before these features can be used.

<b>Attributes</b>:
<ul>
<li> AssetReference: materialRef.
<div> Sky material reference.</div>
<li> QString : materialAttr.
<div> The name of the material the box will use </div>
<li> QVariantList : textureAttr.
<div>  List of </div>
<li> Quaternion : orientationAttr
<div> Optional parameter to specify the orientation of the box. </div>
<li> float : distanceAttr.
<div> Distance in world coordinates from the camera to each plane of the box. </div>
<li> bool : drawFirstAttr.
<div> If true, the box is drawn before all other geometry in the scene. </div>

</ul>
</table>
*/
    class EC_SkyBox : public IComponent
    {
        Q_OBJECT
        DECLARE_EC(EC_SkyBox);
    
    public:
        virtual ~EC_SkyBox();

        virtual bool IsSerializable() const { return true; }

        /// Sky material reference
        DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, materialRef);
        Q_PROPERTY(AssetReference materialRef READ getmaterialRef WRITE setmaterialRef);

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

#endif
