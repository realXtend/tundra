// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IComponent.h"
#include "CoreDefines.h"
#include "OgreModuleApi.h"
#include "AssetRefListener.h"
#include "OgreModuleFwd.h"

/// Ogre material modifier component
/**
<table class="header">
<tr>
<td>
<h2>Material</h2>
Modifies an existing material asset by a parameter list and outputs it as another asset.
This allows network-replicated changes to material assets.
Registered by OgreRenderer::OgreRenderingModule.

<b>Attributes</b>:
<ul>
<li>QVariantList : parameters
<div> @copydoc parameters </div>
<li>QString : inputMat
<div> @copydoc inputMat </div>
<li>QString : outputMat
<div> @copydoc outputMat </div>
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>...
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

</table>
*/
class OGRE_MODULE_API EC_Material : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_Material", 31)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_Material(Scene* scene);

    virtual ~EC_Material();

    /// The parameters to apply.
    Q_PROPERTY(QVariantList parameters READ getparameters WRITE setparameters);
    DEFINE_QPROPERTY_ATTRIBUTE(QVariantList, parameters);

    /// Input material asset reference. Can also be a submesh number in the same entity's EC_Mesh, or empty to use submesh 0 material.
    Q_PROPERTY(QString inputMat READ getinputMat WRITE setinputMat);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, inputMat);

    /// Output material asset.
    Q_PROPERTY(QString outputMat READ getoutputMat WRITE setoutputMat);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, outputMat);

private slots:
    
    /// Parent entity has been set. Check for existence of EC_Mesh
    void OnParentEntitySet();
    
    /// Component has been added to the entity. Check if it is EC_Mesh
    void OnComponentAdded(IComponent* component, AttributeChange::Type change);
    
    /// Attributes of EC_Mesh component have changed. Check for material change
    void OnMeshAttributeUpdated(IAttribute* attribute);
    
    /// Input asset has been successfully loaded.
    void OnMaterialAssetLoaded(AssetPtr material);

signals:
    void AppliedOutputMaterial(Entity *entity, const QString &meshCompName, const int index, const QString &material);
    
private:
    void AttributesChanged();

    /// Return the submesh number to use from EC_Mesh, or -1 if not using the EC_Mesh's material
    int GetSubmeshNumber() const;

    /// Return the input material name to use in asset request
    QString GetInputMaterialName() const;

    /// Attributes have changed. Request input material.
    void CheckForInputMaterial();
    
    /// Apply parameters to the output material. Requires loaded input material.
    void ApplyParameters(OgreMaterialAsset* srcMatAsset);
    
    /// Ref listener for the input material asset
    AssetRefListenerPtr materialAsset;
};

