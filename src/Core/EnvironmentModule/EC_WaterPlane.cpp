// For conditions of distribution and use, see copyright notice in LICENSE

#include "Math/MathFwd.h"
#define MATH_OGRE_INTEROP
#include "DebugOperatorNew.h"

#include "EC_WaterPlane.h"
#include "EC_Fog.h"
#include "EC_EnvironmentLight.h"

#include "Entity.h"
#include "EC_Placeable.h"
#include "IAttribute.h"
#include "AttributeMetadata.h"
#include "Renderer.h"
#include "Scene/Scene.h"
#include "OgreWorld.h"
#include "LoggingFunctions.h"
#include "FrameAPI.h"
#include "Math/MathFunc.h"
#include "Profiler.h"
#include "OgreMaterialAsset.h"
#include "AssetRefListener.h"

#include <Ogre.h>

#include "MemoryLeakCheck.h"

EC_WaterPlane::EC_WaterPlane(Scene* scene) :
    IComponent(scene),
    INIT_ATTRIBUTE_VALUE(xSize, "X-size", 5000),
    INIT_ATTRIBUTE_VALUE(ySize, "Y-size", 5000),
    INIT_ATTRIBUTE_VALUE(depth, "Depth", 10000),
    INIT_ATTRIBUTE_VALUE(position, "Position", float3::zero),
    INIT_ATTRIBUTE_VALUE(rotation, "Rotation", Quat::identity),
    INIT_ATTRIBUTE_VALUE(scaleUfactor, "U factor", 0.0002f),
    INIT_ATTRIBUTE_VALUE(scaleVfactor, "V factor", 0.0002f),
    INIT_ATTRIBUTE_VALUE(xSegments, "Segments in x", 10),
    INIT_ATTRIBUTE_VALUE(ySegments, "Segments in y", 10),
    INIT_ATTRIBUTE_VALUE(materialName, "Material", QString("Ocean")),
    INIT_ATTRIBUTE(materialRef, "Material ref"),
    INIT_ATTRIBUTE_VALUE(fogColor, "Fog color", Color(0.2f,0.4f,0.35f,1.0f)),
    INIT_ATTRIBUTE_VALUE(fogStartDistance, "Fog start dist.", 100.f),
    INIT_ATTRIBUTE_VALUE(fogEndDistance, "Fog end dist.", 2000.f),
    INIT_ATTRIBUTE_VALUE(fogMode, "Fog mode", 3),
    INIT_ATTRIBUTE_VALUE(fogExpDensity, "Fog exponential density", 0.001f),
    entity_(0),
    node_(0),
    attached_(false),
    attachedToRoot_(false),
    cameraInsideWaterCube(false)
{
    static AttributeMetadata fogModeMetadata, segmentMetadata;
    static bool metadataInitialized = false;
    if(!metadataInitialized)
    {
        fogModeMetadata.enums[Ogre::FOG_NONE] = "NoFog";
        fogModeMetadata.enums[Ogre::FOG_EXP] = "Exponential";
        fogModeMetadata.enums[Ogre::FOG_EXP2] = "ExponentiallySquare";
        fogModeMetadata.enums[Ogre::FOG_LINEAR] = "Linear";
        segmentMetadata.minimum = "1"; // Exception spam from Ogre when < 1, crash when negative.
        metadataInitialized = true;
    }

    fogMode.SetMetadata(&fogModeMetadata);
    xSegments.SetMetadata(&segmentMetadata);
    ySegments.SetMetadata(&segmentMetadata);

    if (scene)
        world_ = scene->Subsystem<OgreWorld>();
    OgreWorldPtr world = world_.lock();
    if (world)
        node_ = world->OgreSceneManager()->createSceneNode(world->GetUniqueObjectName("EC_WaterPlane_Root"));

    connect(this, SIGNAL(ParentEntitySet()), this, SLOT(Create()));

    materialAsset = MAKE_SHARED(AssetRefListener);
    connect(materialAsset.get(), SIGNAL(Loaded(AssetPtr)), SLOT(OnMaterialAssetLoaded(AssetPtr)));
    // If there exist placeable copy its position for default position and rotation.
   
    /*
    EC_Placeable* placeable = dynamic_cast<EC_Placeable*>(FindPlaceable().get());
    if (placeable != 0)
    {
        float3 vec = placeable->GetPosition();
        position.Set(vec,AttributeChange::Default);
   
        Quaternion rot =placeable->GetOrientation();
        rotation.Set(rot, AttributeChange::Default);
        ComponentChanged(AttributeChange::Default);
    }
    */
}

EC_WaterPlane::~EC_WaterPlane()
{
    RemoveWaterPlane();
    RestoreFog();

    if (node_ && !world_.expired())
        world_.lock()->OgreSceneManager()->destroySceneNode(node_);
    node_ = 0;
}

void EC_WaterPlane::Create()
{
    if (!ViewEnabled())
        return;
    
    CreateWaterPlane();
    
    // Parent entity has set.
    // Has parent a placeable?
    EC_Placeable* placeable = dynamic_cast<EC_Placeable*>(FindPlaceable().get());
    if (placeable && attached_) // Are we currently attached?
    {
        // Now there might be that we are attached to OgreRoot not to placeable node.
        DetachEntity();
        AttachEntity();
    }

    connect(parentEntity,SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)),
        SLOT(ComponentAdded(IComponent*, AttributeChange::Type)));
    connect(parentEntity,SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)),
        SLOT(ComponentRemoved(IComponent*, AttributeChange::Type)));
}

void EC_WaterPlane::ComponentAdded(IComponent* component, AttributeChange::Type type)
{
    if (component->TypeId() == EC_Placeable::TypeIdStatic())
    {
        DetachEntity();

        EC_Placeable* placeable = static_cast<EC_Placeable* >(component);
        if (placeable == 0)
            return;
        if (entity_ == 0)
            return;

        try
        {
            Ogre::SceneNode* node = placeable->OgreSceneNode();
            node->addChild(node_);
            node_->attachObject(entity_);
            node_->setVisible(true);
            attached_ = true;
        }
        catch(const Ogre::Exception &e)
        {
            LogError("EC_WaterPlane::ComponentRemoved: exception caught: " + std::string(e.what()));
        }
    }
}

void EC_WaterPlane::ComponentRemoved(IComponent* component, AttributeChange::Type type)
{
    if (world_.expired())
        return;
    
    if (component->TypeId() == EC_Placeable::TypeIdStatic())
    {
        DetachEntity();

        try
        {
            // Attach entity directly to Ogre root.
            node_->attachObject(entity_);
            world_.lock()->OgreSceneManager()->getRootSceneNode()->addChild(node_);
            node_->setVisible(true);
            attachedToRoot_ = true;
            attached_ = true;
        }
        catch(const Ogre::Exception &e)
        {
            LogError("EC_WaterPlane::ComponentRemoved: exception caught: " + std::string(e.what()));
        }
    }
}

void EC_WaterPlane::Update()
{
    OgreWorldPtr world = world_.lock();
    if (!world)
        return;
    PROFILE(EC_WaterPlane_Update)
    bool cameraWasInsideWaterCube = cameraInsideWaterCube;
    cameraInsideWaterCube = IsCameraInsideWaterCube();
    if (!cameraWasInsideWaterCube && cameraInsideWaterCube)
        SetUnderwaterFog();
    else if (cameraWasInsideWaterCube && !cameraInsideWaterCube)
        RestoreFog();
}

float3 EC_WaterPlane::GetPointOnPlane(const float3 &point) const 
{
    if (node_ == 0)
        return float3::nan;

    Ogre::Matrix4 worldTM = node_->_getFullTransform(); // local->world
    Ogre::Matrix4 inv = worldTM.inverse(); // world->local
    Ogre::Vector4 local = inv * Ogre::Vector4(point.x, point.y, point.z, 1.f);

    local.y = 0;
    Ogre::Vector4 world = worldTM * local;
    return float3(world.x, world.y, world.z);
}

float EC_WaterPlane::GetDistanceToWaterPlane(const float3& point) const
{
    if (!node_)
        return 0.f;
    return point.y - GetPointOnPlane(point).y;
}

bool EC_WaterPlane::IsTopOrBelowWaterPlane(const float3& point) const
{
    if (!node_)
        return false;

    Ogre::Vector3 local = node_->_getDerivedOrientation().Inverse() * (point - node_->_getDerivedPosition()) / node_->_getDerivedScale();

    int x = xSize.Get(), y = ySize.Get();

    float xMax = x*0.5;
    float yMax = y*0.5;
    float xMin = -x*0.5;
    float yMin = -y*0.5;

    if (local.x > xMin && local.x < xMax && local.y > yMin && local.y < yMax)
        return true;

    return false;
}

bool EC_WaterPlane::IsPointInsideWaterCube(const float3& point) const
{
    if (entity_ == 0 || !ViewEnabled())
        return false;

    if (IsTopOrBelowWaterPlane(point))
    {
        float d = GetDistanceToWaterPlane(point);
        if (d < 0 && depth.Get() >= fabs(d))
            return true;
    }

    return false;
}

bool EC_WaterPlane::IsCameraInsideWaterCube()
{
    if (entity_ == 0 || !ViewEnabled())
        return false;

    Ogre::Camera *camera = world_.lock()->Renderer()->MainOgreCamera();
    if (!camera)
        return false;

    float3 posCamera = camera->getDerivedPosition();
    if (IsTopOrBelowWaterPlane(posCamera))
    {
        float d = GetDistanceToWaterPlane(posCamera);
        if (d < 0 && depth.Get() >= fabs(d))
            return true;
    }

    return false;
}

void EC_WaterPlane::CreateWaterPlane()
{
    if (!ViewEnabled())
        return;

    if (entity_)
        RemoveWaterPlane();

    OgreWorldPtr world = world_.lock();
    if (world && node_)
    {
        int x = xSize.Get();
        int y = ySize.Get();
        float uTile =  scaleUfactor.Get() * x; // Default x-size 5000 --> uTile 1.0
        float vTile =  scaleVfactor.Get() * y;
        // Clamp xSegments and ySegments: Exception spam from Ogre when < 1, crash when negative.
        int xSeg = Clamp(xSegments.Get(), 1, std::numeric_limits<int>::max());
        int ySeg = Clamp(ySegments.Get(), 1, std::numeric_limits<int>::max());
        try
        {
            Ogre::MeshPtr mesh = Ogre::MeshManager::getSingleton().createPlane(Name().toStdString(),
                Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::Plane(Ogre::Vector3::UNIT_Y, 0),
                x, y, xSeg, ySeg, true, 1, uTile, vTile, Ogre::Vector3::UNIT_X);

            entity_ = world->OgreSceneManager()->createEntity(world->GetUniqueObjectName("EC_WaterPlane_entity"), Name().toStdString().c_str());
            entity_->setMaterialName(currentMaterial.toStdString());
            entity_->setCastShadows(false);
            // Tries to attach entity, if there is no EC_Placeable available, it will not attach the object.
            AttachEntity();

            connect(framework->Frame(), SIGNAL(Updated(float)), SLOT(Update()));
        }
        catch(const Ogre::Exception &e)
        {
            LogError("EC_WaterPlane::CreateWaterPlane: exception caught: " + std::string(e.what()));
        }
    }
}

void EC_WaterPlane::RemoveWaterPlane()
{
    if (world_.expired() || !entity_ || !ViewEnabled())
        return;

    DetachEntity();

    try
    {
        world_.lock()->OgreSceneManager()->destroyEntity(entity_);
        entity_ = 0;

        Ogre::MeshManager::getSingleton().remove(Name().toStdString());
    }
    catch(const Ogre::Exception &e)
    {
        LogError("EC_WaterPlane::RemoveWaterPlane: exception caught: " + std::string(e.what()));
    }

    disconnect(framework->Frame(), SIGNAL(Updated(float)), this, SLOT(Update()));
}

void EC_WaterPlane::OnMaterialAssetLoaded(const AssetPtr &mat)
{
    OgreMaterialAssetPtr material = dynamic_pointer_cast<OgreMaterialAsset>(mat);
    if (material && !material->ogreMaterial.isNull())
    {
        currentMaterial = material->ogreAssetName;
        UpdateMaterial();
    }
}

void EC_WaterPlane::AttributesChanged()
{
    if (xSize.ValueChanged() || ySize.ValueChanged() || scaleUfactor.ValueChanged() || scaleVfactor.ValueChanged())
        CreateWaterPlane();

    if (xSegments.ValueChanged() || ySegments.ValueChanged())
        CreateWaterPlane();

    if (position.ValueChanged())
        SetPosition();

    if (rotation.ValueChanged())
        SetOrientation();

    if (materialName.ValueChanged() && !materialName.Get().isEmpty())
    {
        LogWarning("EC_WaterPlane:: materialName attribute will be deprecated. Use materialRef instead.");
        currentMaterial = materialName.Get();
        UpdateMaterial();
    }

    if (fogColor.ValueChanged() || fogStartDistance.ValueChanged() || fogEndDistance.ValueChanged() ||
        fogMode.ValueChanged() || fogExpDensity.ValueChanged())
    {
        if (IsCameraInsideWaterCube()) // Apply fog immediately only if the camera is within the water cube.
            SetUnderwaterFog();
    }

    if (materialRef.ValueChanged())
        materialAsset->HandleAssetRefChange(framework->Asset(), materialRef.Get().ref);
}

void EC_WaterPlane::SetPosition()
{
    if (!node_ || !ViewEnabled())
        return;

    const float3 &pos = position.Get();
    if (pos.IsFinite())
        node_->setPosition(pos);
}

void EC_WaterPlane::SetOrientation()
{
    if (ViewEnabled() && node_)
        node_->_setDerivedOrientation(rotation.Get());
}

ComponentPtr EC_WaterPlane::FindPlaceable() const
{
    /// @todo The static_pointer_cast should not be necessary here, but without it when compiling TUNDRA_NO_BOOST build with VC9 we get:
    /// "error C2664: 'void std::tr1::_Ptr_base<_Ty>::_Reset(_Ty *,std::tr1::_Ref_count_base *)' : cannot convert parameter 1 from 'IComponent *const ' to 'EC_Placeable *'"
    return ParentEntity() ? static_pointer_cast<IComponent>(ParentEntity()->GetComponent<EC_Placeable>()) : ComponentPtr();
}

void EC_WaterPlane::AttachEntity()
{
    if (attached_ || entity_ == 0)
        return;

    EC_Placeable* placeable = dynamic_cast<EC_Placeable* >(FindPlaceable().get());

    try
    {
        // If there exist placeable attach node and entity to it
        if (placeable != 0 )
        {
            Ogre::SceneNode* node = placeable->OgreSceneNode();
            node->addChild(node_);
            node_->attachObject(entity_);
            node_->setVisible(true);
        }
        else
        {
            // There is no placeable attacht entity to OgreSceneRoot 
            node_->attachObject(entity_);
            world_.lock()->OgreSceneManager()->getRootSceneNode()->addChild(node_);
            node_->setVisible(true);
            attachedToRoot_ = true;
        }

        attached_ = true;
    }
    catch(const Ogre::Exception &e)
    {
        LogError("EC_WaterPlane::AttachEntity: exception caught: " + std::string(e.what()));
    }
}

void EC_WaterPlane::DetachEntity()
{
    if (!attached_ || entity_ == 0)
        return;

    try
    {
        EC_Placeable* placeable = dynamic_cast<EC_Placeable*>(FindPlaceable().get());
        if (placeable != 0 && !attachedToRoot_)
        {
            Ogre::SceneNode* node = placeable->OgreSceneNode();
            node_->detachObject(entity_);
            node->removeChild(node_); 
        }
        else
        {
            // Attached to root.
            // Sanity check..
            if (entity_->isAttached())
            {
                node_->detachObject(entity_);
                world_.lock()->OgreSceneManager()->getRootSceneNode()->removeChild(node_);
                attachedToRoot_ = false;
            }
        }

        attached_ = false;
    }
    catch(Ogre::Exception &e)
    {
        LogError("EC_WaterPlane::DetachEntity: exception caught: " + std::string(e.what()));
    }
}

void EC_WaterPlane::SetUnderwaterFog()
{
    if (!ViewEnabled())
        return;
    OgreWorldPtr world = world_.lock();
    if (world)
    {
        /// @todo in Tundra1-series, if we were within EC_WaterPlane, the waterPlaneColor*fogColor was used as the scene fog color.
        /// Do we want to do the same here?
        world->OgreSceneManager()->setFog((Ogre::FogMode)fogMode.Get(), fogColor.Get(), fogExpDensity.Get(), fogStartDistance.Get(), fogEndDistance.Get());
        world->Renderer()->MainViewport()->setBackgroundColour(fogColor.Get());
    }
}

void EC_WaterPlane::RestoreFog()
{
    if (!ViewEnabled())
        return;
    OgreWorldPtr world = world_.lock();
    if (world)
    {
        // Restore current scene fog settings, if existing.
        /// @todo Optimize
        std::vector<shared_ptr<EC_Fog> > fogs = ParentScene() ? ParentScene()->Components<EC_Fog>() : std::vector<shared_ptr<EC_Fog> >();
        if (!fogs.empty())
        {
            world->OgreSceneManager()->setFog((Ogre::FogMode)fogs[0]->mode.Get(), fogs[0]->color.Get(),
                fogs[0]->expDensity.Get(), fogs[0]->startDistance.Get(), fogs[0]->endDistance.Get());
            world->Renderer()->MainViewport()->setBackgroundColour(fogs[0]->color.Get());
        }
        // No scene fog, set the default ineffective fog.
        else
            world->SetDefaultSceneFog();
    }
}

void EC_WaterPlane::UpdateMaterial()
{
    if (entity_)
        entity_->setMaterialName(currentMaterial.toStdString());
}
