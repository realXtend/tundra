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
#include <QTimer>

#include <QDebug>

EC_3DCanvas::EC_3DCanvas(Foundation::ModuleInterface *module) :
    Foundation::ComponentInterface(module->GetFramework()),
    framework_(module->GetFramework()),
    entity_(0),
    widget_(0),
    update_internals_(false),
    refresh_timer_(0),
    update_interval_msec_(0)
{
    boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetServiceManager()->
        GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();

    if (renderer)
    {
        // Create material
        material_ = OgreRenderer::GetOrCreateLitTexturedMaterial(renderer->GetUniqueObjectName());
        
        // Create texture
        Ogre::TextureManager &texture_manager = Ogre::TextureManager::getSingleton();
        texture_ = texture_manager.create(renderer->GetUniqueObjectName(),
            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    }
}

EC_3DCanvas::~EC_3DCanvas()
{
    OgreRenderer::RemoveMaterial(material_);
}

void EC_3DCanvas::Start()
{
    update_internals_ = true;
    if (update_interval_msec_ != 0 && refresh_timer_)
        if (!refresh_timer_->isActive())
            refresh_timer_->start(update_interval_msec_);
    else
        Update();
}

void EC_3DCanvas::SetWidget(QWidget *widget)
{
    if (widget_ != widget)
        widget_ = widget;
}

void EC_3DCanvas::SetRefreshRate(int refresh_per_second)
{
    SAFE_DELETE(refresh_timer_);

    if (refresh_per_second != 0)
    {
        update_interval_msec_ = 1000 / refresh_per_second;
        refresh_timer_ = new QTimer(this);
        connect(refresh_timer_, SIGNAL(timeout()), SLOT(Update()));
    }
    else
        update_interval_msec_ = 0;
}

void EC_3DCanvas::SetSubmesh(uint submesh)
{
    submeshes_.clear();
    submeshes_.append(submesh);
    update_internals_ = true;
}

void EC_3DCanvas::SetSubmeshes(const QList<uint> &submeshes)
{
    submeshes_.clear();
    submeshes_ = submeshes;
    update_internals_ = true;
}

void EC_3DCanvas::SetEntity(Scene::Entity *entity)
{
    entity_ = entity;
    update_internals_ = true;
}

void EC_3DCanvas::Update()
{
    if (!widget_ || texture_.isNull())
        return;

    // Uncomment to see the actual page that we take screenshots from
    //widget_->show()
    //qDebug() << endl << ">> Updating mediaurl texture";

    QImage buffer(widget_->size(), QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&buffer);
    widget_->render(&painter);

    Ogre::DataStreamPtr image_data(new Ogre::MemoryDataStream((void*)buffer.bits(), buffer.byteCount()));
    texture_->loadRawData(image_data, buffer.width(), buffer.height(), Ogre::PF_A8R8G8B8);

    // Set texture to material
    if (update_internals_)
    {
        OgreRenderer::SetTextureUnitOnMaterial(material_, texture_->getName());
        UpdateSubmeshes();
        update_internals_ = false;
        qDebug() << endl << ">> EC_3DCanvas: Internals updated";
    }    
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
            {
                ec_mesh->SetMaterial(index, material_name);
                qDebug() << "Setting material " << QString::fromStdString(material_name) << " to submesh " << index;
            }
            else if (ec_mesh->GetMaterialName(index) == material_name)
            {
                if (restore_materials.contains(index))
                {
                     ec_mesh->SetMaterial(index, restore_materials[index]);
                     qDebug() << "Restoring original material " << QString::fromStdString(restore_materials[index]) << " to submesh " << index;
                }
            }
        }
    }
}