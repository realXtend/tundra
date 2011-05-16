// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "OgreRenderingModule.h"
#include "OgreMaterialAsset.h"
#include "FrameAPI.h"
#include "Entity.h"
#include "EC_Material.h"
#include "LoggingFunctions.h"
#include "AssetAPI.h"

EC_Material::EC_Material(Framework *fw) :
    IComponent(fw),
    parameters(this, "Parameters", QVariantList()),
    inputMat(this, "Input Material", ""),
    outputMat(this, "Output Material", "")
{
    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)),
            SLOT(OnAttributeUpdated(IAttribute*)));
}

EC_Material::~EC_Material()
{
}

void EC_Material::OnAttributeUpdated(IAttribute* attribute)
{
    if ((attribute == &parameters) || (attribute == &inputMat) || (attribute == &outputMat))
        ProcessMaterial();
}

void EC_Material::ProcessMaterial()
{
    AssetAPI* assetAPI = framework_->Asset();
    
    QString inputMatName = inputMat.Get();
    QString outputMatName = outputMat.Get();
    
    // If output material name contains ?, replace it with entity ID
    int questionMark = outputMatName.indexOf('?');
    if (questionMark >= 0)
    {
        Entity* parentEntity = GetParentEntity();
        if (parentEntity)
            outputMatName.replace(questionMark, 1, QString::number(parentEntity->GetId()));
    }
    // If outputmaterial name does not end in ".material", add it here, because otherwise the material will not be correctly requested by other components
    //if (!outputMatName.toLower().endsWith(".material"))
    //    outputMatName += ".material";
    
    
    AssetPtr srcAsset;
    AssetPtr destAsset;
    OgreMaterialAsset* srcMatAsset = 0;
    OgreMaterialAsset* destMatAsset = 0;
    
    /// \todo Empty input material name currently not supported. Implement getting the first submesh material from an EC_Mesh, or other components
    if (inputMatName.isEmpty())
        return;
    
    srcAsset = assetAPI->GetAsset(assetAPI->ResolveAssetRef("", inputMatName));
    srcMatAsset = dynamic_cast<OgreMaterialAsset*>(srcAsset.get());
    
    if (!outputMatName.isEmpty())
    {
        // If dest. asset does not exist, create it now
        destAsset = assetAPI->GetAsset(assetAPI->ResolveAssetRef("", outputMatName));
        if (!destAsset)
            destAsset = assetAPI->CreateNewAsset("OgreMaterial", outputMatName);
        
        destMatAsset = dynamic_cast<OgreMaterialAsset*>(destAsset.get());
        // Copy source material to destination
        if ((srcMatAsset) && (destMatAsset) && (srcMatAsset != destMatAsset) && (srcMatAsset->IsLoaded()))
            destMatAsset->CopyContent(srcAsset);
        else
            return;
    }
    else
    {
        // If dest. material is empty, apply in-place
        destMatAsset = srcMatAsset;
        if ((!destMatAsset) || (!destMatAsset->IsLoaded()))
            return;
    }
    
    // Apply parameters
    const QVariantList& params = parameters.Get();
    for (int i = 0; i < params.size(); ++i)
    {
        QString param = params[i].toString();
        QStringList parts = param.split('=');
        if (parts.size() == 2)
            destMatAsset->SetAttribute(parts[0], parts[1]);
    }
}
