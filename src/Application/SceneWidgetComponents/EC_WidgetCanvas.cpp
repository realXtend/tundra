// For conditions of distribution and use, see copyright notice in LICENSE

#include "Math/MathNamespace.h"

#include "DebugOperatorNew.h"
#include "EC_WidgetCanvas.h"

#include "Framework.h"
#include "IRenderer.h"
#include "Entity.h"
#include "LoggingFunctions.h"

#include "OgreMaterialUtils.h"
#include "EC_Mesh.h"
#include "EC_OgreCustomObject.h"

#include <OgreTextureManager.h>
#include <OgreMaterialManager.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreTechnique.h>

#include <QTimer>
#include <QWidget>
#include <QPainter>
#include <QDebug>

#if defined(DIRECTX_ENABLED) && defined(WIN32)
#ifdef SAFE_DELETE
#undef SAFE_DELETE
#endif
#ifdef SAFE_DELETE_ARRAY
#undef SAFE_DELETE_ARRAY
#endif
#include <d3d9.h>
#include <OgreD3D9RenderSystem.h>
#include <OgreD3D9HardwarePixelBuffer.h>
#endif

#include "MemoryLeakCheck.h"

EC_WidgetCanvas::EC_WidgetCanvas(Scene *scene) :
    IComponent(scene),
    widget_(0),
    update_internals_(false),
    mesh_hooked_(false),
    refresh_timer_(0),
    update_interval_msec_(0),
    material_name_(""),
    texture_name_("")
{
    if (framework->IsHeadless())
        return;
	
	if (framework->Renderer())
    {
        // Create texture
        texture_name_ = framework->Renderer()->GetUniqueObjectName("EC_3DCanvas_tex");
        Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().createManual(
            texture_name_, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            Ogre::TEX_TYPE_2D, 1, 1, 0, Ogre::PF_A8R8G8B8, 
            Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);
        if (texture.isNull())
        {
            LogError("EC_WidgetCanvas: Could not create texture for usage!");
            return;
        }

        // Create material: Make sure we have one tech with one pass with one texture unit.
        // Don't use our lit textured templates here as emissive will not work there as it has vertex etc programs in it.
        material_name_ = framework->Renderer()->GetUniqueObjectName("EC_3DCanvas_mat");
        Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create(material_name_, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        if (material->getNumTechniques() == 0)
            material->createTechnique();
        if (material->getTechnique(0) && 
            material->getTechnique(0)->getNumPasses() == 0)
            material->getTechnique(0)->createPass();
        if (material->getTechnique(0)->getPass(0) && 
            material->getTechnique(0)->getPass(0)->getNumTextureUnitStates() == 0)
            material->getTechnique(0)->getPass(0)->createTextureUnitState(texture_name_);        
    }

    connect(this, SIGNAL(ParentEntitySet()), SLOT(ParentEntitySet()), Qt::UniqueConnection);
}

EC_WidgetCanvas::~EC_WidgetCanvas()
{
    if (framework->IsHeadless())
        return;

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

void EC_WidgetCanvas::Start()
{
    if (framework->IsHeadless())
        return;

    update_internals_ = true;
    if (update_interval_msec_ != 0 && refresh_timer_)
    {
        if (refresh_timer_->isActive())
            refresh_timer_->stop();
        refresh_timer_->start(update_interval_msec_);
    }
    else
        Update();

    if(!mesh_hooked_)
    {
        Entity* entity = ParentEntity();
        EC_Mesh* ec_mesh = entity->GetComponent<EC_Mesh>().get();
        if (ec_mesh)
        {
            connect(ec_mesh, SIGNAL(MaterialChanged(uint, const QString)), SLOT(MeshMaterialsUpdated(uint, const QString)));
            mesh_hooked_ = true;
        }
    }
}

void EC_WidgetCanvas::MeshMaterialsUpdated(uint index, const QString &material_name)
{
    if (framework->IsHeadless())
        return;

    if (material_name_.empty())
        return;
    if(material_name.compare(QString(material_name_.c_str())) != 0 )
    {
        bool has_index = false;
        for (int i=0; i<submeshes_.length(); i++)
        {
            if (index == submeshes_[i])
                has_index = true;
        }
        if(has_index)
            UpdateSubmeshes();
    }
}

void EC_WidgetCanvas::Stop()
{
    if (framework->IsHeadless())
        return;

    if (refresh_timer_)
        if (refresh_timer_->isActive())
            refresh_timer_->stop();
}

void EC_WidgetCanvas::Setup(QWidget *widget, const QList<uint> &submeshes, int refresh_per_second)
{
    if (framework->IsHeadless())
        return;

    SetWidget(widget);
    SetSubmeshes(submeshes);
    SetRefreshRate(refresh_per_second);
}

void EC_WidgetCanvas::SetWidget(QWidget *widget)
{
    if (framework->IsHeadless())
        return;

    if (widget_ != widget)
    {
        widget_ = widget;
        if (widget_)
            connect(widget_, SIGNAL(destroyed(QObject*)), SLOT(WidgetDestroyed(QObject *)), Qt::UniqueConnection);
    }
}

void EC_WidgetCanvas::SetRefreshRate(int refresh_per_second)
{
    if (framework->IsHeadless())
        return;

    if (refresh_per_second < 0)
        refresh_per_second = 0;

    bool was_running = false;
    if (refresh_timer_)
        was_running = refresh_timer_->isActive();
    SAFE_DELETE(refresh_timer_);

    if (refresh_per_second != 0)
    {
        refresh_timer_ = new QTimer(this);
        connect(refresh_timer_, SIGNAL(timeout()), SLOT(Update()), Qt::UniqueConnection);

        int temp_msec = 1000 / refresh_per_second;
        if (update_interval_msec_ != temp_msec && was_running)
            refresh_timer_->start(temp_msec);
        update_interval_msec_ = temp_msec;
    }
    else
        update_interval_msec_ = 0;
}

void EC_WidgetCanvas::SetSubmesh(uint submesh)
{
    if (framework->IsHeadless())
        return;

    submeshes_.clear();
    submeshes_.append(submesh);
    update_internals_ = true;
    UpdateSubmeshes();
}

void EC_WidgetCanvas::SetSubmeshes(const QList<uint> &submeshes)
{
    if (framework->IsHeadless())
        return;

    submeshes_.clear();
    submeshes_ = submeshes;
    update_internals_ = true;
    UpdateSubmeshes();
}

void EC_WidgetCanvas::WidgetDestroyed(QObject *obj)
{
    if (framework->IsHeadless())
        return;

    widget_ = 0;
    Stop();
    RestoreOriginalMeshMaterials();
    SAFE_DELETE(refresh_timer_);
}

void EC_WidgetCanvas::Update(QImage buffer)
{
    if (framework->IsHeadless())
        return;
    if (buffer.width() <= 0 || buffer.height() <= 0)
        return;

    if (buffer.format() != QImage::Format_ARGB32 && buffer.format() != QImage::Format_ARGB32_Premultiplied)
    {
        LogWarning("EC_WidgetCanvas::Update(QImage buffer): Input format needs to be Format_ARGB32 or Format_ARGB32_Premultiplied, preforming auto conversion!");
        buffer = buffer.convertToFormat(QImage::Format_ARGB32);
        if (buffer.isNull())
        {
            LogError("-- Auto conversion failed, not updating!");
            return;
        }
    }

    try
    {
        Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().getByName(texture_name_);
        if (texture.isNull())
            return;

        // Set texture to material if need be
        if (update_internals_ && !material_name_.empty())
        {
            Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName(material_name_);
            if (material.isNull())
                return;
            // Just for good measure, this is done once in the ctor already if everything went well.
            OgreRenderer::SetTextureUnitOnMaterial(material, texture_name_);
            UpdateSubmeshes();
            update_internals_ = false;
        }

        if ((int)texture->getWidth() != buffer.width() || (int)texture->getHeight() != buffer.height())
        {
            texture->freeInternalResources();
            texture->setWidth(buffer.width());
            texture->setHeight(buffer.height());
            texture->createInternalResources();
        }

        Blit(buffer, texture);
    }
    catch (Ogre::Exception &e) // inherits std::exception
    {
        LogError("Exception occurred while blitting texture data from memory: " + std::string(e.what()));
    }
    catch (...)
    {
        LogError("Unknown exception occurred while blitting texture data from memory.");
    }
}

void EC_WidgetCanvas::Update()
{
    if (framework->IsHeadless())
        return;

    if (!widget_.data() || texture_name_.empty())
        return;
    if (widget_->width() <= 0 || widget_->height() <= 0)
        return;

    try
    {
        Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().getByName(texture_name_);
        if (texture.isNull())
            return;

        if (buffer_.size() != widget_->size())
            buffer_ = QImage(widget_->size(), QImage::Format_ARGB32_Premultiplied);
        if (buffer_.width() <= 0 || buffer_.height() <= 0)
            return;

        QPainter painter(&buffer_);
        widget_->render(&painter);

        // Set texture to material
        if (update_internals_ && !material_name_.empty())
        {
            Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName(material_name_);
            if (material.isNull())
                return;
            // Just for good measure, this is done once in the ctor already if everything went well.
            OgreRenderer::SetTextureUnitOnMaterial(material, texture_name_);
            UpdateSubmeshes();
            update_internals_ = false;
        }

        if ((int)texture->getWidth() != buffer_.width() || (int)texture->getHeight() != buffer_.height())
        {
            texture->freeInternalResources();
            texture->setWidth(buffer_.width());
            texture->setHeight(buffer_.height());
            texture->createInternalResources();
        }

        Blit(buffer_, texture);
    }
    catch (Ogre::Exception &e) // inherits std::exception
    {
        LogError("Exception occurred while blitting texture data from memory: " + std::string(e.what()));
    }
    catch (...)
    {
        LogError("Unknown exception occurred while blitting texture data from memory.");
    }
}

bool EC_WidgetCanvas::Blit(const QImage &source, Ogre::TexturePtr destination)
{
#if defined(DIRECTX_ENABLED) && defined(WIN32)
    Ogre::HardwarePixelBufferSharedPtr pb = destination->getBuffer();
    Ogre::D3D9HardwarePixelBuffer *pixelBuffer = dynamic_cast<Ogre::D3D9HardwarePixelBuffer*>(pb.get());
    if (!pixelBuffer)
        return false;

    LPDIRECT3DSURFACE9 surface = pixelBuffer->getSurface(Ogre::D3D9RenderSystem::getActiveD3D9Device());
    if (surface)
    {
        D3DSURFACE_DESC desc;
        HRESULT hr = surface->GetDesc(&desc);
        if (SUCCEEDED(hr))
        {
            D3DLOCKED_RECT lock;
            HRESULT hr = surface->LockRect(&lock, 0, 0);
            if (SUCCEEDED(hr))
            {
                const int bytesPerPixel = 4; ///\todo Count from Ogre::PixelFormat!
                const int sourceStride = bytesPerPixel * source.width();
                if (lock.Pitch == sourceStride)
                    memcpy(lock.pBits, source.bits(), sourceStride * source.height());
                else
                    for(int y = 0; y < source.height(); ++y)
                        memcpy((u8*)lock.pBits + lock.Pitch * y, source.bits() + sourceStride * y, sourceStride);
                surface->UnlockRect();
            }
        }
    }
#else
    if (!destination->getBuffer().isNull())
    {
        Ogre::Box update_box(0, 0, source.width(), source.height());
        Ogre::PixelBox pixel_box(update_box, Ogre::PF_A8R8G8B8, (void*)source.bits());
        destination->getBuffer()->blitFromMemory(pixel_box, update_box);
    }
#endif

    return true;
}

void EC_WidgetCanvas::UpdateSubmeshes()
{
    if (framework->IsHeadless())
        return;

    Entity* entity = ParentEntity();
    
    if (material_name_.empty() || !entity)
        return;

    uint submesh_count = 0;
    EC_Mesh* ec_mesh = entity->GetComponent<EC_Mesh>().get();
    EC_OgreCustomObject* ec_custom_object = entity->GetComponent<EC_OgreCustomObject>().get();

    if (ec_mesh)
        submesh_count = ec_mesh->GetNumMaterials();
    else if (ec_custom_object)
        submesh_count = ec_custom_object->GetNumMaterials();
    else
    {
        ///\todo Log out error - entity doesn't contain either a EC_Mesh or EC_OgreCustomObject!
        return;
    }

    // Iterate trough sub meshes
    for(uint index = 0; index < submesh_count; ++index)
    {
        // Store original materials
        std::string submesh_material_name;
        if (ec_mesh)
            submesh_material_name = ec_mesh->GetMaterialName(index);
        else
            submesh_material_name = ec_custom_object->GetMaterialName(index);

        if (submesh_material_name != material_name_)
            restore_materials_[index] = submesh_material_name;

        if (submeshes_.contains(index))
        {
            if (ec_mesh)
                ec_mesh->SetMaterial(index, QString::fromStdString(material_name_));
            else
                ec_custom_object->SetMaterial(index, material_name_);
        }
        else 
        {
            // If submesh not contained, restore the original material
            if (ec_mesh)
            {
                if (ec_mesh->GetMaterialName(index) == material_name_)
                    if (restore_materials_.contains(index))
                        ec_mesh->SetMaterial(index, QString::fromStdString(restore_materials_[index]));
            }
            else
            {
                if (ec_custom_object->GetMaterialName(index) == material_name_)
                    if (restore_materials_.contains(index))
                        ec_custom_object->SetMaterial(index, restore_materials_[index]);
            }
        }
    }
}

void EC_WidgetCanvas::RestoreOriginalMeshMaterials()
{
    if (framework->IsHeadless())
        return;

    if (restore_materials_.empty())
    {
        update_internals_ = true;
        return;
    }

    Entity* entity = ParentEntity();

    if (material_name_.empty() || !entity)
        return;

    uint submesh_count = 0;
    EC_Mesh* ec_mesh = entity->GetComponent<EC_Mesh>().get();
    EC_OgreCustomObject* ec_custom_object = entity->GetComponent<EC_OgreCustomObject>().get();

    if (ec_mesh)
        submesh_count = ec_mesh->GetNumMaterials();
    else if (ec_custom_object)
        submesh_count = ec_custom_object->GetNumMaterials();
    else
    {
        ///\todo Log out error - entity doesn't contain either a EC_Mesh or EC_OgreCustomObject!
        return;
    }

    // Iterate trough sub meshes
    for(uint index = 0; index < submesh_count; ++index)
    {
        // If submesh not contained, restore the original material
        if (ec_mesh)
        {
            if (ec_mesh->GetMaterialName(index) == material_name_)
                if (restore_materials_.contains(index))
                    ec_mesh->SetMaterial(index, QString::fromStdString(restore_materials_[index]));
        }
        else
        {
            if (ec_custom_object->GetMaterialName(index) == material_name_)
                if (restore_materials_.contains(index))
                    ec_custom_object->SetMaterial(index, restore_materials_[index]);
        }
    }

    restore_materials_.clear();
    update_internals_ = true;
}

void EC_WidgetCanvas::ParentEntitySet()
{
    if (framework->IsHeadless())
        return;

    // Warn users if they are creating this component with sync enabled.
    /// \todo Maybe this component should not be a component anymore as components can no longer be non-serializable. Maybe these rendering operations should be moved to some util code inside SceneWidgetComponents.
    if (IsReplicated())
        LogWarning("EC_WidgetCanvas: My network sync seems to be enabled, this is not advisable in most situations! I don't have any attributes and am intended for local rendering operations.");

    if (ParentEntity())
        connect(ParentEntity(), SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)), SLOT(ComponentRemoved(IComponent*, AttributeChange::Type)), Qt::UniqueConnection);
}

void EC_WidgetCanvas::ComponentRemoved(IComponent *component, AttributeChange::Type change)
{
    if (framework->IsHeadless())
        return;

    if (component == this)
    {
        Stop();
        RestoreOriginalMeshMaterials();
        SetWidget(0);
        submeshes_.clear();
    }
}

void EC_WidgetCanvas::SetSelfIllumination(bool illuminating)
{
    if (material_name_.empty())
        return;

    Ogre::ColourValue emissiveColor;
    if (illuminating)
        emissiveColor = Ogre::ColourValue(1.0f, 1.0f, 1.0f, 1.0f);
    else
        emissiveColor = Ogre::ColourValue(0.0f, 0.0f, 0.0f, 0.0f);

    Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName(material_name_);
    if (!material.isNull())
    {
        Ogre::Material::TechniqueIterator iter = material->getTechniqueIterator();
        while(iter.hasMoreElements())
        {
            Ogre::Technique *tech = iter.getNext();
            if (!tech)
                continue;
            Ogre::Technique::PassIterator passIter = tech->getPassIterator();
            while(passIter.hasMoreElements())
            {
                Ogre::Pass *pass = passIter.getNext();
                if (pass)
                    pass->setSelfIllumination(emissiveColor);
            }
        }
    }
}
