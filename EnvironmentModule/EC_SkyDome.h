// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EnvironmentModule_EC_SkyDome_h
#define incl_EnvironmentModule_EC_SkyDome_h

#include "IComponent.h"
#include "IAttribute.h"
#include "Declare_EC.h"
#include "CoreTypes.h"
#include "OgreModuleFwd.h"
#ifdef ENABLE_TAIGA_SUPPORT
#include "RexTypes.h"
#endif
#include "Quaternion.h"
#include "AssetReference.h"

#include <QVariant>

namespace Environment
{
/// Makes the entity a SkyDome
/**
<table class="header">
<tr>
<td>
<h2>SkyDome</h2>

Registered by Enviroment::EnvironmentModule.

<h3> Used to synchronize sky in Taiga </h3>

Currently (not in Tundra) EC_SkyDome component can be used to synchronize sky in Taiga worlds. This can be done
so that user creates entity and sets entity EC_Name-component. If this component name is set as "SkyEnvironment" our current implementation
will create automatically a EC_SkyBox-component on it. This component is now usable for every users and all changes on it will be transfered 
to all users. This synchronized sky plane component can also edit through environment editor (in world tools). Currently Caelum must be disabled 
before these features can be used.

<b>Attributes</b>:
<ul>
<li> AssetReference: materialRef.
<div> Sky material reference.</div>
<li> AssetReference: textureRef.
<div> Sky texture reference.</div>
<li> float : distanceAttr.
<div> Distance in world coordinates from the camera to each plane of the box the dome is rendered on. </div>
<li> float : curvatureAttr.
<div> The curvature of the dome. Good values are between 2 and 65. Higher values are more curved leading to a smoother effect,
lower values are less curved meaning more distortion at the horizons but a better distance effect.  </div>
<li> float : tilingAttr.
<div> How many times to tile the texture(s) across the dome.  </div>
<li> int : xSegmentsAttr.
<div> The number of segments to the dome in the x direction.  </div>
<li> int : ySegmentsAttr.
<div> The number of segments to the dome in the y direction.  </div>
<li> int : ySegmentsKeepAttr.
<div> </div>
<li> bool : drawFirstAttr.
<div> If true, the dome is drawn before all other geometry in the scene, without updating the depth buffer  </div>
<li> Quaternion : orientationAttr
<div>  Optional parameter to specify the orientation of the dome </div>
</ul>
</table>
*/
    class EC_SkyDome : public IComponent
    {
        Q_OBJECT
        DECLARE_EC(EC_SkyDome);

    public:
        virtual ~EC_SkyDome();

        virtual bool IsSerializable() const { return true; }

        /// Sky material reference
        DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, materialRef);
        Q_PROPERTY(AssetReference materialRef READ getmaterialRef WRITE setmaterialRef);

        /// Sky texture reference.
        DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, textureRef);
        Q_PROPERTY(AssetReference textureRef READ gettextureRef WRITE settextureRef);

        /// Defines distance in world coordinates from the camera to each plane of the box the dome is rendered on.
        DEFINE_QPROPERTY_ATTRIBUTE(float, distanceAttr);
        Q_PROPERTY(float distanceAttr READ getdistanceAttr WRITE setdistanceAttr); 

        /// The curvature of the dome.
        /** Good values are between 2 and 65. Higher values are more curved leading to a smoother effect,
            lower values are less curved meaning more distortion at the horizons but a better distance effect.
        */
        DEFINE_QPROPERTY_ATTRIBUTE(float, curvatureAttr);
        Q_PROPERTY(float curvatureAttr READ getcurvatureAttr WRITE setcurvatureAttr);

        /// Defines how many times to tile the texture(s) across the dome. 
        DEFINE_QPROPERTY_ATTRIBUTE(float, tilingAttr);
        Q_PROPERTY(float tilingAttr READ gettilingAttr WRITE settilingAttr); 

        DEFINE_QPROPERTY_ATTRIBUTE(int, xSegmentsAttr);
        Q_PROPERTY(int xSegmentsAttr READ getxSegmentsAttr WRITE setxSegmentsAttr); 

        DEFINE_QPROPERTY_ATTRIBUTE(int, ySegmentsAttr);
        Q_PROPERTY(int ySegmentsAttr READ getySegmentsAttr WRITE setySegmentsAttr); 

        /// Optional parameter to specify the orientation of the dome
        DEFINE_QPROPERTY_ATTRIBUTE(Quaternion, orientationAttr);
        Q_PROPERTY(Quaternion orientationAttr READ getorientationAttr WRITE setorientationAttr); 

        DEFINE_QPROPERTY_ATTRIBUTE(int, ySegmentsKeepAttr);
        Q_PROPERTY(int ySegmentsKeepAttr READ getySegmentsKeepAttr WRITE setySegmentsKeepAttr); 

        /// If true, the dome is drawn before all other geometry in the scene, without updating the depth buffer
        DEFINE_QPROPERTY_ATTRIBUTE(bool, drawFirstAttr);
        Q_PROPERTY(bool drawFirstAttr READ getdrawFirstAttr WRITE setdrawFirstAttr); 

    public slots:
        void View(const QString &attributeName);
         /// Called when component's attribute value has changed.
        void OnAttributeUpdated(IAttribute* attribute);
        void DisableSky();

    private:
        /// Constructor.
        /**@param module Module where component belongs.
        */
        explicit EC_SkyDome(IModule *module);

        /// Helper function which is used to update sky plane state. 
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

#endif
