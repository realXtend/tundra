// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EC_BillboardWidget.h"
#include <QPainter>
#include <QWidget>
#include <QImage>
#include <Ogre.h>
#include <OgreBillboardSet.h>
#include <OgreTextureManager.h>
#include <OgreResource.h>
#include "OgreMaterialUtils.h"
#include "EC_Placeable.h"
#include "Renderer.h"
#include "Entity.h"

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("EC_BillboardWidget")

#include "MemoryLeakCheck.h"

#include <QDebug>

EC_BillboardWidget::EC_BillboardWidget(IModule *module):
    IComponent(module->GetFramework()),
    widget_(0),
    billboard_set_(0),
    billboard_(0),
    width_(1),
    height_(1),
    position_(0,0,0),
    visible_(true)
{
    renderer_ = GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Service::ST_Renderer);
}

EC_BillboardWidget::~EC_BillboardWidget()
{
    UninitializeOgreBillboard();
}

void EC_BillboardWidget::SetWidget(QWidget* widget)
{
    widget_ = widget;
    Refresh();
}

QWidget* EC_BillboardWidget::GetWidget()
{
    return widget_;
}

void EC_BillboardWidget::SetPosition(Vector3df& pos)
{
    SetPosition(pos.x, pos.y, pos.z);
}

void EC_BillboardWidget::SetPosition(float x, float y, float z)
{
    position_.x = x; position_.y = y; position_.z = z;
    if (billboard_)
        billboard_->setPosition(Ogre::Vector3(position_.x, position_.y, position_.z));
}

void EC_BillboardWidget::SetSize(float width, float height)
{
    width_ = width;
    height_ = height;
    if (billboard_)
        billboard_->setDimensions(width_, height_); 
}

void EC_BillboardWidget::SetWidth(float width)
{
    width_ = width;
    if (billboard_)
        billboard_->setDimensions(width_, height_); 
}

void EC_BillboardWidget::SetHeight(float height)
{
    height_ = height;
    if (billboard_)
        billboard_->setDimensions(width_, height_); 
}

void EC_BillboardWidget::InitializeOgreBillboard()
{
    if (renderer_.expired())
        return;

    Ogre::SceneManager *ogre_scene = renderer_.lock()->GetSceneManager();
    assert(ogre_scene);
    if (!ogre_scene)
        return;

    Scene::Entity *entity = GetParentEntity();
    assert(entity);
    if (!entity)
        return;

    EC_Placeable *node = entity->GetComponent<EC_Placeable>().get();
    if (!node)
        return;

    Ogre::SceneNode *sceneNode = node->GetSceneNode();
    assert(sceneNode);
    if (!sceneNode)
        return;

    // Create billboard if it doesn't exist.
    if (!billboard_set_ && !billboard_ )
    {
        billboard_set_ = ogre_scene->createBillboardSet(renderer_.lock()->GetUniqueObjectName("EC_BillboardWidget_bbset"), 1);
        assert(billboard_set_);

        material_name_ = std::string("material")+ std::string("name") + renderer_.lock()->GetUniqueObjectName("EC_BillboardWidget_mat");
        OgreRenderer::CloneMaterial("HoveringText", material_name_); // ???

        billboard_set_->setMaterialName(material_name_);
        billboard_set_->setCastShadows(false);
   //     billboard_set_->setBillboardType(Ogre::BBT_ORIENTED_COMMON);
        billboard_set_->setCommonUpVector(Ogre::Vector3::UNIT_Z);
        billboard_ = billboard_set_->createBillboard(position_);
        assert(billboard_);
        billboard_->setDimensions(width_, height_); 
        sceneNode->attachObject(billboard_set_);
    }

    billboard_set_->setVisible(false);
    Refresh();
}

void EC_BillboardWidget::UninitializeOgreBillboard()
{
    widget_ = 0;

    if (renderer_.expired())
        return;

    boost::shared_ptr<OgreRenderer::Renderer> renderer = renderer_.lock();

    if (texture_name_.size() > 0)
    {
        Ogre::TextureManager::getSingleton().remove(texture_name_);
        texture_name_ = "";
    }

    if (material_name_.size() > 0)
    {
        Ogre::MaterialManager::getSingleton().remove(material_name_);
        material_name_ = "";
    }

	if (billboard_set_ && billboard_)
    {
		billboard_set_->removeBillboard(billboard_);
        billboard_ = 0;
    }

	if (billboard_set_)
	{
		Ogre::SceneManager* scene = renderer->GetSceneManager();
		if (scene)
        {
			scene->destroyBillboardSet(billboard_set_);
            billboard_set_ = 0;
        }
    }
}

void EC_BillboardWidget::Refresh()
{
    if (!widget_)
    {
        UninitializeOgreBillboard();
        return;
    }

    if (!billboard_set_)
        InitializeOgreBillboard();

    if (renderer_.expired() || !billboard_set_ || !billboard_)
        return;

    QPixmap pixmap(widget_->size());
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    widget_->render(&painter, QPoint(0,0), widget_->rect(), QWidget::DrawChildren );
    QImage image = pixmap.toImage();

    Ogre::TextureManager &texture_manager = Ogre::TextureManager::getSingleton();
    Ogre::TexturePtr texture;
    try
    {
        // If texture don't exist, create them now.
        if (texture_name_.empty())
        {
            texture_name_ = "billboardTexture" + renderer_.lock()->GetUniqueObjectName("EC_BillboardWidget_bbtex");
            texture = texture_manager.createManual(texture_name_, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                Ogre::TEX_TYPE_2D, image.width(), image.height(), Ogre::MIP_DEFAULT, Ogre::PF_A8R8G8B8, Ogre::TU_DEFAULT);

            if (texture.isNull())
            {
                std::cout << "Failed to create texture " << texture_name_ << std::endl;
                return;
            }
        }
        else
        {
            // Textures already exists. Re-use them.
            texture = texture_manager.getByName(texture_name_);
            assert(!texture.isNull());

            // See if size/format changed, have to delete/recreate internal resources
            if (image.width() != (int)texture->getWidth() || image.height() != (int)texture->getHeight())
            {
                texture->freeInternalResources();
                texture->setWidth(image.width());
                texture->setHeight(image.height());
                texture->setFormat(Ogre::PF_A8R8G8B8);
                texture->createInternalResources();
            }
        }

        Ogre::Box dimensions(0,0, image.width(), image.height());
        Ogre::PixelBox pixel_box(dimensions, Ogre::PF_A8R8G8B8, (void*)image.bits());
        if (!texture->getBuffer().isNull())
            texture->getBuffer()->blitFromMemory(pixel_box);
    }
    catch (Ogre::Exception &e)
    {
        std::cout << "Failed to create textures: " << std::string(e.what()) << std::endl;
        return;
    }

    // Set new texture for the material
    if (!material_name_.empty())
    {
        Ogre::MaterialManager &material_manager = Ogre::MaterialManager::getSingleton();
        Ogre::MaterialPtr material = material_manager.getByName(material_name_);
        material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureFiltering(Ogre::TFO_ANISOTROPIC);
        assert(material.get());
        OgreRenderer::SetTextureUnitOnMaterial(material, texture_name_);
    }
    
    billboard_set_->setVisible(true);
}

void EC_BillboardWidget::SetVisible(bool visible)
{
    visible_ = visible;
    billboard_set_->setVisible(visible_);
}
