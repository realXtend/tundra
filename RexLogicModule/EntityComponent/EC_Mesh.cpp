#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EC_Mesh.h"
#include "EC_OgrePlaceable.h"

#include "OgreRenderingModule.h"
#include "OgreMeshResource.h"
#include "OgreMaterialResource.h"
#include "Renderer.h"
#include "SceneManager.h"
#include "SceneEvents.h"
#include "EventManager.h"

#include <Ogre.h>
#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("EC_Mesh")

#include "MemoryLeakCheck.h"

EC_Mesh::EC_Mesh(Foundation::ModuleInterface *module):
    Foundation::ComponentInterface(module->GetFramework()),
    meshResouceId_(this, "Mesh id", ""),
    meshMaterial_(this, "Mesh materials"),
    drawDistance_(this, "Draw distance", 0.0f),
    castShadows_(this, "Cast shadows", false),
    framework_(module->GetFramework()),
    entity_(0),
    node_(0),
    attached_(false)
{
    static Foundation::AttributeMetadata drawDistanceData;
    static bool metadataInitialized = false;
    if(!metadataInitialized)
    {
        drawDistanceData.min = "0";
        drawDistanceData.max = "10000";
    }
    drawDistance_.SetMetadata(&drawDistanceData);

    OgreRenderer::OgreRenderingModule *rendererModule = framework_->GetModuleManager()->GetModule<OgreRenderer::OgreRenderingModule>().lock().get();
    if(!rendererModule)
        return;

    renderer_ = OgreRenderer::RendererWeakPtr(rendererModule->GetRenderer());
    if(!renderer_.expired())
    {
        Ogre::SceneManager* scene_mgr = renderer_.lock()->GetSceneManager();
        node_ = scene_mgr->createSceneNode();
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

void EC_Mesh::SetPlaceable(Foundation::ComponentPtr placeable)
{
    if (dynamic_cast<OgreRenderer::EC_OgrePlaceable*>(placeable.get()) == 0)
    {
        LogError("Attempted to set a placeable which is not a placeable");
        return;
    }
    
    if (placeable_ == placeable)
        return;
    
    placeable_ = placeable;
}

bool EC_Mesh::HandleResourceEvent(event_id_t event_id, Foundation::EventDataInterface* data)
{
    if (event_id != Resource::Events::RESOURCE_READY)
        return false;

    Resource::Events::ResourceReady* event_data = checked_static_cast<Resource::Events::ResourceReady*>(data);
    
    AssetRequestTags::iterator iter = requestTags_.find(event_data->tag_);
    if (iter == requestTags_.end())
        return false;

    Foundation::ResourcePtr res = event_data->resource_;
    asset_type_t asset_type = -1;
    // First we identify what kind of resource is ready:
    if (res->GetType() == OgreRenderer::OgreMeshResource::GetTypeStatic())
        asset_type = RexTypes::RexAT_Mesh;
    else if (res->GetType() == OgreRenderer::OgreMaterialResource::GetTypeStatic())
        asset_type = RexTypes::RexAT_MaterialScript;

    switch(asset_type)
    {
    case RexTypes::RexAT_Mesh:
    {
        if (!res)
            return false;
        if (res->GetType() != OgreRenderer::OgreMeshResource::GetTypeStatic())
            return false;
        OgreRenderer::OgreMeshResource* meshResource = checked_static_cast<OgreRenderer::OgreMeshResource*>(res.get());
        SetMesh(meshResource->GetId());
        //HandleMeshReady(res);
        break;
    }
    case RexTypes::RexAT_MaterialScript:
    {
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
            if (!res || index > meshMaterial_.Get().size()) 
                return false;
            OgreRenderer::OgreMaterialResource* materialResource = checked_static_cast<OgreRenderer::OgreMaterialResource*>(res.get());
            SetMaterial(index, materialResource->GetMaterial()->getName());
            materialRequestTags_[index] = 0;
        }
        break;
    }
    default:
    {
        assert(false && "Invalid asset_type added to prim_resource_request_tags_! Don't know how it ended up there and don't know how to handle!");
        break;
    }
    }

    requestTags_.erase(iter);
    return true;
}

void EC_Mesh::SetMesh(const std::string &name, bool clone)
{
    if(renderer_.expired() || name == "" || !node_)
        return;

    RemoveMesh();
    OgreRenderer::Renderer *renderer = renderer_.lock().get();
    Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
    try
    {
        entity_ = scene_mgr->createEntity(renderer->GetUniqueObjectName(), name);
        if (!entity_)
        {
            LogError("Could not set mesh " + name);
            return;
        }
        
        entity_->setRenderingDistance(drawDistance_.Get());
        entity_->setCastShadows(castShadows_.Get());
        entity_->setUserAny(Ogre::Any(GetParentEntity()));
        // Set UserAny also on subentities
        for (uint i = 0; i < entity_->getNumSubEntities(); ++i)
            entity_->getSubEntity(i)->setUserAny(entity_->getUserAny());
    }
    catch (Ogre::Exception& e)
    {
        LogError("Could not set mesh " + name + ": " + std::string(e.what()));
        return;
    }
    AttachEntity();

    // Convert orientation from Ogre space to Rex space.
    Quaternion adjust(PI/2, 0, PI);
    node_->setOrientation(Ogre::Quaternion(adjust.w, adjust.x, adjust.y, adjust.z));

    // Check if materials have changed and need to be updated.
    if(HasMaterialsChanged())
    {
        std::vector<QVariant> materials = meshMaterial_.Get();
        materialRequestTags_.resize(materials.size(), 0);
        for(uint i = 0; i < materials.size(); i++)
        {
            if(materialRequestTags_[i] != 0)
                continue;
            // We insert material tag in two locations cause we need to beaware of what are materials indexes.
            request_tag_t tag = RequestResource(materials[i].toString().toStdString(), OgreRenderer::OgreMaterialResource::GetTypeStatic());
            requestTags_.insert(tag);
            materialRequestTags_[i] = tag;
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

bool EC_Mesh::SetMaterial(uint index, const std::string& material_name)
{
    if(!entity_)
        return false;

    if (index >= entity_->getNumSubEntities())
    {
        LogError("Could not set material " + material_name + ": illegal submesh index " + ToString<uint>(index));
        return false;
    }
    try
    {
        Ogre::SubEntity *sub = entity_->getSubEntity(index);
        if(sub)
            sub->setMaterialName(material_name);
    }
    catch (Ogre::Exception& e)
    {
        LogError("Could not set material " + material_name + ": " + std::string(e.what()));
        return false;
    }

    emit OnMaterialChanged(index, material_name);
    return true;
}

bool EC_Mesh::HasMaterialsChanged() const
{
    if(!entity_ || !meshMaterial_.Get().size())
        return false;
    std::vector<QVariant> materials = meshMaterial_.Get();
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
    disconnect(this, SLOT(AttributeUpdated(Foundation::ComponentInterface *, Foundation::AttributeInterface *)));
    connect(GetParentEntity()->GetScene(), SIGNAL(AttributeChanged(Foundation::ComponentInterface*, Foundation::AttributeInterface*, AttributeChange::Type)),
            this, SLOT(AttributeUpdated(Foundation::ComponentInterface*, Foundation::AttributeInterface*)));
    placeable_ = GetParentEntity()->GetComponent<OgreRenderer::EC_OgrePlaceable>();
    if(!placeable_)
        LogError("Component need to have a EC_OgrePlaceable component so that mesh can be attach into the world.");
}

void EC_Mesh::AttributeUpdated(Foundation::ComponentInterface *component, Foundation::AttributeInterface *attribute)
{
    if(component != this)
        return;
    QString attrName = attribute->GetName();
    request_tag_t tag = 0;
    if(meshResouceId_.GetName() == attrName)
    {
        //Ensure that mesh is requested only when it's has actualy changed.
        if(entity_)
            if(entity_->getMesh()->getName() == meshResouceId_.Get())
                return;

        tag = RequestResource(meshResouceId_.Get(), OgreRenderer::OgreMeshResource::GetTypeStatic());
        if(tag)
            requestTags_.insert(tag);
    }
    else if(meshMaterial_.GetName() == attrName)
    {
        // We wont request materials until we are sure that mesh has been loaded and it's safe to apply materials into it.
        if(!HasMaterialsChanged())
            return;
        std::vector<QVariant> materials = meshMaterial_.Get();
        materialRequestTags_.resize(materials.size(), 0);
        for(uint i = 0; i < materials.size(); i++)
        {
            //if(materialRequestTags_[i] != 0)
            //    continue;
            // We insert material tag in two locations cause we need to beaware of what is the material's index.
            tag = RequestResource(materials[i].toString().toStdString(), OgreRenderer::OgreMaterialResource::GetTypeStatic());
            if(tag)
            {
                requestTags_.insert(tag);
                materialRequestTags_[i] = tag;
            }
        }
    }
    else if(drawDistance_.GetName() == attrName)
    {
        if(entity_)
            entity_->setRenderingDistance(drawDistance_.Get());
    }
    else if(castShadows_.GetName() == attrName)
    {
        if(entity_)
            entity_->setCastShadows(castShadows_.Get());
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
        LogWarning("Failed to request asset:" + id + " : " + type);
        return 0;
    }

    return tag;
}

void EC_Mesh::AttachEntity()
{
    if ((!entity_) || (!placeable_) || attached_)
        return;

    OgreRenderer::EC_OgrePlaceable* placeable = checked_static_cast<OgreRenderer::EC_OgrePlaceable*>(placeable_.get());
    Ogre::SceneNode* node = placeable->GetSceneNode();
    node->addChild(node_);
    node_->attachObject(entity_);

    attached_ = true;
}

void EC_Mesh::DetachEntity()
{
    if ((!attached_) || (!entity_) || (!placeable_))
        return;

    OgreRenderer::EC_OgrePlaceable* placeable = checked_static_cast<OgreRenderer::EC_OgrePlaceable*>(placeable_.get());
    Ogre::SceneNode* node = placeable->GetSceneNode();
    node_->detachObject(entity_);
    node->removeChild(node_);

    attached_ = false;
}