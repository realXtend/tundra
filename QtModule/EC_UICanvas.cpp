// For conditions of distribution and use, see copyright notice in license.txt
#include "StableHeaders.h"

#include "EC_UICanvas.h"

#include "EC_OgreMesh.h"
#include "EC_OgreCustomObject.h"
#include "Renderer.h"
#include "OgreMaterialUtils.h"
#include "QtModule.h"
#include "SceneManager.h"

namespace QtUI
{
    EC_UICanvas::EC_UICanvas(Foundation::ModuleInterface* module) : 
        Foundation::ComponentInterface(module->GetFramework()),
        entity_(0)
    {
        Foundation::Framework* framework = module->GetFramework();
        
        boost::shared_ptr<OgreRenderer::Renderer> renderer = framework->GetServiceManager()->
            GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        if (renderer)        
        {
            material_ = OgreRenderer::GetOrCreateLitTexturedMaterial(renderer->GetUniqueObjectName());
        }
    }
    
    EC_UICanvas::~EC_UICanvas()
    {
        // Note: entity may no longer be valid at this point, we don't know really since we only have a raw pointer
        // so can't restore the original submesh materials
        OgreRenderer::RemoveMaterial(material_);
    }
    
    void EC_UICanvas::SetEntity(Scene::Entity* entity)
    {
        entity_ = entity;
        
        Refresh();
    }
    
    void EC_UICanvas::SetCanvas(boost::shared_ptr<UICanvas> canvas)
    {
        if (material_.isNull())
            return;
        
        UICanvas* canvasptr = canvas.get();
        if (!canvasptr)
        {
            QtModule::LogError("Null UI canvas");
            return;
        }
        if (canvasptr->GetDisplayMode() == UICanvas::External)
        {
            QtModule::LogError("Can not use an external canvas for EC_UICanvas");
            return;
        }
        
        canvas_ = canvas;
     
        // Assign canvas texture to our material
        std::string texname = "tex" + canvasptr->GetID().toStdString();
        OgreRenderer::SetTextureUnitOnMaterial(material_, texname);
        
        Refresh();
    }
   
    void EC_UICanvas::SetSubmeshes(const std::vector<uint>& submeshes)
    {
        submeshes_ = submeshes;
        if (!submeshes_.size())
            canvas_.reset();
            
        Refresh();
    }

    void EC_UICanvas::SetSubmeshes(uint submesh)
    {
        submeshes_.clear();
        submeshes_.push_back(submesh);
        
        Refresh();
    }
    
    void EC_UICanvas::ClearSubmeshes()
    {
        submeshes_.clear();
        canvas_.reset();

        Refresh();
    }

    void EC_UICanvas::Refresh()
    {
        if (material_.isNull())
            return;
        if (!entity_)
            return;

        OgreRenderer::EC_OgreMesh* meshptr = entity_->GetComponent<OgreRenderer::EC_OgreMesh>().get();
        OgreRenderer::EC_OgreCustomObject* customptr = entity_->GetComponent<OgreRenderer::EC_OgreCustomObject>().get();
        
        // If have both a mesh & custom object, too weird to handle. Abort.
        if (meshptr && customptr)
            return;
        
        std::string mat_name = material_->getName();
        
        // Set material to select submeshes of EC_OgreMesh
        if (meshptr)
        {        
                         
            original_materials_.resize(meshptr->GetNumMaterials());
            for (uint i = 0; i < meshptr->GetNumMaterials(); ++i)
            {
                // If this submesh does not have the UICanvas material yet, store the material for restoring later
                {
                    std::string orig_mat_name = meshptr->GetMaterialName(i);
                    if (orig_mat_name != mat_name)
                        original_materials_[i] = orig_mat_name;
                }
                
                // Now see if this submesh should have the UICanvas material
                bool set_uicanvas = false;
                for (uint j = 0; j < submeshes_.size(); ++j)
                    if (submeshes_[j] == i) set_uicanvas = true;
               
                if (set_uicanvas)
                    meshptr->SetMaterial(i, mat_name);
                else
                {
                    // Restore original if we know it
                    if (meshptr->GetMaterialName(i) == mat_name)
                    {
                        if (!original_materials_[i].empty())
                            meshptr->SetMaterial(i, original_materials_[i]);
                    }
                }
            }
        }    
        
        // Set material to select submeshes of EC_OgreCustomObject
        if (customptr)
        {
            // If custom object does not exist as actual committed geometry yet, can't do anything
            if (!customptr->IsCommitted())
                return;
                                  
            original_materials_.resize(customptr->GetNumMaterials());
            for (uint i = 0; i < customptr->GetNumMaterials(); ++i)
            {
                // If this submesh does not have the UICanvas material yet, store the material for restoring later
                {
                    std::string orig_mat_name = customptr->GetMaterialName(i);
                    if (orig_mat_name != mat_name)
                        original_materials_[i] = orig_mat_name;
                }
                
                // Now see if this submesh should have the UICanvas material
                bool set_uicanvas = false;
                for (uint j = 0; j < submeshes_.size(); ++j)
                    if (submeshes_[j] == i) set_uicanvas = true;
               
                if (set_uicanvas)
                    customptr->SetMaterial(i, mat_name);
                else
                {
                    // Restore original if we know it
                    if (customptr->GetMaterialName(i) == mat_name)
                    {
                        if (!original_materials_[i].empty())
                            customptr->SetMaterial(i, original_materials_[i]);
                    }
                }
            }
        }
    }
}