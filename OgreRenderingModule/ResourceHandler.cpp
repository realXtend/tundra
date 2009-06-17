// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AssetEvents.h"
#include "OgreRenderingModule.h"
#include "OgreTextureResource.h"
#include "OgreMeshResource.h"
#include "OgreMaterialResource.h"
#include "OgreParticleResource.h"
#include "ResourceInterface.h"
#include "ResourceHandler.h"
#include "OgreMaterialUtils.h"

namespace OgreRenderer
{
    ResourceHandler::ResourceHandler(Foundation::Framework* framework) :
        framework_(framework)
    {
        source_types_[OgreTextureResource::GetTypeStatic()] = "Texture";
        source_types_[OgreMeshResource::GetTypeStatic()] = "Mesh";
        source_types_[OgreMaterialResource::GetTypeStatic()] = "MaterialScript";
        source_types_[OgreParticleResource::GetTypeStatic()] = "ParticleScript";
    }

    ResourceHandler::~ResourceHandler()
    {
        // Check for still outstanding resource references
        std::map<std::string, Foundation::ResourceReferenceVector>::iterator i = outstanding_references_.begin();
        while (i != outstanding_references_.end())
        {
            Foundation::ResourceReferenceVector& vec = i->second;
            for (Core::uint j = 0; j < vec.size(); ++j)
            {
                OgreRenderingModule::LogDebug("Remaining outstanding reference " + vec[j].id_ + " type " + vec[j].type_ + " for resource " + i->first);
            }
            ++i;
        }
                
        resources_.clear();
    }
    
    void ResourceHandler::PostInitialize()
    {
        Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
        
        resourcecategory_id_ = event_manager->QueryEventCategory("Resource");
    }
    
    Foundation::ResourcePtr ResourceHandler::GetResource(const std::string& id, const std::string& type)
    {
        Foundation::ResourcePtr res = ResourceHandler::GetResourceInternal(id ,type);
        if (!res)
            return res;
        
        // Make sure resource is valid & has no outstanding references, before returning to caller
        if ((!res->IsValid()) || (GetNumOutstandingReferences(res->GetId())))
            return Foundation::ResourcePtr();
            
        return res;
    }

    Foundation::ResourcePtr ResourceHandler::GetResourceInternal(const std::string& id, const std::string& type)
    {
        Foundation::ResourceMap::iterator i = resources_.find(id);
        if (i == resources_.end())
            return Foundation::ResourcePtr();

        // Make sure type matches
        if (i->second->GetType() != type)
        {
            OgreRenderingModule::LogWarning("Requested resource " + id + " with mismatching type " + type);
            return Foundation::ResourcePtr();
        }
        
        return i->second;
    }
    
    Core::request_tag_t ResourceHandler::RequestResource(const std::string& id, const std::string& type)
    {
        if (type == OgreTextureResource::GetTypeStatic())
            return RequestTexture(id);
        if (type == OgreMeshResource::GetTypeStatic() || type == OgreMaterialResource::GetTypeStatic() ||
            type == OgreParticleResource::GetTypeStatic())
            return RequestOtherResource(id, type);
            
        OgreRenderingModule::LogWarning("Requested unknown renderer resource type " + type);
        return 0;
    }
    
    void ResourceHandler::RemoveResource(const std::string& id, const std::string& type)
    {
        Foundation::ResourceMap::iterator i = resources_.find(id);
        if (i == resources_.end())
            return;
        else
        {
            if (i->second->GetType() == type)
                resources_.erase(i);
            else
            {
                OgreRenderingModule::LogWarning("Attempted to remove resource " + id + " with mismatching type " + type + ", real type is " + i->second->GetType());
            }
        }
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

                if (event_data->asset_type_ == "ParticleScript")
                    UpdateParticles(event_data->asset_, event_data->tag_);
            }
            break;
            
            case Asset::Events::ASSET_CANCELED:
            {
                Asset::Events::AssetCanceled *event_data = checked_static_cast<Asset::Events::AssetCanceled*>(data);   
                // Remove client request tags related to this asset, we're not going to get it
                request_tags_.erase(event_data->asset_id_);
                // Check if the asset matches outstanding resource references
                std::map<std::string, Foundation::ResourceReferenceVector>::iterator i = outstanding_references_.begin();
                while (i != outstanding_references_.end())
                {
                    Foundation::ResourceReferenceVector& vec = i->second;
                    for (Core::uint j = 0; j < vec.size(); ++j)
                    {
                        if (vec[j].id_ == event_data->asset_id_)
                        {
                            OgreRenderingModule::LogInfo("Resource " + i->first + " is not going to get referred resource " + event_data->asset_id_ + ", asset transfer canceled");
                        }
                    }
                    ++i;
                }
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
                // This might be for a reference, check it
                ProcessReferenceReady(event_data->resource_, event_data->tag_);
                
                // Check for texture arriving from the texture decoder
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
        Foundation::ResourcePtr tex = GetResource(id, OgreTextureResource::GetTypeStatic());
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

    bool ResourceHandler::UpdateTexture(Foundation::ResourcePtr source, Core::request_tag_t tag)
    {
        Foundation::TexturePtr source_tex = boost::shared_dynamic_cast<Foundation::TextureInterface>(source);
        if (!source_tex) 
            return false;
    
        // If not found, prepare new
        Foundation::ResourcePtr tex = GetResourceInternal(source_tex->GetId(), OgreTextureResource::GetTypeStatic());
        if (!tex)
        {
            tex = Foundation::ResourcePtr(new OgreTextureResource(source_tex->GetId()));
        }

        // If highest level, erase texture decode request tag (should not get more raw resource events for this texture)
        if (source_tex->GetLevel() == 0)
            expected_request_tags_.erase(tag);

        // If success, send Ogre resource ready event
        bool success = false;
        if (checked_static_cast<OgreTextureResource*>(tex.get())->SetData(source_tex))
        {
            resources_[source_tex->GetId()] = tex;
            
            // Create legacy material already here
            // Needed for example with particle scripts, where there is no further code to 
            // create the material/set the texture, because it's all created by Ogre from the script
            Ogre::MaterialPtr mat = GetOrCreateLitTexturedMaterial(source_tex->GetId().c_str());
            SetTextureUnitOnMaterial(mat, source_tex->GetId().c_str());
            
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

    Core::request_tag_t ResourceHandler::RequestOtherResource(const std::string& id, const std::string& type)
    {
        if (source_types_.find(type) == source_types_.end())
            return 0;
    
        Core::request_tag_t tag = framework_->GetEventManager()->GetNextRequestTag();
        
        // See if already have the resource with valid data
        Foundation::ResourcePtr res = GetResource(id, type);
        if (res)
        {
            Resource::Events::ResourceReady* event_data = new Resource::Events::ResourceReady(res->GetId(), res, tag);
            framework_->GetEventManager()->SendDelayedEvent(resourcecategory_id_, Resource::Events::RESOURCE_READY, Foundation::EventDataPtr(event_data));
            return tag;
        }
        
        // Request from asset system
        Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager(); 
        if (service_manager->IsRegistered(Foundation::Service::ST_Asset))
        {
            boost::shared_ptr<Foundation::AssetServiceInterface> asset_service = service_manager->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();

            // Perform the actual asset request only once, for the first request
            if (request_tags_.find(id) == request_tags_.end())
            {
                Core::request_tag_t source_tag = asset_service->RequestAsset(id, source_types_[type]);
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

    bool ResourceHandler::UpdateMesh(Foundation::AssetPtr source, Core::request_tag_t tag)
    {    
        expected_request_tags_.erase(tag);
            
        // If not found, prepare new
        Foundation::ResourcePtr mesh = GetResourceInternal(source->GetId(), OgreMeshResource::GetTypeStatic());
        if (!mesh)
        {
            mesh = Foundation::ResourcePtr(new OgreMeshResource(source->GetId()));
        }

        bool success = false;
        OgreMeshResource* mesh_res = checked_static_cast<OgreMeshResource*>(mesh.get());

        // If data successfully set, or already have valid data, success (send RESOURCE_READY_EVENT)
        if ((mesh_res->IsValid()) || (mesh_res->SetData(source)))
        {
            resources_[source->GetId()] = mesh;
            ProcessResourceReferences(mesh);
            
            success = true;
        }
        
        return success;
    }    

    bool ResourceHandler::UpdateMaterial(Foundation::AssetPtr source, Core::request_tag_t tag)
    {    
        expected_request_tags_.erase(tag);
            
        // If not found, prepare new
        Foundation::ResourcePtr material = GetResourceInternal(source->GetId(), OgreMaterialResource::GetTypeStatic());
        if (!material)
        {
            material = Foundation::ResourcePtr(new OgreMaterialResource(source->GetId()));
        }

        bool success = false;
        OgreMaterialResource* material_res = checked_static_cast<OgreMaterialResource*>(material.get());

        // If data successfully set, or already have valid data, success; check resource references if any
        Core::StringVector tex_names;
        if ((material_res->IsValid()) || (material_res->SetData(source)))
        {
            resources_[source->GetId()] = material;
            ProcessResourceReferences(material);
            
            success = true;
        }

        return success;
    }
    
    bool ResourceHandler::UpdateParticles(Foundation::AssetPtr source, Core::request_tag_t tag)
    {
        expected_request_tags_.erase(tag);
        
        // If not found, prepare new
        Foundation::ResourcePtr particle = GetResourceInternal(source->GetId(), OgreParticleResource::GetTypeStatic());
        if (!particle)
        {
            particle = Foundation::ResourcePtr(new OgreParticleResource(source->GetId()));
        }

        bool success = false;
        OgreParticleResource* particle_res = checked_static_cast<OgreParticleResource*>(particle.get());

        // If data successfully set, or already have valid data, success; check resource references if any
        Core::StringVector tex_names;
        if ((particle_res->IsValid()) || (particle_res->SetData(source)))
        {
            resources_[source->GetId()] = particle;
            ProcessResourceReferences(particle);

            success = true;
        }
        
        return success;
    }
    
    void ResourceHandler::ProcessResourceReferences(Foundation::ResourcePtr resource)
    {
        assert(resource);
        Foundation::ResourceReferenceVector& references = resource->GetReferences();
        
        // If references already processed once & outstanding, do not request again
        if (GetNumOutstandingReferences(resource->GetId()))
            return;
        
        for (Core::uint i = 0; i < references.size(); ++i)
        {
            Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager(); 
            if (service_manager->IsRegistered(Foundation::Service::ST_Asset))
            {
                boost::shared_ptr<Foundation::AssetServiceInterface> asset_service = service_manager->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();
                // Check that the dependency is asset based
                if (asset_service->IsValidId(references[i].id_))
                {
                    // If can get the depended resource directly, OK
                    Foundation::ResourcePtr res = GetResourceInternal(references[i].id_, references[i].type_);
                    // Don't care of the dependency's own outstanding references, assume they're being worked on
                    if ((res) && (res->IsValid()))
                        continue;

                    outstanding_references_[resource->GetId()].push_back(references[i]);
                    Core::request_tag_t tag = RequestResource(references[i].id_, references[i].type_);
                    if (tag)
                        reference_request_tags_[tag] = resource->GetId();
                }
            }
        }
        
        // If no outstanding references, send RESOURCE_READY
        if (!GetNumOutstandingReferences(resource->GetId()))
        {
            const Core::RequestTagVector& tags = request_tags_[resource->GetId()];
            for (Core::uint i = 0; i < tags.size(); ++i)
            {
                Resource::Events::ResourceReady event_data(resource->GetId(), resource, tags[i]);
                framework_->GetEventManager()->SendEvent(resourcecategory_id_, Resource::Events::RESOURCE_READY, &event_data);
            }
            request_tags_.erase(resource->GetId());
        }
    }
    
    //! Processes resource ready for an outstanding reference
    void ResourceHandler::ProcessReferenceReady(Foundation::ResourcePtr resource, Core::request_tag_t tag)
    {
        if (reference_request_tags_.find(tag) != reference_request_tags_.end())
        {
            // The dependent resource must exist & be valid
            Foundation::ResourceMap::iterator i = resources_.find(reference_request_tags_[tag]);
            if ((i == resources_.end()) || (!i->second->IsValid()))
            {
                OgreRenderingModule::LogWarning("Reference resource ready event for nonexisting or invalid resource " + reference_request_tags_[tag]);
            }
            else
            {
                Foundation::ResourcePtr dependent = i->second;
                bool send_ready = false;
                
                if (outstanding_references_.find(dependent->GetId()) != outstanding_references_.end())
                {
                    Foundation::ResourceReferenceVector& refs = outstanding_references_[dependent->GetId()];
                    Foundation::ResourceReferenceVector::iterator j = refs.begin();
                    while (j != refs.end())
                    {
                        if ((j->id_ == resource->GetId()) && (j->type_ == resource->GetType()))
                        {
                            OgreRenderingModule::LogDebug("Resolved reference " + j->id_ + " for resource " + dependent->GetId());
                            refs.erase(j);
                            if (!refs.size())
                            {
                                send_ready = true;
                                outstanding_references_.erase(dependent->GetId());
                            }
                            break;
                        }
                        ++j;
                    }
                    
                    // If no outstanding references any more, send RESOURCE_READY for the dependent resource
                    if (send_ready)
                    {
                        OgreRenderingModule::LogDebug("Last reference, sending RESOURCE_READY for " + dependent->GetId());
                        const Core::RequestTagVector& tags = request_tags_[dependent->GetId()];
                        for (Core::uint i = 0; i < tags.size(); ++i)
                        {
                            Resource::Events::ResourceReady event_data(dependent->GetId(), dependent, tags[i]);
                            framework_->GetEventManager()->SendEvent(resourcecategory_id_, Resource::Events::RESOURCE_READY, &event_data);
                        }
                        request_tags_.erase(dependent->GetId());
                    }
                }
            }
            
            reference_request_tags_.erase(tag);
        }
    }
        
    //! Gets number of outstanding (not yet loaded) references for resource
    unsigned ResourceHandler::GetNumOutstandingReferences(const std::string& id)
    {
        if (outstanding_references_.find(id) == outstanding_references_.end())
            return 0;
        return outstanding_references_[id].size();
    }
    
    bool ResourceHandler::ProcessBraces(const std::string& line, int& braceLevel)
    {
        if (line == "{")
        {
            ++braceLevel;
            return true;
        } 
        else if (line == "}")
        {
            --braceLevel;
            return true;
        }
        else return false;
    }
}
