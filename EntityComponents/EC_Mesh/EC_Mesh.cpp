// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EC_Mesh.h"
#include "EC_OgrePlaceable.h"

#include "OgreMeshResource.h"
#include "OgreMaterialResource.h"
#include "OgreSkeletonResource.h"
#include "Renderer.h"
#include "SceneManager.h"
#include "SceneEvents.h"
#include "EventManager.h"

#include <Ogre.h>
#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("EC_Mesh")

#include "MemoryLeakCheck.h"

EC_Mesh::EC_Mesh(IModule *module):
    Foundation::ComponentInterface(module->GetFramework()),
    nodeTransformation(this, "Transform"),
    meshResourceId(this, "Mesh ref", ""),
    skeletonId(this, "Skeleton ref", ""),
    meshMaterial(this, "Mesh materials"),
    drawDistance(this, "Draw distance", 0.0f),
    castShadows(this, "Cast shadows", false),
    entity_(0),
    node_(0),
    skeleton_(0),
    attached_(false)
{
    static AttributeMetadata drawDistanceData("", "0", "10000");
    drawDistance.SetMetadata(&drawDistanceData);

    renderer_ = GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>();
    if(!renderer_.expired())
    {
        Ogre::SceneManager* scene_mgr = renderer_.lock()->GetSceneManager();
        node_ = scene_mgr->createSceneNode();
    }

    Foundation::EventManager *event_manager = framework_->GetEventManager().get();
    if(event_manager)
    {
        event_manager->RegisterEventSubscriber(this, 99);
        resource_event_category_ = event_manager->QueryEventCategory("Resource");
    }

    QObject::connect(this, SIGNAL(ParentEntitySet()), this, SLOT(UpdateSignals()));
}

EC_Mesh::~EC_Mesh()
{
    if (renderer_.expired())
        return;
    OgreRenderer::RendererPtr renderer = renderer_.lock();
    RemoveMesh();
    
    if (node_)
    {
        Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
        scene_mgr->destroySceneNode(node_);
        node_ = 0;
    }
}

void EC_Mesh::SetMesh(const QString &name)
{
    if(renderer_.expired() || !node_)
        return;

    RemoveMesh();
    OgreRenderer::Renderer *renderer = renderer_.lock().get();
    Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
    try
    {
        entity_ = scene_mgr->createEntity(renderer->GetUniqueObjectName(), name.toStdString());
        if (!entity_)
        {
            LogError("Could not set mesh " + name.toStdString());
            return;
        }
        
        entity_->setRenderingDistance(drawDistance.Get());
        entity_->setCastShadows(castShadows.Get());
        entity_->setUserAny(Ogre::Any(GetParentEntity()));
        // Set UserAny also on subentities
        for (uint i = 0; i < entity_->getNumSubEntities(); ++i)
            entity_->getSubEntity(i)->setUserAny(entity_->getUserAny());
    }
    catch (Ogre::Exception& e)
    {
        LogError("Could not set mesh " + name.toStdString() + ": " + std::string(e.what()));
        return;
    }
    AttachEntity();

    if(node_)
    {
        Transform newTransform = nodeTransformation.Get();
        node_->setPosition(newTransform.position.x, newTransform.position.y, newTransform.position.z);
        Quaternion adjust(DEGTORAD * newTransform.rotation.x,
                          DEGTORAD * newTransform.rotation.y,
                          DEGTORAD * newTransform.rotation.z);
        adjust = Quaternion(PI/2, 0, PI) * adjust;
        node_->setOrientation(Ogre::Quaternion(adjust.w, adjust.x, adjust.y, adjust.z));
        node_->setScale(newTransform.scale.x, newTransform.scale.y, newTransform.scale.z);
    }

    // Check if new materials need to be requested.
    if(HasMaterialsChanged())
    {
        QVariantList materials = meshMaterial.Get();
        materialRequestTags_.resize(materials.size(), 0);
        for(uint i = 0; i < materials.size(); i++)
        {
            if(materialRequestTags_[i] != 0)
                continue;
            // We insert material tag in two locations cause we need to beaware of what are materials indexes.
            request_tag_t tag = RequestResource(materials[i].toString().toStdString(), OgreRenderer::OgreMaterialResource::GetTypeStatic());
            if(tag)
            {
                resRequestTags_[ResourceKeyPair(tag, OgreRenderer::OgreMaterialResource::GetTypeStatic())] = 
                    boost::bind(&EC_Mesh::HandleMaterialResourceEvent, this, _1, _2);
                materialRequestTags_[i] = tag; 
            }
        }
    }

    Scene::Events::EntityEventData event_data;
    event_data.entity = GetParentEntity()->GetSharedPtr();
    Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
    event_manager->SendEvent("Scene", Scene::Events::EVENT_ENTITY_VISUALS_MODIFIED, &event_data);

    emit OnMeshChanged();
}

void EC_Mesh::RemoveMesh()
{
    if (renderer_.expired() || !entity_)
        return;
    OgreRenderer::RendererPtr renderer = renderer_.lock();

    DetachEntity();
    Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
    scene_mgr->destroyEntity(entity_);
    entity_ = 0;
}

bool EC_Mesh::SetMaterial(uint index, const QString &material_name)
{
    if(!entity_)
        return false;

    if (index >= entity_->getNumSubEntities())
    {
        LogError("Could not set material " + material_name.toStdString() + ": illegal submesh index " + ToString<uint>(index));
        return false;
    }
    try
    {
        Ogre::SubEntity *sub = entity_->getSubEntity(index);
        if(sub)
            sub->setMaterialName(material_name.toStdString());
    }
    catch (Ogre::Exception& e)
    {
        LogError("Could not set material " + material_name.toStdString() + ": " + std::string(e.what()));
        return false;
    }

    emit OnMaterialChanged(index, material_name);
    return true;
}

bool EC_Mesh::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface *data)
{
    if(category_id == resource_event_category_)
    {
        if(event_id == Resource::Events::RESOURCE_READY)
        {
            return HandleResourceEvent(event_id, data);
        }
    }
    return false;
}

bool EC_Mesh::HasMaterialsChanged() const
{
    if(!entity_ || !meshMaterial.Get().size())
        return false;
    QVariantList materials = meshMaterial.Get();
    for(uint i = 0; i < entity_->getNumSubEntities(); i++)
    {
        // No point to continue if all materials are not setted.
        if(i >= materials.size())
            break;

        if(entity_->getSubEntity(i)->getMaterial()->getName() != materials[i].toString().toStdString())
            return true;
    }
    return false;
}

void EC_Mesh::UpdateSignals()
{
    disconnect(this, SLOT(AttributeUpdated(Foundation::ComponentInterface *, AttributeInterface *)));
    if(!GetParentEntity())
        return;

    Scene::SceneManager *scene = GetParentEntity()->GetScene();
    if(scene)
        connect(scene, SIGNAL(AttributeChanged(Foundation::ComponentInterface*, AttributeInterface*, AttributeChange::Type)),
                this, SLOT(AttributeUpdated(Foundation::ComponentInterface*, AttributeInterface*)));
}

void EC_Mesh::AttributeUpdated(Foundation::ComponentInterface *component, AttributeInterface *attribute)
{
    if(component != this)
        return;
    QString attrName = QString::fromStdString(attribute->GetNameString());
    request_tag_t tag = 0;
    if(QString::fromStdString(meshResourceId.GetNameString()) == attrName)
    {
        //Ensure that mesh is requested only when it's has actualy changed.
        if(entity_)
            if(QString::fromStdString(entity_->getMesh()->getName()) == meshResourceId.Get())
                return;

        tag = RequestResource(meshResourceId.Get().toStdString(), OgreRenderer::OgreMeshResource::GetTypeStatic());
        if(tag)
            resRequestTags_[ResourceKeyPair(tag, OgreRenderer::OgreMeshResource::GetTypeStatic())] = 
                boost::bind(&EC_Mesh::HandleMeshResourceEvent, this, _1, _2);
        else
            RemoveMesh();
    }
    else if(QString::fromStdString(meshMaterial.GetNameString()) == attrName)
    {
        // We wont request materials until we are sure that mesh has been loaded and it's safe to apply materials into it.
        if(!HasMaterialsChanged())
            return;
        QVariantList materials = meshMaterial.Get();
        materialRequestTags_.resize(materials.size(), 0);
        for(uint i = 0; i < materials.size(); i++)
        {
            // We insert material tag in two locations cause we need to beaware of what is the material's index.
            tag = RequestResource(materials[i].toString().toStdString(), OgreRenderer::OgreMaterialResource::GetTypeStatic());
            if(tag)
            {
                resRequestTags_[ResourceKeyPair(tag, OgreRenderer::OgreMaterialResource::GetTypeStatic())] = 
                    boost::bind(&EC_Mesh::HandleMaterialResourceEvent, this, _1, _2);
                materialRequestTags_[i] = tag;
            }
        }
    }
    else if(QString::fromStdString(skeletonId.GetNameString()) == attrName)
    {
        if(!skeletonId.Get().isEmpty())
        {
            // If same name skeleton has already setted no point to do it again.
            if(skeleton_ && skeleton_->getName() == skeletonId.Get().toStdString())
                return;

            std::string resouceType = OgreRenderer::OgreSkeletonResource::GetTypeStatic();
            tag = RequestResource(skeletonId.Get().toStdString(), resouceType);
            if(tag)
                resRequestTags_[ResourceKeyPair(tag, resouceType)] = boost::bind(&EC_Mesh::HandleSkeletonResourceEvent, this, _1, _2);
        }
    }
    else if(QString::fromStdString(drawDistance.GetNameString()) == attrName)
    {
        if(entity_)
            entity_->setRenderingDistance(drawDistance.Get());
    }
    else if(QString::fromStdString(castShadows.GetNameString()) == attrName)
    {
        if(entity_)
            entity_->setCastShadows(castShadows.Get());
    }
    else if(QString::fromStdString(nodeTransformation.GetNameString()) == attrName)
    {
        if(node_)
        {
            Transform newTransform = nodeTransformation.Get();
            node_->setPosition(newTransform.position.x, newTransform.position.y, newTransform.position.z);
            Quaternion adjust(DEGTORAD * newTransform.rotation.x,
                              DEGTORAD * newTransform.rotation.y,
                              DEGTORAD * newTransform.rotation.z);
            adjust = Quaternion(PI/2, 0, PI) * adjust;
            node_->setOrientation(Ogre::Quaternion(adjust.w, adjust.x, adjust.y, adjust.z));
            node_->setScale(newTransform.scale.x, newTransform.scale.y, newTransform.scale.z);
        }
    }
}

request_tag_t EC_Mesh::RequestResource(const std::string& id, const std::string& type)
{
    request_tag_t tag = 0;
    if(renderer_.expired())
        return tag;

    tag = renderer_.lock()->RequestResource(id, type);
    if(tag == 0)
    {
        LogWarning("Failed to request resource:" + id + " : " + type);
        return 0;
    }

    return tag;
}

Foundation::ComponentPtr EC_Mesh::FindPlaceable() const
{
    assert(framework_);
    Foundation::ComponentPtr comp;
    if(!GetParentEntity())
        return comp;
    comp = GetParentEntity()->GetComponent<OgreRenderer::EC_OgrePlaceable>();
    return comp;
}

void EC_Mesh::AttachEntity()
{
    OgreRenderer::EC_OgrePlaceable* placeable = dynamic_cast<OgreRenderer::EC_OgrePlaceable*>(FindPlaceable().get());
    if ((!entity_) || (!placeable) || attached_)
        return;

    Ogre::SceneNode* node = placeable->GetSceneNode();
    node->addChild(node_);
    node_->attachObject(entity_);

    attached_ = true;
}

void EC_Mesh::DetachEntity()
{
    OgreRenderer::EC_OgrePlaceable* placeable = dynamic_cast<OgreRenderer::EC_OgrePlaceable*>(FindPlaceable().get());
    if ((!attached_) || (!entity_) || (!placeable))
        return;

    Ogre::SceneNode* node = placeable->GetSceneNode();
    node_->detachObject(entity_);
    node->removeChild(node_);

    attached_ = false;
}

void EC_Mesh::AttachSkeleton(const QString &skeletonName)
{
    if(!entity_)
        return;

    try
    {
        Ogre::SkeletonPtr skel = Ogre::SkeletonManager::getSingleton().getByName(skeletonName.toStdString());
        if(skel.get())
        {
            entity_->getMesh()->_notifySkeleton(skel);
            LogDebug("Set skeleton " + skeleton_->getName() + " to mesh " + entity_->getName());
            emit OnSkeletonChanged(QString::fromStdString(skeleton_->getName()));
        }
    }
    catch (Ogre::Exception& e)
    {
        LogError("Could not set skeleton " + skeleton_->getName() + " to mesh " + entity_->getName() + ": " + std::string(e.what()));
    }
}

bool EC_Mesh::HandleResourceEvent(event_id_t event_id, Foundation::EventDataInterface* data)
{
    if (event_id != Resource::Events::RESOURCE_READY)
        return false;

    Resource::Events::ResourceReady* event_data = checked_static_cast<Resource::Events::ResourceReady*>(data);

    ResourceKeyPair event_key(event_data->tag_, event_data->resource_->GetType());
    MeshResourceHandlerMap::iterator iter2 = resRequestTags_.find(event_key);
    if(iter2 != resRequestTags_.end())
    {
        iter2->second(event_id, data);
        resRequestTags_.erase(iter2);
        return true;
    }
    return false;
}

bool EC_Mesh::HandleMeshResourceEvent(event_id_t event_id, Foundation::EventDataInterface* data)
{
    Resource::Events::ResourceReady* event_data = checked_static_cast<Resource::Events::ResourceReady*>(data);
    Foundation::ResourcePtr res = event_data->resource_;

    if (!res)
        return false;
    if (res->GetType() != OgreRenderer::OgreMeshResource::GetTypeStatic())
        return false;
    OgreRenderer::OgreMeshResource* meshResource = checked_static_cast<OgreRenderer::OgreMeshResource*>(res.get());
    //! @todo for some reason compiler will have linking error if we try to call ResourceInterface's GetId inline method
    //! remember to track the cause of this when I some extra time.
    SetMesh(QString::fromStdString(meshResourceId.Get().toStdString()));

    return true;
}

bool EC_Mesh::HandleSkeletonResourceEvent(event_id_t event_id, Foundation::EventDataInterface* data)
{
    if(!entity_)
        return false;

    Resource::Events::ResourceReady* event_data = checked_static_cast<Resource::Events::ResourceReady*>(data);
    Foundation::ResourcePtr res = event_data->resource_;

    OgreRenderer::OgreSkeletonResource *skeletonRes = dynamic_cast<OgreRenderer::OgreSkeletonResource*>(res.get());
    if(skeletonRes)
    {
        skeleton_ = skeletonRes->GetSkeleton().get();
        if(!skeleton_)
            return false;

        // If old skeleton is same as a new one no need to replace it.
        if(entity_->getSkeleton() && entity_->getSkeleton()->getName() == skeleton_->getName())
            return false;

        AttachSkeleton(QString::fromStdString(skeleton_->getName()));
    }
    return true;
}

bool EC_Mesh::HandleMaterialResourceEvent(event_id_t event_id, Foundation::EventDataInterface* data)
{
    Resource::Events::ResourceReady* event_data = checked_static_cast<Resource::Events::ResourceReady*>(data);
    Foundation::ResourcePtr res = event_data->resource_;

    //! a bit hackish way to get materials in right order.
    bool found = false;
    uint index = 0;
    for(; index < materialRequestTags_.size(); index++)
    {
        if(materialRequestTags_[index] == event_data->tag_)
        {
            found = true;
            break;
        }
    }
    if(found)
    {
        //HandleMaterialReady(index, res);
        if (!res || index > meshMaterial.Get().size()) 
            return false;
        OgreRenderer::OgreMaterialResource* materialResource = checked_static_cast<OgreRenderer::OgreMaterialResource*>(res.get());
        QString material_name = QString::fromStdString(materialResource->GetMaterial()->getName());
        SetMaterial(index, material_name);
        materialRequestTags_[index] = 0;
    }
    return true;
}