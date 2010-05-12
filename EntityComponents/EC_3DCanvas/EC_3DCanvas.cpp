// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_3DCanvas.h"

#include "Renderer.h"
#include "ModuleInterface.h"
#include "OgreMaterialUtils.h"
#include "Entity.h"
#include "RexTypes.h"

#include "EC_OgreMesh.h"
#include "EC_OgreCustomObject.h"
#include "EC_OpenSimPrim.h"

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
    paint(false),
    refresh_timer_(0),
    update_interval_msec_(0),
    material_name_(""),
    texture_name_(""),
    material_manager_(Ogre::MaterialManager::getSingleton()),
    texture_manager_(Ogre::TextureManager::getSingleton())
{
    boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetServiceManager()->
        GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();

    if (renderer)
    {
        // Create material
        material_name_ = "EC3DCanvasMaterial" + renderer->GetUniqueObjectName();
        Ogre::MaterialPtr material = OgreRenderer::GetOrCreateLitTexturedMaterial(material_name_);
        if (material.isNull())
            material_name_ = "";

        // Create texture
        texture_name_ = "EC3DCanvasTexture" + renderer->GetUniqueObjectName();
        Ogre::TexturePtr texture = texture_manager_.createManual(texture_name_, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                                 Ogre::TEX_TYPE_2D, 1, 1, 0, Ogre::PF_A8R8G8B8, 
                                                                 Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
        if (texture.isNull())
            texture_name_ = "";

        paint = true;
    }
}

EC_3DCanvas::~EC_3DCanvas()
{
    paint = false;
    widget_ = 0;

    if (refresh_timer_)
        refresh_timer_->stop();
    SAFE_DELETE(refresh_timer_);

    if (!material_name_.empty())
    {
        Ogre::MaterialPtr material = material_manager_.getByName(material_name_);
        OgreRenderer::RemoveMaterial(material);
    }
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
    if (!paint)
        return; 

    if (!widget_ || texture_name_.empty())
        return;

    Ogre::TexturePtr texture = texture_manager_.getByName(texture_name_);
    if (!texture.isNull())
    {

        QImage buffer(widget_->size(), QImage::Format_ARGB32_Premultiplied);
        QPainter painter(&buffer);
        widget_->render(&painter);

        // Set texture to material
        if (update_internals_ && !material_name_.empty())
        {
            Ogre::MaterialPtr material = material_manager_.getByName(material_name_);
            if (material.isNull())
                return;
            OgreRenderer::SetTextureUnitOnMaterial(material, texture_name_);
            UpdateSubmeshes();
            update_internals_ = false;
        }

        if (texture->getWidth() != buffer.width() || texture->getHeight() != buffer.height())
        {
            texture->freeInternalResources();
            texture->setWidth(buffer.width());
            texture->setHeight(buffer.height());
            texture->createInternalResources();
        }

        Ogre::Box update_box(0,0, buffer.width(), buffer.height());
        Ogre::PixelBox pixel_box(update_box, Ogre::PF_A8R8G8B8, (void*)buffer.bits());
        texture->getBuffer()->blitFromMemory(pixel_box, update_box);
    }
}

void EC_3DCanvas::UpdateSubmeshes()
{
    if (material_name_.empty() || !entity_ || submeshes_.count() == 0)
        return;

    QMap<uint, std::string> restore_materials;
    bool apply_material;

    int draw_type = -1;
    uint submesh_count = 0;
    OgreRenderer::EC_OgreMesh* ec_mesh = entity_->GetComponent<OgreRenderer::EC_OgreMesh>().get();
    OgreRenderer::EC_OgreCustomObject* ec_custom_object = entity_->GetComponent<OgreRenderer::EC_OgreCustomObject>().get();
    
    //qDebug() << "EC_3DCanvas::UpdateSubmeshes() =====================";

    if (ec_mesh)
    {
        //qDebug() << "Type MESH";
        draw_type = RexTypes::DRAWTYPE_MESH;
        submesh_count = ec_mesh->GetNumMaterials();
    }
    else if (ec_custom_object)
    {
        //qDebug() << "Type CUSTOM OBJECT";
        draw_type = RexTypes::DRAWTYPE_PRIM;
        submesh_count = ec_custom_object->GetNumMaterials();
    }
    if (draw_type == -1)
        return;

    //qDebug() << "Submesh count      : " << submesh_count;
    //qDebug() << "Apply to submeshes : " << submeshes_;

    // Iterate trough sub meshes
    for (uint index = 0; index < submesh_count; ++index)
    {
        //qDebug() << ">> Checking index " << index;

        // Store original materials
        std::string submesh_material_name;
        if (draw_type == RexTypes::DRAWTYPE_MESH)
            submesh_material_name = ec_mesh->GetMaterialName(index);
        else if (draw_type == RexTypes::DRAWTYPE_PRIM)
            submesh_material_name = ec_custom_object->GetMaterialName(index);
        else
            return;

        if (submesh_material_name != material_name_)
            restore_materials[index] = submesh_material_name;

        if (submeshes_.contains(index))
        {
            //qDebug() << ">> Applying to index " << index << " - " << QString::fromStdString(material_name_);

            if (draw_type == RexTypes::DRAWTYPE_MESH)
                ec_mesh->SetMaterial(index, material_name_);
            else if (draw_type == RexTypes::DRAWTYPE_PRIM)
                ec_custom_object->SetMaterial(index, material_name_);
            else
                return;
        }
        else 
        {
            if (draw_type == RexTypes::DRAWTYPE_MESH)
            {
                if (ec_mesh->GetMaterialName(index) == material_name_)
                    if (restore_materials.contains(index))
                        ec_mesh->SetMaterial(index, restore_materials[index]);
            }
            else if(draw_type == RexTypes::DRAWTYPE_PRIM)
            {
                if (ec_custom_object->GetMaterialName(index) == material_name_)
                    if (restore_materials.contains(index))
                        ec_custom_object->SetMaterial(index, restore_materials[index]);
            }
            else return;
        }
    }
}