// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EnvironmentModule_EC_SkyBox_h
#define incl_EnvironmentModule_EC_SkyBox_h

#include "IComponent.h"
#include "IAttribute.h"
#include "Quaternion.h"
#include "AssetReference.h"
#include "OgreModuleFwd.h"
#include "AssetRefListener.h"

namespace Environment
{
/// Makes the entity a SkyBox
/**
<table class="header">
<tr>
<td>
<h2>SkyBox plane</h2>

Registered by Enviroment::EnvironmentModule. 

<h3> Using component to synchronize sky in Taiga </h3>

Currently (not in Tundra) EC_Sky component can be used to synchronize sky in Taiga worlds.
This can be done so that user creates entity and sets entity EC_Name-component.

If this component name is set as "SkyEnvironment" our current implementation will create automatically
a EC_Sky-component on it. This component is now usable for every user and all changes on it will be replicated.
This synchronized sky plane component can also be edited by using environment editor (in world tools).

Currently Caelum must be disabled before these features can be used.

<b>Attributes</b>:
<ul>
<li> AssetReference: materialRef.
<div> Sky material reference.</div>
<li> AssetReferenceList : textureRefs.
<div>Sky texture references.</div>
<li> Quaternion : orientation
<div> Optional parameter to specify the orientation of the box. </div>
<li> float : distance
<div> Distance in world coordinates from the camera to each plane of the box. </div>
<li> bool : drawFirst
<div> If true, the box is drawn before all other geometry in the scene. </div>

</ul>
</table>
*/
    class EC_Sky : public IComponent
    {
        Q_OBJECT

    public:
        /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
        explicit EC_Sky(Framework *fw);

        virtual ~EC_Sky();

        /// Sky material reference
        DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, materialRef);
        Q_PROPERTY(AssetReference materialRef READ getmaterialRef WRITE setmaterialRef);

        /// Sky texture references.
        DEFINE_QPROPERTY_ATTRIBUTE(AssetReferenceList, textureRefs);
        Q_PROPERTY(AssetReferenceList textureRefs READ gettextureRefs WRITE settextureRefs);

        /// Distance in world coordinates from the camera to each plane of the box.
        DEFINE_QPROPERTY_ATTRIBUTE(float, distance);
        Q_PROPERTY(float distance READ getdistance WRITE setdistance);

        /// Optional parameter to specify the orientation of the box.
        DEFINE_QPROPERTY_ATTRIBUTE(Quaternion, orientation);
        Q_PROPERTY(Quaternion orientation READ getorientation WRITE setorientation);

         /// Defines that is sky drawn first
        DEFINE_QPROPERTY_ATTRIBUTE(bool, drawFirst);
        Q_PROPERTY(bool drawFirst READ getdrawFirst WRITE setdrawFirst);

        COMPONENT_NAME("EC_Sky", 10)
    public slots:

        /// View sky assets.
        void View(const QString &attributeName);
        /// Called If some of the attributes has been changed.
        void OnAttributeUpdated(IAttribute* attribute);

        /// Disables the sky box.
        void DisableSky();

        /// Called when texture asset has been downloaded.
        void OnTextureAssetLoaded(AssetPtr tex);

    private:
        void CreateSky();
        void SetTextures();

        QString lastMaterial_;
        float lastDistance_;
        bool lastDrawFirst_;
        Quaternion lastOrientation_;

        /// Renderer
        OgreRenderer::RendererWeakPtr renderer_;

        std::vector<AssetRefListenerPtr> textureAssets;
    };
}

#endif
