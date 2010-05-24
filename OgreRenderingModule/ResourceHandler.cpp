// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AssetEvents.h"
#include "OgreRenderingModule.h"
#include "OgreImageTextureResource.h"
#include "OgreTextureResource.h"
#include "OgreMeshResource.h"
#include "OgreMaterialResource.h"
#include "OgreParticleResource.h"
#include "OgreSkeletonResource.h"
#include "ResourceInterface.h"
#include "ResourceHandler.h"
#include "OgreMaterialUtils.h"
#include "RexTypes.h"
#include "TextureServiceInterface.h"
#include "AssetServiceInterface.h"
#include "Framework.h"
#include "EventManager.h"
#include "ServiceManager.h"

namespace OgreRenderer
{
    ResourceHandler::ResourceHandler(Foundation::Framework* framework) :
        framework_(framework)
    {
        source_types_[OgreTextureResource::GetTypeStatic()] = RexTypes::ASSETTYPENAME_TEXTURE;
        source_types_[OgreMeshResource::GetTypeStatic()] = RexTypes::ASSETTYPENAME_MESH;
        source_types_[OgreSkeletonResource::GetTypeStatic()] = RexTypes::ASSETTYPENAME_SKELETON;
        source_types_[OgreMaterialResource::GetTypeStatic()] = RexTypes::ASSETTYPENAME_MATERIAL_SCRIPT;
        source_types_[OgreParticleResource::GetTypeStatic()] = RexTypes::ASSETTYPENAME_PARTICLE_SCRIPT;
        source_types_[OgreImageTextureResource::GetTypeStatic()] = RexTypes::ASSETTYPENAME_IMAGE;
    }

    ResourceHandler::~ResourceHandler()
    {
        // Check for still outstanding resource references
        std::map<std::string, Foundation::ResourceReferenceVector>::iterator i = outstanding_references_.begin();
        while (i != outstanding_references_.end())
        {
            Foundation::ResourceReferenceVector& vec = i->second;
            for (uint j = 0; j < vec.size(); ++j)
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
        resource_event_category_ = event_manager->QueryEventCategory("Resource");
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
            // If it's OgreTexture instead of OgreImageTexture, allow that
            if ((i->second->GetType() == OgreTextureResource::GetTypeStatic()) && (type == OgreImageTextureResource::GetTypeStatic()))
                return i->second;
            // If it's OgreImageTexture instead of OgreTexture, allow that
            if ((i->second->GetType() == OgreImageTextureResource::GetTypeStatic()) && (type == OgreTextureResource::GetTypeStatic()))
                return i->second;
                
            OgreRenderingModule::LogWarning("Requested resource " + id + " with mismatching type " + type + ", correct type would be " + i->second->GetType());
            return Foundation::ResourcePtr();
        }
        
        return i->second;
    }
    
    request_tag_t ResourceHandler::RequestResource(const std::string& id, const std::string& type)
    {
        if (type == OgreTextureResource::GetTypeStatic())
            return RequestTexture(id);
        if (type == OgreMeshResource::GetTypeStatic() || type == OgreMaterialResource::GetTypeStatic() ||
            type == OgreParticleResource::GetTypeStatic() || type == OgreImageTextureResource::GetTypeStatic() ||
            type == OgreSkeletonResource::GetTypeStatic())
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
    
    bool ResourceHandler::HandleAssetEvent(event_id_t event_id, Foundation::EventDataInterface* data)
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

                if (event_data->asset_type_ == RexTypes::ASSETTYPENAME_MESH)
                    UpdateMesh(event_data->asset_, event_data->tag_);

                if (event_data->asset_type_ == RexTypes::ASSETTYPENAME_SKELETON)
                    UpdateSkeleton(event_data->asset_, event_data->tag_);

                if (event_data->asset_type_ == RexTypes::ASSETTYPENAME_MATERIAL_SCRIPT)
                    UpdateMaterial(event_data->asset_, event_data->tag_);

                if (event_data->asset_type_ == RexTypes::ASSETTYPENAME_PARTICLE_SCRIPT)
                    UpdateParticles(event_data->asset_, event_data->tag_);

                if (event_data->asset_type_ == RexTypes::ASSETTYPENAME_IMAGE)
                    UpdateImageTexture(event_data->asset_, event_data->tag_);
            }
            break;
            
            case Asset::Events::ASSET_CANCELED:
            {
                Asset::Events::AssetCanceled *event_data = checked_static_cast<Asset::Events::AssetCanceled*>(data);
                // Send a RESOURCE_CANCELED event for each request that was made for this asset, then clear the tags
                const RequestTagVector& tags = request_tags_[event_data->asset_id_];
                for (uint i = 0; i < tags.size(); ++i)
                {
                    Resource::Events::ResourceCanceled canceled_event_data(event_data->asset_id_, tags[i]);
                    framework_->GetEventManager()->SendEvent(resource_event_category_, Resource::Events::RESOURCE_CANCELED, &canceled_event_data);
                }
                request_tags_.erase(event_data->asset_id_);
                
                // Check if the asset matches outstanding resource references
                std::map<std::string, Foundation::ResourceReferenceVector>::iterator i = outstanding_references_.begin();
                while (i != outstanding_references_.end())
                {
                    Foundation::ResourceReferenceVector& vec = i->second;
                    for (uint j = 0; j < vec.size(); ++j)
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

    bool ResourceHandler::HandleResourceEvent(event_id_t event_id, Foundation::EventDataInterface* data)
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

    request_tag_t ResourceHandler::RequestTexture(const std::string& id)
    {
        request_tag_t tag = framework_->GetEventManager()->GetNextRequestTag();
            
        // See if already have the texture and at maximum quality level
        Foundation::ResourcePtr tex = GetResource(id, OgreTextureResource::GetTypeStatic());
        if (tex)
        {
            if (checked_static_cast<OgreTextureResource*>(tex.get())->GetLevel() == 0)
            {
                Resource::Events::ResourceReady* event_data = new Resource::Events::ResourceReady(tex->GetId(), tex, tag);
                framework_->GetEventManager()->SendDelayedEvent(resource_event_category_, Resource::Events::RESOURCE_READY, Foundation::EventDataPtr(event_data));
                return tag;
            }
        }

        // Request from texture decoder
        Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager(); 
        boost::shared_ptr<Foundation::TextureServiceInterface> texture_service = service_manager->GetService<Foundation::TextureServiceInterface>(Foundation::Service::ST_Texture).lock();            
        if (texture_service)
        {
            // Perform the actual decode request only once, for the first request
            if (request_tags_.find(id) == request_tags_.end())
            {
                request_tag_t source_tag = texture_service->RequestTexture(id);
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

    bool ResourceHandler::UpdateTexture(Foundation::ResourcePtr source, request_tag_t tag)
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
            
            // Create legacy material(s) based on the texture
            CreateLegacyMaterials(source_tex->GetId(), true);
            
            const RequestTagVector& tags = request_tags_[source_tex->GetId()];
            for (uint i = 0; i < tags.size(); ++i)
            {
                Resource::Events::ResourceReady event_data(tex->GetId(), tex, tags[i]);
                framework_->GetEventManager()->SendEvent(resource_event_category_, Resource::Events::RESOURCE_READY, &event_data);
            }
           
            success = true;
        }

        // If highest level, erase also request tags 
        if (source_tex->GetLevel() == 0)
            request_tags_.erase(source_tex->GetId());

        return success;
    }    

    request_tag_t ResourceHandler::RequestOtherResource(const std::string& id, const std::string& type)
    {
        if (source_types_.find(type) == source_types_.end())
            return 0;
    
        request_tag_t tag = framework_->GetEventManager()->GetNextRequestTag();
        
        // See if already have the resource with valid data
        Foundation::ResourcePtr res = GetResource(id, type);
        if (res)
        {
            Resource::Events::ResourceReady* event_data = new Resource::Events::ResourceReady(res->GetId(), res, tag);
            framework_->GetEventManager()->SendDelayedEvent(resource_event_category_, Resource::Events::RESOURCE_READY, Foundation::EventDataPtr(event_data));
            return tag;
        }
        
        // Request from asset system
        Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager(); 
        boost::shared_ptr<Foundation::AssetServiceInterface> asset_service = service_manager->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();
        if (asset_service)
        {
            // Perform the actual asset request only once, for the first request
            if (request_tags_.find(id) == request_tags_.end())
            {
                request_tag_t source_tag = asset_service->RequestAsset(id, source_types_[type]);
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

    bool ResourceHandler::UpdateMesh(Foundation::AssetPtr source, request_tag_t tag)
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

    bool ResourceHandler::UpdateImageTexture(Foundation::AssetPtr source, request_tag_t tag)
    {    
        expected_request_tags_.erase(tag);
            
        // If not found, prepare new
        Foundation::ResourcePtr tex = GetResourceInternal(source->GetId(), OgreImageTextureResource::GetTypeStatic());
        if (!tex)
        {
            tex = Foundation::ResourcePtr(new OgreImageTextureResource(source->GetId()));
        }

        bool success = false;
        OgreImageTextureResource* tex_res = checked_static_cast<OgreImageTextureResource*>(tex.get());

        // If data successfully set, or already have valid data, success (send RESOURCE_READY_EVENT)
        if ((tex_res->IsValid()) || (tex_res->SetData(source)))
        {
            resources_[source->GetId()] = tex;
            ProcessResourceReferences(tex);
            
            success = true;
        }
        
        return success;
    }

    bool ResourceHandler::UpdateMaterial(Foundation::AssetPtr source, request_tag_t tag)
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
        StringVector tex_names;
        if ((material_res->IsValid()) || (material_res->SetData(source)))
        {
            resources_[source->GetId()] = material;
            ProcessResourceReferences(material);
            
            success = true;
        }

        return success;
    }
    
    bool ResourceHandler::UpdateParticles(Foundation::AssetPtr source, request_tag_t tag)
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
        StringVector tex_names;
        if ((particle_res->IsValid()) || (particle_res->SetData(source)))
        {
            resources_[source->GetId()] = particle;
            ProcessResourceReferences(particle);

            success = true;
        }
        
        return success;
    }
    
    bool ResourceHandler::UpdateSkeleton(Foundation::AssetPtr source, request_tag_t tag)
    {    
        expected_request_tags_.erase(tag);
            
        // If not found, prepare new
        Foundation::ResourcePtr skeleton = GetResourceInternal(source->GetId(), OgreSkeletonResource::GetTypeStatic());
        if (!skeleton)
        {
            skeleton = Foundation::ResourcePtr(new OgreSkeletonResource(source->GetId()));
        }

        bool success = false;
        OgreSkeletonResource* skeleton_res = checked_static_cast<OgreSkeletonResource*>(skeleton.get());

        // If data successfully set, or already have valid data, success (send RESOURCE_READY_EVENT)
        if ((skeleton_res->IsValid()) || (skeleton_res->SetData(source)))
        {
            resources_[source->GetId()] = skeleton;
            ProcessResourceReferences(skeleton);
            
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
        
        for (uint i = 0; i < references.size(); ++i)
        {
            Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager(); 
            boost::shared_ptr<Foundation::AssetServiceInterface> asset_service = service_manager->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();
            if (asset_service)
            {
                // Check that the dependency is asset based
                if (asset_service->IsValidId(references[i].id_, references[i].type_))
                {
                    // If can get the depended resource directly, OK
                    Foundation::ResourcePtr res = GetResourceInternal(references[i].id_, references[i].type_);
                    // Don't care of the dependency's own outstanding references, assume they're being worked on
                    if ((res) && (res->IsValid()))
                        continue;

                    outstanding_references_[resource->GetId()].push_back(references[i]);
                    request_tag_t tag = RequestResource(references[i].id_, references[i].type_);
                    if (tag)
                        reference_request_tags_[tag] = resource->GetId();
                }
            }
        }
        
        // If no outstanding references, send RESOURCE_READY
        if (!GetNumOutstandingReferences(resource->GetId()))
        {
            const RequestTagVector& tags = request_tags_[resource->GetId()];
            for (uint i = 0; i < tags.size(); ++i)
            {
                Resource::Events::ResourceReady event_data(resource->GetId(), resource, tags[i]);
                framework_->GetEventManager()->SendEvent(resource_event_category_, Resource::Events::RESOURCE_READY, &event_data);
            }
            request_tags_.erase(resource->GetId());
        }
    }
    
    //! Processes resource ready for an outstanding reference
    void ResourceHandler::ProcessReferenceReady(Foundation::ResourcePtr resource, request_tag_t tag)
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
                        const RequestTagVector& tags = request_tags_[dependent->GetId()];
                        for (uint i = 0; i < tags.size(); ++i)
                        {
                            Resource::Events::ResourceReady event_data(dependent->GetId(), dependent, tags[i]);
                            framework_->GetEventManager()->SendEvent(resource_event_category_, Resource::Events::RESOURCE_READY, &event_data);
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
