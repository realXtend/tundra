/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   EC_Billboard.cpp
    @brief  EC_Billboard shows a billboard (3D sprite) that is attached to an entity. */

#include "StableHeaders.h"
#define MATH_OGRE_INTEROP
#include "DebugOperatorNew.h"

#include "EC_Billboard.h"
#include "Renderer.h"
#include "EC_Placeable.h"
#include "OgreMaterialAsset.h"
#include "OgreWorld.h"

#include "Entity.h"
#include "Framework.h"
#include "Scene/Scene.h"
#include "LoggingFunctions.h"

#include <OgreBillboardSet.h>
#include <OgreTextureManager.h>
#include <OgreResource.h>
#include <OgreBillboard.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreMaterialManager.h>

#include <QTimer>

#include "MemoryLeakCheck.h"

EC_Billboard::EC_Billboard(Scene* scene) :
    IComponent(scene),
    billboardSet_(0),
    billboard_(0),
    attached_(false),
    materialRef(this, "Material ref"),
    position(this, "Position", float3::zero),
    width(this, "Size X", 1.0f),
    height(this, "Size Y", 1.0f),
    rotation(this, "Rotation", 0.0f),
    show(this, "Show billboard", true)
{
    if (scene)
        world_ = scene->GetWorld<OgreWorld>();

    materialAsset_ = AssetRefListenerPtr(new AssetRefListener());
    connect(materialAsset_.get(), SIGNAL(Loaded(AssetPtr)), this, SLOT(OnMaterialAssetLoaded(AssetPtr)), Qt::UniqueConnection);
    connect(materialAsset_.get(), SIGNAL(TransferFailed(IAssetTransfer*, QString)), this, SLOT(OnMaterialAssetFailed(IAssetTransfer*, QString)), Qt::UniqueConnection);
    
    connect(this, SIGNAL(ParentEntitySet()), SLOT(OnParentEntitySet()));
}

EC_Billboard::~EC_Billboard()
{
    DestroyBillboard();
}

void EC_Billboard::OnParentEntitySet()
{
    Entity* parent = ParentEntity();
    if (!parent)
        return;
    
    connect(parent, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), this, SLOT(CheckForPlaceable()));
    connect(parent, SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)), SLOT(OnComponentRemoved(IComponent*, AttributeChange::Type)));
    
    CreateBillboard();
    CheckForPlaceable();
}

void EC_Billboard::CheckForPlaceable()
{
    if (placeable_)
        return; // Already assigned
    
    placeable_ = ParentEntity()->GetComponent<EC_Placeable>();
    if ((placeable_) && (show.Get()))
        Show();
}

void EC_Billboard::OnComponentRemoved(IComponent* component, AttributeChange::Type change)
{
    if (component == placeable_.get())
    {
        DetachBillboard();
        placeable_.reset();
    }
}

void EC_Billboard::CreateBillboard()
{
    if (!ViewEnabled())
        return;
    OgreWorldPtr world = world_.lock();
    if (!world)
        return;
    Ogre::SceneManager* scene = world->OgreSceneManager();
    if (!scene)
        return;

    if (!billboardSet_)
    {
        billboardSet_ = scene->createBillboardSet(world->GetUniqueObjectName("EC_Billboard"), 1);
        billboardSet_->Ogre::MovableObject::setUserAny(Ogre::Any(static_cast<IComponent *>(this)));
        billboardSet_->Ogre::Renderable::setUserAny(Ogre::Any(static_cast<IComponent *>(this)));
    }
    
    // Remove old billboard if it existed
    if (billboard_)
    {
        billboardSet_->removeBillboard(billboard_);
        billboard_ = 0;
    }
    
    billboard_ = billboardSet_->createBillboard(position.Get());
    UpdateBillboardProperties();
}

void EC_Billboard::UpdateBillboardProperties()
{
    if (billboard_)
    {
        billboard_->setPosition(position.Get());
        billboard_->setDimensions(width.Get(), height.Get());
        // Bug in OGRE: It does not use the actual billboard bounding box size in the computation, but instead guesses it from the "default size", so
        // also set the default size to the size of our billboard (fortunately we only have one billboard in the set)
        billboardSet_->setDefaultDimensions(width.Get()*0.5f, height.Get()*0.5f); // Another bug in OGRE: It computes the billboard AABB padding to 2*width and 2*height, not width & height.
        billboard_->setRotation(Ogre::Radian(Ogre::Degree(rotation.Get())));
        billboardSet_->_updateBounds(); // Documentation of Ogre::BillboardSet says the update is never called automatically, so now do it manually.
    }
}

void EC_Billboard::DestroyBillboard()
{
    OgreWorldPtr world = world_.lock();
    if (!world)
    {
        if (billboard_ || billboardSet_)
            LogError("EC_Billboard: World has expired, skipping uninitialization!");
        return;
    }
    
    DetachBillboard();
    
    if (billboard_ && billboardSet_)
    {
        billboardSet_->removeBillboard(billboard_);
        billboard_ = 0;
    }
    if (billboardSet_)
    {
        world->OgreSceneManager()->destroyBillboardSet(billboardSet_);
        billboardSet_ = 0;
    }
}


void EC_Billboard::Show()
{
    AttachBillboard();
}

void EC_Billboard::Hide()
{
    DetachBillboard();
}

void EC_Billboard::AttachBillboard()
{
    if (placeable_ && !attached_ && billboardSet_)
    {
        placeable_->GetSceneNode()->attachObject(billboardSet_);
        attached_ = true;
    }
}

void EC_Billboard::DetachBillboard()
{
    if (placeable_ && attached_ && billboardSet_)
    {
        placeable_->GetSceneNode()->detachObject(billboardSet_);
        attached_ = false;
    }
}

// Matches OgrePrerequisites.h:62 (OGRE_VERSION #define)
#define OGRE_VER(major, minor, patch) (((major) << 16) | ((minor) << 8) | (patch))

void EC_Billboard::AttributesChanged()
{
    if (!ViewEnabled())
        return;
    if (position.ValueChanged() || width.ValueChanged()  || height.ValueChanged() || rotation.ValueChanged())
    {
        if (billboard_)
            UpdateBillboardProperties();
        else
            CreateBillboard();
    }
    if (materialRef.ValueChanged())
    {
        // In the case of empty ref setting it to the ref listener will
        // make sure we don't get called to OnMaterialAssetLoaded. This would happen 
        // if something touches the previously set material in the asset system (eg. reload).
        materialAsset_->HandleAssetRefChange(&materialRef);

        try
        {
            // If we previously had a material set and its not removed, update the visuals from ogre.
#if OGRE_VERSION >= OGRE_VER(1,7,3)
            if (materialRef.Get().ref.isEmpty() && billboardSet_)
                billboardSet_->setMaterial(Ogre::MaterialPtr());
#endif
        }
        catch (Ogre::Exception &e)
        {
            LogError("EC_Billboard: Failed to reset visuals after material was removed: " + std::string(e.what()));
        }
    }
    if (show.ValueChanged())
    {
        if (show.Get())
            Show();
        else
            Hide();
    }
}

void EC_Billboard::OnMaterialAssetLoaded(AssetPtr asset)
{
    OgreMaterialAsset *ogreMaterial = dynamic_cast<OgreMaterialAsset*>(asset.get());
    if (!ogreMaterial)
    {
        LogError("OnMaterialAssetLoaded: Material asset load finished for asset \"" +
            asset->Name() + "\", but downloaded asset was not of type OgreMaterialAsset!");
        return;
    }

    Ogre::MaterialPtr material = ogreMaterial->ogreMaterial;
    
    if (billboardSet_)
        billboardSet_->setMaterialName(material->getName());
}

void EC_Billboard::OnMaterialAssetFailed(IAssetTransfer* transfer, QString reason)
{
    if (billboardSet_)
        billboardSet_->setMaterialName("AssetLoadError");
}
