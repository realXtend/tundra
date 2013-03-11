// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
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

#include <Ogre.h>

#include "MemoryLeakCheck.h"

EC_WaterPlane::EC_WaterPlane(Scene* scene) :
    IComponent(scene),
    xSize(this, "x-size", 5000),
    ySize(this, "y-size", 5000),
    depth(this, "Depth", 10000),
    position(this, "Position", float3::zero),
    rotation(this, "Rotation", Quat::identity),
    scaleUfactor(this, "U factor", 0.0002f),
    scaleVfactor(this, "V factor", 0.0002f),
    xSegments(this, "Segments in x", 10),
    ySegments(this, "Segments in y", 10),
    materialName(this, "Material", QString("Ocean")),
    materialRef(this, "Material ref"),
   //textureNameAttr(this, "Texture", QString("DefaultOceanSkyCube.dds")),
    fogColor(this, "Fog color", Color(0.2f,0.4f,0.35f,1.0f)),
    fogStartDistance(this, "Fog start dist.", 100.f),
    fogEndDistance(this, "Fog end dist.", 2000.f),
    fogMode(this, "Fog mode", 3),
    fogExpDensity(this, "Fog exponential density", 0.001f),
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
        world_ = scene->GetWorld<OgreWorld>();
    OgreWorldPtr world = world_.lock();
    if (world)
        node_ = world->OgreSceneManager()->createSceneNode(world->GetUniqueObjectName("EC_WaterPlane_Root"));

    lastXsize_ = xSize.Get();
    lastYsize_ = ySize.Get();

    connect(this, SIGNAL(ParentEntitySet()), this, SLOT(Create()));

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
    if (world_.expired())
        return;

    OgreWorldPtr world = world_.lock();
    RemoveWaterPlane();

    if (node_)
        world->OgreSceneManager()->destroySceneNode(node_);
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
    if (component->TypeName() == EC_Placeable::TypeNameStatic())
    {
        DetachEntity();

        EC_Placeable* placeable = static_cast<EC_Placeable* >(component);
        if (placeable == 0)
            return;
        if (entity_ == 0)
            return;

        try
        {
            Ogre::SceneNode* node = placeable->GetSceneNode();
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
    
    if (component->TypeName() == EC_Placeable::TypeNameStatic())
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
    PROFILE(EC_WaterPlane_Update)
    OgreWorldPtr world = world_.lock();
    if (!world)
        return;

    bool cameraWasInsideWaterCube = cameraInsideWaterCube;
    cameraInsideWaterCube = IsCameraInsideWaterCube();
    if (!cameraWasInsideWaterCube && cameraInsideWaterCube)
    {
        SetUnderwaterFog();
    }
    else if (cameraWasInsideWaterCube && !cameraInsideWaterCube)
    {
        // Restore current scene fog settings, if existing.
        /// @todo Optimize
        EntityList entities = ParentScene()->GetEntitiesWithComponent(EC_Fog::TypeNameStatic());
        if (!entities.empty())
        {
            EC_Fog *sceneFog = (*entities.begin())->GetComponent<EC_Fog>().get();
            world->OgreSceneManager()->setFog((Ogre::FogMode)sceneFog->mode.Get(),sceneFog->color.Get(),
                sceneFog->expDensity.Get(), sceneFog->startDistance.Get(), sceneFog->endDistance.Get());
            world->Renderer()->MainViewport()->setBackgroundColour(sceneFog->color.Get());
        }
        else // No scene fog, set the default ineffective fog.
        {
            world->SetDefaultSceneFog();
        }
    }
    // else status quo
}

float3 EC_WaterPlane::GetPointOnPlane(const float3 &point) const 
{
    if (node_ == 0)
        return float3::nan;

    Ogre::Quaternion rot = node_->_getDerivedOrientation();
    Ogre::Vector3 trans = node_->_getDerivedPosition();
    Ogre::Vector3 scale = node_->_getDerivedScale();

    Ogre::Matrix4 worldTM;
    worldTM.makeTransform(trans, scale, rot);

    /// @todo Use the 1.7.x API? Not supporting Ogre 1.6.x any longer afaik.
    // In Ogre 1.7.1 we could simply use the following line, but since we're also supporting Ogre 1.6.4 for now, the above
    // lines are used instead, which work in both.
    // Ogre::Matrix4 worldTM = node_->_getFullTransform(); // local->world. 

    Ogre::Matrix4 inv = worldTM.inverse(); // world->local
    Ogre::Vector4 local = inv * Ogre::Vector4(point.x, point.y, point.z, 1.f);

    local.y = 0;
    Ogre::Vector4 world = worldTM * local;
    return float3(world.x, world.y, world.z);
}

float EC_WaterPlane::GetDistanceToWaterPlane(const float3& point) const
{
    if (!node_)
        return 0;

    float3 pointOnPlane = GetPointOnPlane(point);
    //Ogre::Vector3 local = node_->_getDerivedOrientation().Inverse() * ( OgreRenderer::ToOgreVector3(point) - node_->_getDerivedPosition()) / node_->_getDerivedScale();
    return point.y - pointOnPlane.y;
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
            entity_->setMaterialName(materialName.Get().toStdString().c_str());
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

void EC_WaterPlane::AttributesChanged()
{
    if ((xSize.ValueChanged() || ySize.ValueChanged() || scaleUfactor.ValueChanged() || scaleVfactor.ValueChanged()) &&
        (lastXsize_ != xSize.Get() || lastYsize_ != ySize.Get()))
    {
        CreateWaterPlane();
        lastXsize_ = xSize.Get();
        lastYsize_ = ySize.Get();
    }
    if (xSegments.ValueChanged() || ySegments.ValueChanged())
    {
        CreateWaterPlane();
    }
    if (position.ValueChanged())
    {
        SetPosition();
    }
    if (rotation.ValueChanged())
    {
        // Is there placeable component? If not use given rotation 
        //if (dynamic_cast<EC_Placeable*>(FindPlaceable().get()) == 0 )
        //{
           SetOrientation();
        //}
    }
    if (depth.ValueChanged())
    {
        // Change depth
        // Currently do nothing..
    }
    if (materialName.ValueChanged())
    {
        //Change material
        if (entity_)
            entity_->setMaterialName(materialName.Get().toStdString());
    }
    if (fogColor.ValueChanged() || fogStartDistance.ValueChanged() || fogEndDistance.ValueChanged() ||
        fogMode.ValueChanged() || fogExpDensity.ValueChanged())
    {
        // Apply fog immediately only if the camera is within the water cube.
        if (IsCameraInsideWaterCube())
            SetUnderwaterFog();
    }
/*
    // Currently commented out, working feature but not enabled yet.
    if (name == textureNameAttr.GetNameString())
    {

        QString currentMaterial = materialName.Get();
        
        // Check that has texture really changed. 
        
        StringVector names;
        Ogre::MaterialPtr materialPtr = Ogre::MaterialManager::getSingleton().getByName(currentMaterial.toStdString().c_str());
        
        if (materialPtr.get() == 0)
            return;

        OgreRenderer::GetTextureNamesFromMaterial(materialPtr, names);
        
        QString textureName = textureNameAttr.Get();
        
        for(StringVector::iterator iter = names.begin(); iter != names.end(); ++iter)
        {
            QString currentTextureName(iter->c_str());
            if (currentTextureName == textureName)
                return;
        }

        // So texture has really changed, let's change it. 
        OgreRenderer::SetTextureUnitOnMaterial(materialPtr, textureName.toStdString(), 0);
    }
*/
}

void EC_WaterPlane::SetPosition()
{
    if (!node_ || !ViewEnabled())
        return;

    const float3 &pos = position.Get();
    //node_->setPosition(vec.x, vec.y, vec.z);

    /// @todo Remove these ifdefs? Not supporting Ogre 1.6.x any longer afaik.
#if OGRE_VERSION_MINOR <= 6 && OGRE_VERSION_MAJOR <= 1
    //Ogre::Vector3 current_pos = node_->_getDerivedPosition();
    if (!pos.IsFinite())
        return;
    node_->setPosition(pos);
#else
    if (!pos.IsFinite())
        return;
    //node_->_setDerivedPosition(pos);
    node_->setPosition(pos);
#endif
}

void EC_WaterPlane::SetOrientation()
{
    if (!node_ || !ViewEnabled())
        return;
    /// @todo Remove these ifdefs? Not supporting Ogre 1.6.x any longer afaik.
#if OGRE_VERSION_MINOR <= 6 && OGRE_VERSION_MAJOR <= 1
    node_->setOrientation(node_->_getDerivedOrientation() * rotation.Get());
#else
    node_->_setDerivedOrientation(rotation.Get());
#endif
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
            Ogre::SceneNode* node = placeable->GetSceneNode();
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
            Ogre::SceneNode* node = placeable->GetSceneNode();
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
