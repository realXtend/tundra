// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "IComponent.h"
#include "IAttribute.h"
#include "Quaternion.h"
#include "AssetReference.h"
#include "OgreModuleFwd.h"
#include "AssetRefListener.h"

/// Makes the entity a sky.
/**
<table class="header">
<tr>
<td>
<h2>Makes the entity a sky.</h2>

Registered by Enviroment::EnvironmentModule. 

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
    explicit EC_Sky(Scene* scene);

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

    /// Ogre scene
    OgreWorldWeakPtr world_;

    std::vector<AssetRefListenerPtr> textureAssets;
};
