// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "OgreModuleApi.h"
#include "IComponent.h"
#include "IAttribute.h"
#include "Math/Quat.h"
#include "AssetReference.h"
#include "OgreModuleFwd.h"
#include "AssetRefListener.h"

/// Makes the entity a sky.
/** <table class="header">
    <tr>
    <td>
    <h2>Sky</h2>

    Makes the entity a sky.

    Registered by OgreRenderingModule.

    <b>Attributes</b>:
    <ul>
    <li>AssetReference: materialRef
    <div> @copydoc materialRef </div>
    <li>Quat: orientation
    <div> @copydoc orientation </div>
    <li>float: distance
    <div> @copydoc distance </div>
    <li>bool : drawFirst
    <div> @copydoc drawFirst </div>
    </ul>
    </table> */
class OGRE_MODULE_API EC_Sky : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_Sky", 10)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_Sky(Scene* scene);

    virtual ~EC_Sky();

    /// Sky material reference.
    /** Material defines how the sky looks. Dy default and typically a material with cubic texture is used,
        but also f.ex. a simpler material can be used to paint the sky with solid color. */
    Q_PROPERTY(AssetReference materialRef READ getmaterialRef WRITE setmaterialRef);
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, materialRef);

    /// Sky texture references.
    /** @deprecated Use materialRef to set the appearance of the sky.
        @todo Remove. */
    Q_PROPERTY(AssetReferenceList textureRefs READ gettextureRefs WRITE settextureRefs);
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReferenceList, textureRefs);

    /// Distance in world coordinates from the camera to each plane of the box.
    Q_PROPERTY(float distance READ getdistance WRITE setdistance);
    DEFINE_QPROPERTY_ATTRIBUTE(float, distance);

    /// Optional parameter to specify the orientation of the box.
    Q_PROPERTY(Quat orientation READ getorientation WRITE setorientation);
    DEFINE_QPROPERTY_ATTRIBUTE(Quat, orientation);

     /// Defines is sky drawn first.
    Q_PROPERTY(bool drawFirst READ getdrawFirst WRITE setdrawFirst);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, drawFirst);

    /// Is the sky enabled.
    Q_PROPERTY(bool enabled READ getenabled WRITE setenabled);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, enabled);

private:
    void AttributesChanged();
    void Update();

    OgreWorldWeakPtr ogreWorld;
    AssetRefListenerPtr materialAsset;
    std::vector<AssetRefListenerPtr> textureAssets;
    QString currentMaterial; ///< Ogre resource name for the currently used material.

private slots:
    void OnMaterialAssetLoaded(AssetPtr mat);
    /// @deprecated This will be removed when textureRefs attribute is removed.
    void OnTextureAssetLoaded(AssetPtr tex);
};
