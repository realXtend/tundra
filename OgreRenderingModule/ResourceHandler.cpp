// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AssetEvents.h"
#include "OgreRenderingModule.h"
#include "OgreTextureResource.h"
#include "OgreMeshResource.h"
#include "OgreMaterialResource.h"
#include "ResourceInterface.h"
#include "ResourceHandler.h"

namespace OgreRenderer
{
    ResourceHandler::ResourceHandler(Foundation::Framework* framework) :
        framework_(framework)
    {
    }

    ResourceHandler::~ResourceHandler()
    {
        textures_.clear();
        meshes_.clear();    
    }
    
    void ResourceHandler::PostInitialize()
    {
        Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
        
        resourcecategory_id_ = event_manager->QueryEventCategory("Resource");
    }
    
    Foundation::ResourcePtr ResourceHandler::GetResource(const std::string& id, const std::string& type)
    {
        if (type == OgreTextureResource::GetTypeStatic())
            return GetTexture(id);
        if (type == OgreMeshResource::GetTypeStatic())
            return GetMesh(id);
        if (type == OgreMaterialResource::GetTypeStatic())
            return GetMaterial(id);
            
        OgreRenderingModule::LogWarning("Requested unknown renderer resource type " + type);            
        return Foundation::ResourcePtr();
    }
    
    Core::request_tag_t ResourceHandler::RequestResource(const std::string& id, const std::string& type)   
    {
        if (type == OgreTextureResource::GetTypeStatic())
            return RequestTexture(id);
        if (type == OgreMeshResource::GetTypeStatic())
            return RequestMesh(id);
        if (type == OgreMaterialResource::GetTypeStatic())
            return RequestMaterial(id);
            
        OgreRenderingModule::LogWarning("Requested unknown renderer resource type " + type);
        return 0;     
    }
    
    void ResourceHandler::RemoveResource(const std::string& id, const std::string& type)
    {
        if (type == OgreTextureResource::GetTypeStatic())
            return RemoveTexture(id);
        if (type == OgreMeshResource::GetTypeStatic())
            return RemoveMesh(id); 
        if (type == OgreMaterialResource::GetTypeStatic())
            return RemoveMaterial(id);
    }    
    
    bool ResourceHandler::HandleAssetEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        switch (event_id)
        {
            case Asset::Events::ASSET_READY:
            {
                Asset::Events::AssetReady *event_data = checked_static_cast<Asset::Events::AssetReady*>(data); 

                // Check that the request tag matches our request, so we do not (possibly) update unnecessarily many times
                // because of others' requests     
                if (expected_request_tags_.find(event_data->tag_) == expected_request_tags_.end())             
                    return false;

                if (event_data->asset_type_ == "Mesh")
                    UpdateMesh(event_data->asset_, event_data->tag_);

                if (event_data->asset_type_ == "MaterialScript")
                    UpdateMaterial(event_data->asset_, event_data->tag_);
            }
            break;
            
            case Asset::Events::ASSET_CANCELED:
            {
                Asset::Events::AssetCanceled *event_data = checked_static_cast<Asset::Events::AssetCanceled*>(data);   
                // Remove client request tags related to this asset, we're not going to get it
                request_tags_.erase(event_data->asset_id_);
            }
            break;
        }

        return false;
    }

    bool ResourceHandler::HandleResourceEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (event_id == Resource::Events::RESOURCE_READY)
        {     
            Resource::Events::ResourceReady *event_data = checked_static_cast<Resource::Events::ResourceReady*>(data);  
            if (event_data->resource_)
            {
                if (event_data->resource_->GetType() == "Texture")
                {
                    // Check that the request tag matches our request, so we do not (possibly) update unnecessarily many times
                    // because of others' requests
                    if (expected_request_tags_.find(event_data->tag_) != expected_request_tags_.end())
                        UpdateTexture(event_data->resource_, event_data->tag_);
                }
            }
        }

        return false;
    }

    Core::request_tag_t ResourceHandler::RequestTexture(const std::string& id)
    {
        Core::request_tag_t tag = framework_->GetEventManager()->GetNextRequestTag();
            
        // See if already have the texture and at maximum quality level
        Foundation::ResourcePtr tex = GetTexture(id);
        if (tex)
        {
            if (checked_static_cast<OgreTextureResource*>(tex.get())->GetLevel() == 0)
            {
                Resource::Events::ResourceReady* event_data = new Resource::Events::ResourceReady(tex->GetId(), tex, tag);
                framework_->GetEventManager()->SendDelayedEvent(resourcecategory_id_, Resource::Events::RESOURCE_READY, Foundation::EventDataPtr(event_data));
                return tag;
            }
        }           

        // Request from texture decoder
        Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager(); 
        if (service_manager->IsRegistered(Foundation::Service::ST_Texture))
        {
            boost::shared_ptr<Foundation::TextureServiceInterface> texture_service = service_manager->GetService<Foundation::TextureServiceInterface>(Foundation::Service::ST_Texture).lock();            
            // Perform the actual decode request only once, for the first request
            if (request_tags_.find(id) == request_tags_.end())
            {
                Core::request_tag_t source_tag = texture_service->RequestTexture(id);
                if (source_tag)
                {          
                    expected_request_tags_.insert(source_tag);
                    request_tags_[id].push_back(tag); 
                    return tag;
                }
            }
            else
            {
                request_tags_[id].push_back(tag); 
                return tag;
            }                   
        }
        
        return 0;
    }

    Foundation::ResourcePtr ResourceHandler::GetTexture(const std::string& id)
    {
        Foundation::ResourceMap::iterator i = textures_.find(id);
        if (i == textures_.end())
            return Foundation::ResourcePtr();
        else
            return i->second;
    }

    void ResourceHandler::RemoveTexture(const std::string& id)
    {
        Foundation::ResourceMap::iterator i = textures_.find(id);
        if (i == textures_.end())
            return;
        else
            textures_.erase(i);
    }

    bool ResourceHandler::UpdateTexture(Foundation::ResourcePtr source, Core::request_tag_t tag)
    {
        Foundation::TexturePtr source_tex = boost::shared_dynamic_cast<Foundation::TextureInterface>(source);
        if (!source_tex) 
            return false;
    
        // If not found, prepare new
        Foundation::ResourcePtr tex = GetTexture(source_tex->GetId());
        if (!tex)
        {
            tex = textures_[source_tex->GetId()] = Foundation::ResourcePtr(new OgreTextureResource(source_tex->GetId()));
        }

        // If highest level, erase texture decode request tag (should not get more raw resource events for this texture)
        if (source_tex->GetLevel() == 0)
            expected_request_tags_.erase(tag);

        // If success, send Ogre resource ready event
        bool success = false;
        if (checked_static_cast<OgreTextureResource*>(tex.get())->SetData(source_tex))
        {
            const Core::RequestTagVector& tags = request_tags_[source_tex->GetId()];            
            for (Core::uint i = 0; i < tags.size(); ++i)
            {
                Resource::Events::ResourceReady event_data(tex->GetId(), tex, tags[i]);
                framework_->GetEventManager()->SendEvent(resourcecategory_id_, Resource::Events::RESOURCE_READY, &event_data);
            }
           
            success = true;            
        }

        // If highest level, erase also request tags 
        if (source_tex->GetLevel() == 0)
            request_tags_.erase(source_tex->GetId());

        return success;
    }    

    Core::request_tag_t ResourceHandler::RequestMesh(const std::string& id)
    {
        Core::request_tag_t tag = framework_->GetEventManager()->GetNextRequestTag();
        
        // See if already have the mesh with data
        Foundation::ResourcePtr mesh = GetMesh(id);
        if (mesh)
        {
            if (!checked_static_cast<OgreMeshResource*>(mesh.get())->GetMesh().isNull())
            {
                Resource::Events::ResourceReady* event_data = new Resource::Events::ResourceReady(mesh->GetId(), mesh, tag);
                framework_->GetEventManager()->SendDelayedEvent(resourcecategory_id_, Resource::Events::RESOURCE_READY, Foundation::EventDataPtr(event_data));
                return tag;
            }
        }
        
        // Request from asset system
        Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager(); 
        if (service_manager->IsRegistered(Foundation::Service::ST_Asset))
        {
            boost::shared_ptr<Foundation::AssetServiceInterface> asset_service = service_manager->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();

            // Perform the actual asset request only once, for the first request
            if (request_tags_.find(id) == request_tags_.end())
            {
                Core::request_tag_t source_tag = asset_service->RequestAsset(id, "Mesh");
                if (source_tag) 
                {
                    request_tags_[id].push_back(tag);
                    expected_request_tags_.insert(source_tag);
                    return tag;
                }                
            }     
            else
            {
                request_tags_[id].push_back(tag); 
                return tag;
            }                     
        }

        return 0;
    }

    Foundation::ResourcePtr ResourceHandler::GetMesh(const std::string& id)
    {
        Foundation::ResourceMap::iterator i = meshes_.find(id);
        if (i == meshes_.end())
            return Foundation::ResourcePtr();
        else
            return i->second;
    }

    void ResourceHandler::RemoveMesh(const std::string& id)
    {
        Foundation::ResourceMap::iterator i = meshes_.find(id);
        if (i == meshes_.end())
            return;
        else
            meshes_.erase(i);
    }

    bool ResourceHandler::UpdateMesh(Foundation::AssetPtr source, Core::request_tag_t tag)
    {    
        expected_request_tags_.erase(tag);
            
        // If not found, prepare new
        Foundation::ResourcePtr mesh = GetMesh(source->GetId());
        if (!mesh)
        {
            mesh = meshes_[source->GetId()] = Foundation::ResourcePtr(new OgreMeshResource(source->GetId()));
        }

        bool success = false;
        OgreMeshResource* mesh_res = checked_static_cast<OgreMeshResource*>(mesh.get());

        // If data successfully set, or already have valid data, success (send RESOURCE_READY_EVENT)
        if ((!mesh_res->GetMesh().isNull()) || (mesh_res->SetData(source)))
        {
            const Core::RequestTagVector& tags = request_tags_[source->GetId()];            
            for (Core::uint i = 0; i < tags.size(); ++i)
            {        
                Resource::Events::ResourceReady event_data(mesh->GetId(), mesh, tags[i]);
                framework_->GetEventManager()->SendEvent(resourcecategory_id_, Resource::Events::RESOURCE_READY, &event_data);
            }
            
            success = true;
        }
        
        request_tags_.erase(source->GetId());
        return success;
    }    

    Core::request_tag_t ResourceHandler::RequestMaterial(const std::string& id)
    {
        Core::request_tag_t tag = framework_->GetEventManager()->GetNextRequestTag();
        
        // See if already have the material with data
        Foundation::ResourcePtr material = GetMaterial(id);
        if (material)
        {
            if (!checked_static_cast<OgreMaterialResource*>(material.get())->GetMaterial().isNull())
            {
                Resource::Events::ResourceReady* event_data = new Resource::Events::ResourceReady(material->GetId(), material, tag);
                framework_->GetEventManager()->SendDelayedEvent(resourcecategory_id_, Resource::Events::RESOURCE_READY, Foundation::EventDataPtr(event_data));
                return tag;
            }
        }
        
        // Request from asset system
        Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager(); 
        if (service_manager->IsRegistered(Foundation::Service::ST_Asset))
        {
            boost::shared_ptr<Foundation::AssetServiceInterface> asset_service = service_manager->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();

            // Perform the actual asset request only once, for the first request
            if (request_tags_.find(id) == request_tags_.end())
            {
                Core::request_tag_t source_tag = asset_service->RequestAsset(id, "MaterialScript");
                if (source_tag) 
                {
                    request_tags_[id].push_back(tag);
                    expected_request_tags_.insert(source_tag);
                    return tag;
                }                
            }     
            else
            {
                request_tags_[id].push_back(tag); 
                return tag;
            }                     
        }

        return 0;
    }

    Foundation::ResourcePtr ResourceHandler::GetMaterial(const std::string& id)
    {
        Foundation::ResourceMap::iterator i = materials_.find(id);
        if (i == materials_.end())
            return Foundation::ResourcePtr();
        else
            return i->second;
    }

    void ResourceHandler::RemoveMaterial(const std::string& id)
    {
        Foundation::ResourceMap::iterator i = materials_.find(id);
        if (i == materials_.end())
            return;
        else
            materials_.erase(i);
    }

    bool ResourceHandler::UpdateMaterial(Foundation::AssetPtr source, Core::request_tag_t tag)
    {    
        expected_request_tags_.erase(tag);
            
        // If not found, prepare new
        Foundation::ResourcePtr material = GetMaterial(source->GetId());
        if (!material)
        {
            material = materials_[source->GetId()] = Foundation::ResourcePtr(new OgreMaterialResource(source->GetId()));
        }

        bool success = false;
        OgreMaterialResource* material_res = checked_static_cast<OgreMaterialResource*>(material.get());

        // If data successfully set, or already have valid data, success (send RESOURCE_READY_EVENT)
        if ((!material_res->GetMaterial().isNull()) || (material_res->SetData(source)))
        {
            const Core::RequestTagVector& tags = request_tags_[source->GetId()];            
            for (Core::uint i = 0; i < tags.size(); ++i)
            {        
                Resource::Events::ResourceReady event_data(material->GetId(), material, tags[i]);
                framework_->GetEventManager()->SendEvent(resourcecategory_id_, Resource::Events::RESOURCE_READY, &event_data);
            }
            
            success = true;
        }
        
        request_tags_.erase(source->GetId());
        return success;
    }    
}
