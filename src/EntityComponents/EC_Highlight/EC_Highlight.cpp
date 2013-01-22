/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   EC_Highlight.cpp
    @brief  EC_Highlight enables visual highlighting effect for of scene entity. */

#include "DebugOperatorNew.h"
#include "EC_Highlight.h"

#include "Framework.h"
#include "AssetAPI.h"
#include "Entity.h"
#include "Scene/Scene.h"
#include "Renderer.h"
#include "OgreMaterialUtils.h"
#include "OgreMaterialAsset.h"
#include "OgreWorld.h"
#include "EC_Placeable.h"
#include "EC_Mesh.h"
#include "LoggingFunctions.h"
#include "AssetAPI.h"

#include <QTimer>

#include <OgreMaterial.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreEntity.h>
#include <OgreSubEntity.h>

#include "MemoryLeakCheck.h"

// Global flag for detecting conflicting applies from multiple highlights / preventing endless loops
static bool inApply = false;

EC_Highlight::EC_Highlight(Scene* scene) :
    IComponent(scene),
    visible(this, "Is visible", false),
    solidColor(this, "Solid color", Color(0.3f, 0.5f, 0.1f, 0.5f)),
    outlineColor(this, "Outline color", Color(1.0f, 1.0f, 1.0f, 0.5f)),
    reapplyPending_(false)
{
    if (scene)
        world_ = scene->GetWorld<OgreWorld>();
    
    connect(this, SIGNAL(ParentEntitySet()), SLOT(UpdateSignals()));
}

EC_Highlight::~EC_Highlight()
{
    Hide();
}

void  EC_Highlight::Show()
{
    if ((mesh_.expired()) || (world_.expired()))
        return;
    
    inApply = true;
    
    // Remove old materials first if they exist
    Hide();
    
    EC_Mesh* mesh = mesh_.lock().get();
    AssetAPI* assetAPI = framework->Asset();
    
    originalMaterials_.clear();

    // Clone all valid material assets that we can find from the mesh
    /// \todo Currently will clone the same material several times if used on several submeshes
    /// \todo What if the material is yet pending, or is not an asset (LitTextured)
    AssetReferenceList materialList = mesh->meshMaterial.Get();
    for(int i = 0; i < materialList.Size(); ++i)
    {
        if (!materialList[i].ref.isEmpty())
        {
            QString assetFullName = assetAPI->ResolveAssetRef("", materialList[i].ref);
            AssetPtr asset = assetAPI->GetAsset(assetFullName);
            if ((asset) && (asset->IsLoaded()) && (dynamic_cast<OgreMaterialAsset*>(asset.get())))
            {
                AssetPtr clone = asset->Clone(QString::fromStdString(world_.lock()->GetUniqueObjectName("EC_Highlight_Material")) + ".material");
                if (clone)
                {
                    OgreMaterialAsset* matAsset = dynamic_cast<OgreMaterialAsset*>(clone.get());
                    CreateHighlightToOgreMaterial(matAsset);

                    try
                    {
                        if ((mesh) && (mesh->GetEntity()) && (mesh->GetEntity()->getSubEntity(i)))
                            mesh->GetEntity()->getSubEntity(i)->setMaterial(matAsset->ogreMaterial);
                    }
                    catch(Ogre::Exception& e)
                    {
                        LogError("EC_Highlight::Show: Could not set material " + matAsset->Name() + " to subentity " + i + ":" + e.what());
                        continue;
                    }

                    materials_.push_back(clone);

                    // Store original ref to be restored in Hide()
                    originalMaterials_[i] = materialList[i].ref;
                }
            }
        }
    }
}

void EC_Highlight::Hide()
{
    if (!mesh_.expired())
    {
        // Restore mesh component's original materials to hide highlight effect.
        // Use AttributeChange::LocalOnly to ensure all editors will show the real refs.
        AssetAPI * assetAPI = framework->Asset();
        EC_Mesh* mesh = mesh_.lock().get();
        foreach(uint index, originalMaterials_.keys())
        {
            QString fullName = assetAPI->ResolveAssetRef("", originalMaterials_[index]);
            AssetPtr asset = assetAPI->GetAsset(fullName);
            if ((asset) && (asset->IsLoaded()) && (dynamic_cast<OgreMaterialAsset*>(asset.get())))
            {
                OgreMaterialAsset *matAsset = dynamic_cast<OgreMaterialAsset*> (asset.get());
                try
                {
                    if ((mesh) && (mesh->GetEntity()) && (mesh->GetEntity()->getSubEntity(index)))
                        mesh->GetEntity()->getSubEntity(index)->setMaterial(matAsset->ogreMaterial);
                }
                catch(Ogre::Exception& e)
                {
                    LogError("EC_Highlight::Hide: Could not set material " + matAsset->Name() + " to subentity " + index + ":" + e.what());
                    continue;
                }
            }
        }
        originalMaterials_.clear();
    }
    
    // Destroy all the highlight materials
    AssetAPI* assetAPI = framework->Asset();
    for (unsigned i = 0; i < materials_.size(); ++i)
        assetAPI->ForgetAsset(materials_[i], false);
    materials_.clear();
}

bool EC_Highlight::IsVisible() const
{
    return materials_.size() > 0;
}

void EC_Highlight::UpdateSignals()
{
    Entity* parent = ParentEntity();
    if (parent)
    {
        // Connect to ComponentAdded/Removed signals of the parent entity, so we can check when the mesh component gets added or removed
        connect(parent, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), SLOT(AcquireMesh()));
        connect(parent, SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)), SLOT(OnComponentRemoved(IComponent*, AttributeChange::Type)));
        // Also try to acquire the mesh component immediately
        AcquireMesh();
    }
}

void EC_Highlight::OnComponentRemoved(IComponent* component, AttributeChange::Type change)
{
    if (component == mesh_.lock().get())
    {
        Hide();
        mesh_.reset();
    }
}

void EC_Highlight::AcquireMesh()
{
    // Return if already acquired
    if (!mesh_.expired())
        return;
    
    mesh_ = ParentEntity()->GetComponent<EC_Mesh>();
    
    EC_Mesh* mesh = mesh_.lock().get();
    if (mesh)
    {
        // Connect to mesh & material change signals so we can reapply the highlight
        connect(mesh, SIGNAL(MeshChanged()), this, SLOT(TriggerReapply()), Qt::UniqueConnection);
        connect(mesh, SIGNAL(MaterialChanged(uint, const QString &)), this, SLOT(TriggerReapply()), Qt::UniqueConnection);
        
        // Also show right now if highlight active
        if (visible.Get())
            Show();
    }
}

void EC_Highlight::AttributesChanged()
{
    if (visible.ValueChanged())
    {
        if (visible.Get())
            Show();
        else
            Hide();
    }
    
    if (solidColor.ValueChanged() || outlineColor.ValueChanged())
        ApplyHighlightColors();
}

void EC_Highlight::TriggerReapply()
{
    // Disregard signal if in the middle of applying highlight
    if (inApply)
        return;
    
    // We might get multiple requests to reapply, but actually execute on the next frame, so that we don't do needless recreation of materials
    if ((!reapplyPending_) && (visible.Get()))
    {
        reapplyPending_ = true;
        QTimer::singleShot(0, this, SLOT(ReapplyHighlight()));
    }
}

void EC_Highlight::ReapplyHighlight()
{
    if (visible.Get())
        Show();
    reapplyPending_ = false;
}

void EC_Highlight::CreateHighlightToOgreMaterial(OgreMaterialAsset* mat)
{
    if (!mat)
        return;
    
    unsigned numTech = mat->GetNumTechniques();
    for (unsigned i = 0; i < numTech; ++i)
    {
        int pass1 = mat->CreatePass(i);
        int pass2 = mat->CreatePass(i);
        
        // Setting the shaders requires the SolidAmbient.material to exist in the resource groups, so that the shaders exist
        mat->SetLighting(i, pass1, false);
        mat->SetSceneBlend(i, pass1, Ogre::SBT_TRANSPARENT_ALPHA);
        mat->SetDepthWrite(i, pass1, false);
        mat->SetDepthBias(i, pass1, 1.f);
        mat->SetHardwareCullingMode(i, pass1, Ogre::CULL_NONE);
        mat->SetVertexShader(i, pass1, "SolidAmbientVP");
        mat->SetPixelShader(i, pass1, "SolidAmbientFP");
        mat->SetAmbientColor(i, pass1, solidColor.Get());

        mat->SetLighting(i, pass2, false);
        mat->SetSceneBlend(i, pass2, Ogre::SBT_TRANSPARENT_ALPHA);
        mat->SetDepthWrite(i, pass2, false);
        mat->SetDepthBias(i, pass2, 2.f);
        mat->SetHardwareCullingMode(i, pass2, Ogre::CULL_NONE);
        mat->SetVertexShader(i, pass2, "SolidAmbientVP");
        mat->SetPixelShader(i, pass2, "SolidAmbientFP");
        mat->SetAmbientColor(i, pass2, outlineColor.Get());
        mat->SetPolygonMode(i, pass2, Ogre::PM_WIREFRAME);
    }
}

void EC_Highlight::ApplyHighlightColors()
{
    for (unsigned i = 0; i < materials_.size(); ++i)
    {
        OgreMaterialAsset* mat = dynamic_cast<OgreMaterialAsset*>(materials_[i].get());
        unsigned numTech = mat->GetNumTechniques();
        for (unsigned i = 0; i < numTech; ++i)
        {
            unsigned numPasses = mat->GetNumPasses(i);
            // Modify the diffuse color of the last 2 passes
            // (this should be a highlight material that we added the 2 passes to)
            if (numPasses >= 2)
            {
                mat->SetAmbientColor(i, numPasses - 2, solidColor.Get());
                mat->SetAmbientColor(i, numPasses - 1, outlineColor.Get());
            }
        }
    }
}
