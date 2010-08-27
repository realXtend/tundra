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

#include <QWidget>
#include <QPainter>

#include <QDebug>

EC_3DCanvas::EC_3DCanvas(Foundation::ModuleInterface *module) :
    Foundation::ComponentInterface(module->GetFramework()),
    widget_(0),
    update_internals_(false),
    refresh_timer_(0),
    update_interval_msec_(0),
    material_name_(""),
    texture_name_("")
{
    boost::shared_ptr<OgreRenderer::Renderer> renderer = module->GetFramework()->GetServiceManager()->
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
        Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().createManual(
            texture_name_, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            Ogre::TEX_TYPE_2D, 1, 1, 0, Ogre::PF_A8R8G8B8, 
            Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
        if (texture.isNull())
            texture_name_ = "";
    }
}

EC_3DCanvas::~EC_3DCanvas()
{
    submeshes_.clear();
    widget_ = 0;

    if (refresh_timer_)
        refresh_timer_->stop();
    SAFE_DELETE(refresh_timer_);

    if (!material_name_.empty())
    {
        try
        {
            Ogre::MaterialManager::getSingleton().remove(material_name_);
        }
        catch (...) {}
    }

    if (!texture_name_.empty())
    {
        try
        {
            Ogre::TextureManager::getSingleton().remove(texture_name_);
        }
        catch (...) {}
    }
}

void EC_3DCanvas::Start()
{
    update_internals_ = true;
    if (update_interval_msec_ != 0 && refresh_timer_)
    {
        if (refresh_timer_->isActive())
            refresh_timer_->stop();
        refresh_timer_->start(update_interval_msec_);
    }
    else
        Update();
}

void EC_3DCanvas::Setup(QWidget *widget, const QList<uint> &submeshes, int refresh_per_second)
{
    SetWidget(widget);
    SetSubmeshes(submeshes);
    SetRefreshRate(refresh_per_second);
}

void EC_3DCanvas::SetWidget(QWidget *widget)
{
    if (widget_ != widget)
    {
        widget_ = widget;
        if (widget_)
            connect(widget_, SIGNAL(destroyed(QObject*)), SLOT(WidgetDestroyed(QObject *)));
    }
}

void EC_3DCanvas::SetRefreshRate(int refresh_per_second)
{
    if (refresh_per_second < 0)
        refresh_per_second = 0;

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

void EC_3DCanvas::WidgetDestroyed(QObject *obj)
{
    widget_ = 0;
    if (refresh_timer_)
        refresh_timer_->stop();
    SAFE_DELETE(refresh_timer_);
}

void EC_3DCanvas::Update()
{
    if (!widget_ || texture_name_.empty())
        return;

    Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().getByName(texture_name_);
    if (texture.isNull())
        return;

    QImage buffer(widget_->size(), QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&buffer);
    widget_->render(&painter);

    // Set texture to material
    if (update_internals_ && !material_name_.empty())
    {
        Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName(material_name_);
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

void EC_3DCanvas::UpdateSubmeshes()
{
    Scene::Entity* entity = GetParentEntity();
    
    if (material_name_.empty() || !entity)
        return;

    int draw_type = -1;
    uint submesh_count = 0;
    OgreRenderer::EC_OgreMesh* ec_mesh = entity->GetComponent<OgreRenderer::EC_OgreMesh>().get();
    OgreRenderer::EC_OgreCustomObject* ec_custom_object = entity->GetComponent<OgreRenderer::EC_OgreCustomObject>().get();

    if (ec_mesh)
    {
        draw_type = RexTypes::DRAWTYPE_MESH;
        submesh_count = ec_mesh->GetNumMaterials();
    }
    else if (ec_custom_object)
    {
        draw_type = RexTypes::DRAWTYPE_PRIM;
        submesh_count = ec_custom_object->GetNumMaterials();
    }

    if (draw_type == -1)
        return;

    // Iterate trough sub meshes
    for (uint index = 0; index < submesh_count; ++index)
    {
        // Store original materials
        std::string submesh_material_name;
        if (draw_type == RexTypes::DRAWTYPE_MESH)
            submesh_material_name = ec_mesh->GetMaterialName(index);
        else if (draw_type == RexTypes::DRAWTYPE_PRIM)
            submesh_material_name = ec_custom_object->GetMaterialName(index);
        else
            return;

        if (submesh_material_name != material_name_)
            restore_materials_[index] = submesh_material_name;

        if (submeshes_.contains(index))
        {
            if (draw_type == RexTypes::DRAWTYPE_MESH)
                ec_mesh->SetMaterial(index, material_name_);
            else if (draw_type == RexTypes::DRAWTYPE_PRIM)
                ec_custom_object->SetMaterial(index, material_name_);
            else
                return;
        }
        else 
        {
            // If submesh not contained, restore the original material
            if (draw_type == RexTypes::DRAWTYPE_MESH)
            {
                if (ec_mesh->GetMaterialName(index) == material_name_)
                    if (restore_materials_.contains(index))
                        ec_mesh->SetMaterial(index, restore_materials_[index]);
            }
            else if(draw_type == RexTypes::DRAWTYPE_PRIM)
            {
                if (ec_custom_object->GetMaterialName(index) == material_name_)
                    if (restore_materials_.contains(index))
                        ec_custom_object->SetMaterial(index, restore_materials_[index]);
            }
            else 
                return;
        }
    }
}
