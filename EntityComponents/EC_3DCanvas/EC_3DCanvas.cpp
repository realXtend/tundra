// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_3DCanvas.h"

#include "Renderer.h"
#include "ModuleInterface.h"
#include "OgreMaterialUtils.h"
#include "Entity.h"

#include "EC_OgreMesh.h"
#include "EC_OgreCustomObject.h"

#include <Ogre.h>
#include <OgreTextureManager.h>

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QMap>

#include <QDebug>

EC_3DCanvas::EC_3DCanvas(Foundation::ModuleInterface *module) :
    Foundation::ComponentInterface(module->GetFramework()),
    framework_(module->GetFramework()),
    entity_(0),
    widget_(0),
    texture_set_(false)
{
    boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetServiceManager()->
        GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();

    if (renderer)
    {
        // Create material
        material_ = OgreRenderer::GetOrCreateLitTexturedMaterial(renderer->GetUniqueObjectName());
        
        // Create texture
        Ogre::TextureManager &texture_manager = Ogre::TextureManager::getSingleton();
        std::string texture_name = renderer->GetUniqueObjectName();
        texture_ = static_cast<Ogre::Texture *>(texture_manager.create(texture_name,
            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).get());
    }
}

EC_3DCanvas::~EC_3DCanvas()
{
    OgreRenderer::RemoveMaterial(material_);
}

void EC_3DCanvas::SetWidget(QWidget *widget)
{
    widget_ = widget;
}

void EC_3DCanvas::SetSubmesh(uint submesh)
{
    submeshes_.clear();
    submeshes_.append(submesh);
    UpdateSubmeshes();
}

void EC_3DCanvas::SetSubmeshes(const QList<uint> &submeshes)
{
    submeshes_.clear();
    submeshes_ = submeshes;
    UpdateSubmeshes();
}

void EC_3DCanvas::SetEntity(Scene::Entity *entity)
{
    entity_ = entity;
    UpdateSubmeshes();
}

void EC_3DCanvas::Update()
{
    if (!widget_ || !texture_)
        return;

    QImage buffer(widget_->size(), QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&buffer);
    widget_->render(&painter);

    Ogre::DataStreamPtr image_data(new Ogre::MemoryDataStream((void*)buffer.bits(), buffer.byteCount()));
    texture_->loadRawData(image_data, buffer.width(), buffer.height(), Ogre::PF_A8R8G8B8);

    // Set texture to material
    if (!texture_set_)
    {
        OgreRenderer::SetTextureUnitOnMaterial(material_, texture_->getName());
        texture_set_ = !texture_set_;
    }

    UpdateSubmeshes();
}

void EC_3DCanvas::UpdateSubmeshes()
{
    if (material_.isNull() || !entity_ || submeshes_.count() == 0)
        return;

    QMap<uint, std::string> restore_materials;
    std::string material_name = material_->getName();
    bool apply_material;

    OgreRenderer::EC_OgreMesh* ec_mesh = entity_->GetComponent<OgreRenderer::EC_OgreMesh>().get();
    OgreRenderer::EC_OgreCustomObject* ec_custom_object = entity_->GetComponent<OgreRenderer::EC_OgreCustomObject>().get();
    
    if (ec_mesh)
    {
        // Iterate trough sub meshes
        uint submesh_count = ec_mesh->GetNumMaterials();
        for (uint index = 0; index < submesh_count; ++index)
        {
            // Store original materials
            std::string submesh_material_name = ec_mesh->GetMaterialName(index);
            if (submesh_material_name != material_name)
                restore_materials[index] = submesh_material_name;

            apply_material = false;
            foreach(uint apply_index, submeshes_)
                if (apply_index == index)
                    apply_material = true;

            if (apply_material)
                ec_mesh->SetMaterial(index, material_name);
            else if (ec_mesh->GetMaterialName(index) == material_name)
                if (restore_materials.contains(index))
                     ec_mesh->SetMaterial(index, restore_materials[index]);
        }
    }
}