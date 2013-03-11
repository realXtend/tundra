// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "OgreRenderingModule.h"
#include "OgreWorld.h"
#include "Renderer.h"
#include "Entity.h"
#include "Scene/Scene.h"
#include "EC_Placeable.h"
#include "EC_OgreCustomObject.h"

#include <Ogre.h>
#include "MemoryLeakCheck.h"

using namespace OgreRenderer;

EC_OgreCustomObject::EC_OgreCustomObject(Scene* scene) :
    IComponent(scene),
    entity_(0),
    attached_(false),
    cast_shadows_(false),
    draw_distance_(0.0f)
{
    if (scene)
        world_ = scene->GetWorld<OgreWorld>();
}

EC_OgreCustomObject::~EC_OgreCustomObject()
{
    if (world_.expired())
    {
        if (entity_)
            LogError("EC_OgreCustomObject: World has expired, skipping uninitialization!");
        return;
    }
    
    DestroyEntity();
}

void EC_OgreCustomObject::SetPlaceable(ComponentPtr placeable)
{
    if (placeable && !dynamic_cast<EC_Placeable*>(placeable.get()))
    {
        ::LogError("Attempted to set placeable which is not " + EC_Placeable::TypeNameStatic().toStdString());
        return;
    }
    
    DetachEntity();
    placeable_ = placeable;
    AttachEntity();
}

bool EC_OgreCustomObject::CommitChanges(Ogre::ManualObject* object)
{
    if (!object)
        return false;
    
    if (world_.expired())
        return false;
    OgreWorldPtr world = world_.lock();
    
    DestroyEntity();
    
    // If placeable is not set yet, set it manually by searching it from the parent entity
    if (!placeable_)
    {
        Entity* entity = ParentEntity();
        if (entity)
        {
            ComponentPtr placeable = entity->GetComponent(EC_Placeable::TypeNameStatic());
            if (placeable)
                placeable_ = placeable;
        }
    }
    
    if (!object->getNumSections())
        return true;
        
    try
    {
        std::string mesh_name = world->GetUniqueObjectName("EC_OgreCustomObject_mesh");
        object->convertToMesh(mesh_name);
        object->clear();
    
        Ogre::SceneManager* sceneMgr = world->OgreSceneManager();

        entity_ = sceneMgr->createEntity(world->GetUniqueObjectName("EC_OgreCustomObject_entity"), mesh_name);
        if (entity_)
        {
            AttachEntity();
            entity_->setRenderingDistance(draw_distance_);
            entity_->setCastShadows(cast_shadows_);
            entity_->setUserAny(Ogre::Any(static_cast<IComponent *>(this)));
            // Set UserAny also on subentities
            for(uint i = 0; i < entity_->getNumSubEntities(); ++i)
                entity_->getSubEntity(i)->setUserAny(entity_->getUserAny());
        }
        else
        {
            ::LogError("Could not create entity from manualobject mesh");
            return false;
        }
    }
    catch(const Ogre::Exception& e)
    {
        ::LogError("Could not convert manualobject to mesh: " + std::string(e.what()));
        return false;
    }
    
    return true;
}

void EC_OgreCustomObject::SetDrawDistance(float draw_distance)
{
    draw_distance_ = draw_distance;
    if (entity_)
        entity_->setRenderingDistance(draw_distance);
}

void EC_OgreCustomObject::SetCastShadows(bool enabled)
{
    cast_shadows_ = enabled;
    if (entity_)
        entity_->setCastShadows(enabled);
}

bool EC_OgreCustomObject::SetMaterial(uint index, const std::string& material_name)
{
    if (!entity_)
        return false;
    
    if (index >= entity_->getNumSubEntities())
    {
        ::LogError("Could not set material " + material_name + ": illegal submesh index " + QString::number(index).toStdString());
        return false;
    }
    
    try
    {
        entity_->getSubEntity(index)->setMaterialName(material_name);
    }
    catch(const Ogre::Exception& e)
    {
        ::LogError("Could not set material " + material_name + ": " + std::string(e.what()));
        return false;
    }
    
    return true;
}

uint EC_OgreCustomObject::GetNumMaterials() const
{
    if (!entity_)
        return 0;
        
    return entity_->getNumSubEntities();
}

const std::string& EC_OgreCustomObject::GetMaterialName(uint index) const
{
    const static std::string empty;
    
    if (!entity_)
        return empty;
    
    if (index >= entity_->getNumSubEntities())
        return empty;
    
    return entity_->getSubEntity(index)->getMaterialName();
}

void EC_OgreCustomObject::AttachEntity()
{
    if (placeable_ && !attached_ && entity_)
    {
        EC_Placeable* placeable = checked_static_cast<EC_Placeable*>(placeable_.get());
        Ogre::SceneNode* node = placeable->GetSceneNode();
        node->attachObject(entity_);
        attached_ = true;
    }
}

void EC_OgreCustomObject::DetachEntity()
{
    if (placeable_ && attached_ && entity_)
    {
        EC_Placeable* placeable = checked_static_cast<EC_Placeable*>(placeable_.get());
        Ogre::SceneNode* node = placeable->GetSceneNode();
        node->detachObject(entity_);
        attached_ = false;
    }
}

void EC_OgreCustomObject::DestroyEntity()
{
    if (world_.expired())
        return;
    OgreWorldPtr world = world_.lock();
    
    Ogre::SceneManager* sceneMgr = world->OgreSceneManager();
    
    if (entity_)
    {
        DetachEntity();
        std::string mesh_name = entity_->getMesh()->getName();
        sceneMgr->destroyEntity(entity_);
        entity_ = 0;
        try
        {
            Ogre::MeshManager::getSingleton().remove(mesh_name);
        }
        catch(...) {}
    }
}

void EC_OgreCustomObject::GetBoundingBox(float3& min, float3& max) const
{
    if (!entity_)
    {
        min = float3(0.0, 0.0, 0.0);
        max = float3(0.0, 0.0, 0.0);
        return;
    }
 
    const Ogre::AxisAlignedBox& bbox = entity_->getBoundingBox();//getMesh()->getBounds();
    const Ogre::Vector3& bboxmin = bbox.getMinimum();
    const Ogre::Vector3& bboxmax = bbox.getMaximum();
    
    min = float3(bboxmin.x, bboxmin.y, bboxmin.z);
    max = float3(bboxmax.x, bboxmax.y, bboxmax.z);
}
