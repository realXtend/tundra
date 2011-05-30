/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Billboard.cpp
 *  @brief  EC_Billboard shows a billboard (3D sprite) that is attached to an entity.
 *  @note   The entity must have EC_Placeable component available in advance.
 */

#include "EC_Billboard.h"
#include "IModule.h"
#include "Renderer.h"
#include "EC_Placeable.h"
#include "Entity.h"
#include "OgreMaterialAsset.h"
#include "OgreConversionUtils.h"
#include "OgreWorld.h"
#include "Framework.h"
#include "OgreRenderingModule.h"
#include "Scene.h"
#include "LoggingFunctions.h"

#include <OgreBillboardSet.h>
#include <OgreTextureManager.h>
#include <OgreResource.h>
#include <OgreBillboard.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>
#include <OgreMaterialManager.h>

#include <QTimer>

EC_Billboard::EC_Billboard(Scene* scene) :
    IComponent(scene),
    billboardSet_(0),
    billboard_(0),
    attached_(false),
    materialRef(this, "Material ref"),
    position(this, "Position", Vector3df::ZERO),
    width(this, "Size X", 1.0f),
    height(this, "Size Y", 1.0f),
    rotation(this, "Rotation", 0.0f),
    show(this, "Show billboard", true),
    autoHideTime(this, "Auto-hide time", -1.0f)
{
    if (scene)
        world_ = scene->GetWorld<OgreWorld>();
    materialAsset_ = AssetRefListenerPtr(new AssetRefListener());
    connect(materialAsset_.get(), SIGNAL(Loaded(AssetPtr)), this, SLOT(OnMaterialAssetLoaded(AssetPtr)), Qt::UniqueConnection);
    connect(materialAsset_.get(), SIGNAL(TransferFailed(IAssetTransfer*, QString)), this, SLOT(OnMaterialAssetFailed(IAssetTransfer*, QString)), Qt::UniqueConnection);
    
    connect(this, SIGNAL(ParentEntitySet()), SLOT(OnParentEntitySet()));
    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(OnAttributeUpdated(IAttribute*)));
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
    Ogre::SceneManager* scene = world->GetSceneManager();
    if (!scene)
        return;

    if (!billboardSet_)
        billboardSet_ = scene->createBillboardSet(world->GetUniqueObjectName("EC_Billboard"), 1);
    
    // Remove old billboard if it existed
    if (billboard_)
    {
        billboardSet_->removeBillboard(billboard_);
        billboard_ = 0;
    }
    
    billboard_ = billboardSet_->createBillboard(OgreRenderer::ToOgreVector3(position.Get()));
    billboard_->setDimensions(width.Get(), height.Get());
    billboard_->setRotation(Ogre::Radian(Ogre::Degree(rotation.Get())));
}

void EC_Billboard::UpdateBillboardProperties()
{
    if (billboard_)
    {
        billboard_->setPosition(OgreRenderer::ToOgreVector3(position.Get()));
        billboard_->setDimensions(width.Get(), height.Get());
        billboard_->setRotation(Ogre::Radian(Ogre::Degree(rotation.Get())));
    }
}

void EC_Billboard::DestroyBillboard()
{
    OgreWorldPtr world = world_.lock();
    if (!world)
    {
        if ((billboard_) || (billboardSet_))
            LogError("EC_Billboard: World has expired, skipping uninitialization!");
        return;
    }
    
    DetachBillboard();
    
    if ((billboard_) && (billboardSet_))
    {
        billboardSet_->removeBillboard(billboard_);
        billboard_ = 0;
    }
    if (billboardSet_)
    {
        world->GetSceneManager()->destroyBillboardSet(billboardSet_);
        billboardSet_ = 0;
    }
}


void EC_Billboard::Show()
{
    AttachBillboard();
    
    // Optionally autohide
    float hideTime = autoHideTime.Get();
    if (hideTime >= 0.0f)
        QTimer::singleShot((int)(hideTime*1000), this, SLOT(Hide()));
}

void EC_Billboard::Hide()
{
    DetachBillboard();
}

void EC_Billboard::AttachBillboard()
{
    if ((placeable_) && (!attached_) && (billboardSet_))
    {
        placeable_->GetSceneNode()->attachObject(billboardSet_);
        attached_ = true;
    }
}

void EC_Billboard::DetachBillboard()
{
    if ((placeable_) && (attached_) && (billboardSet_))
    {
        placeable_->GetSceneNode()->detachObject(billboardSet_);
        attached_ = false;
    }
}

void EC_Billboard::OnAttributeUpdated(IAttribute *attribute)
{
    if ((attribute == &position) || (attribute == &width) || (attribute == &height) || (attribute == &rotation))
    {
        if (billboard_)
            UpdateBillboardProperties();
        else
            CreateBillboard();
    }
    
    if (attribute == &materialRef)
    {
        if (!ViewEnabled())
            return;
        
        materialAsset_->HandleAssetRefChange(&materialRef);
    }
    
    if (attribute == &show)
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
            asset->Name().toStdString() + "\", but downloaded asset was not of type OgreMaterialAsset!");
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

