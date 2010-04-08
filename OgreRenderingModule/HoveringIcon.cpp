
/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   HoveringIcon.cpp
 *  @brief  HoveringIcon shows a hovering text attached to an entity.
 *  @note   The entity must EC_OgrePlaceable available in advance.
 */

#include "StableHeaders.h"
#include "HoveringIcon.h"
#include "Renderer.h"
#include "EC_OgrePlaceable.h"
#include "Entity.h"
#include "OgreMaterialUtils.h"

#include <OgreBillboardSet.h>
#include <OgreTextureManager.h>
#include <OgreResource.h>

#include <QFile>
#include <QPainter>
#include <QTimer>
#include <QLinearGradient>
#include "EC_HoveringIconGroup.h"

//#include "MemoryLeakCheck.h"

HoveringIcon::HoveringIcon(Foundation::Framework *framework, QString& pic, Ogre::Vector3& vec,EC_HoveringIconGroup* grp) :
    pos_(vec),
    billboardSet_(0),
    billboard_(0),
    pixmap_(pic),
    visibility_animation_timeline_(new QTimeLine(1000, this)),
    visibility_timer_(new QTimer(this)),
    framework_(framework),
    group_(grp)
{
    renderer_ = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);

    visibility_animation_timeline_->setFrameRange(0,100);
    visibility_animation_timeline_->setEasingCurve(QEasingCurve::InOutSine);
    visibility_timer_->setSingleShot(true);

    connect(visibility_animation_timeline_, SIGNAL(frameChanged(int)),
            SLOT(UpdateAnimationStep(int)));
    connect(visibility_animation_timeline_, SIGNAL(finished()),
            SLOT(AnimationFinished()));

    InitializeBillboards();
}

Scene::Entity* HoveringIcon::GetParentEntity()
{
    return group_->GetParentEntity();
}

void HoveringIcon::InitializeBillboards()
{
    if (renderer_.expired())
        return;

    Ogre::SceneManager *scene = renderer_.lock()->GetSceneManager();
    assert(scene);
    if (!scene)
        return;

    Scene::Entity *entity = GetParentEntity();
    assert(entity);
    if (!entity)
        return;

    OgreRenderer::EC_OgrePlaceable *node = entity->GetComponent<OgreRenderer::EC_OgrePlaceable>().get();
    if (!node)
        return;

    Ogre::SceneNode *sceneNode = node->GetSceneNode();
    assert(sceneNode);
    if (!sceneNode)
        return;

    // Create billboard if it doesn't exist.
    if (!billboardSet_ && !billboard_)
    {
        billboardSet_ = scene->createBillboardSet(renderer_.lock()->GetUniqueObjectName(), 1);
        assert(billboardSet_);


        std::string newName = std::string("material") + renderer_.lock()->GetUniqueObjectName(); 
        material_ = OgreRenderer::CloneMaterial("HoveringText", newName);
        billboardSet_->setMaterialName(newName);
        billboardSet_->setCastShadows(false);


        billboard_ = billboardSet_->createBillboard(pos_);
        assert(billboard_);
        billboardSet_->setDefaultDimensions(group_->GetIconWidth(),group_->GetIconHeight());
        

        sceneNode->attachObject(billboardSet_);
        
    }
    Redraw();
    Hide();

}

HoveringIcon::~HoveringIcon()
{
}

void HoveringIcon::SetPosition(const Vector3df& position)
{
    if (billboard_)
        billboard_->setPosition(Ogre::Vector3(position.x, position.y, position.z));
}





void HoveringIcon::Show()
{
    if (billboardSet_)
        billboardSet_->setVisible(true);
}

void HoveringIcon::AnimatedShow()
{
    if (visibility_animation_timeline_->state() == QTimeLine::Running ||
        visibility_timer_->isActive() || IsVisible())
        return;

    UpdateAnimationStep(0);
    Show();

    visibility_animation_timeline_->setDirection(QTimeLine::Forward);
    visibility_animation_timeline_->start();
}

void HoveringIcon::SetVisible(bool val)
{
    if(val)
    {
        Show();
    }
    else
    {
        Hide();
    }
}

void HoveringIcon::Hide()
{
    if (billboardSet_)
        billboardSet_->setVisible(false);
}

void HoveringIcon::AnimatedHide()
{
    if (visibility_animation_timeline_->state() == QTimeLine::Running ||
        visibility_timer_->isActive() || !IsVisible())
        return;

    UpdateAnimationStep(100);
    visibility_animation_timeline_->setDirection(QTimeLine::Backward);
    visibility_animation_timeline_->start();
} 

void HoveringIcon::UpdateAnimationStep(int step)
{
    if (!material_.get())
        return;
    
    float alpha = step;
    alpha /= 100;
    material_->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setAlphaOperation(
        Ogre::LBX_BLEND_MANUAL, Ogre::LBS_TEXTURE, Ogre::LBS_MANUAL, 1.0, 0.0, alpha);
}

void HoveringIcon::AnimationFinished()
{
    if (visibility_animation_timeline_->direction() == QTimeLine::Backward && IsVisible())
        Hide();
} 

bool HoveringIcon::IsVisible() const
{
    if (billboardSet_)
        return billboardSet_->isVisible();
    else
        return false;
}



void HoveringIcon::Redraw()
{
    if (renderer_.expired() || !billboardSet_ || !billboard_)
        return;

    // Get pixmap with text rendered to it.
    QPixmap pixmap = pixmap_;
    if (pixmap.isNull())
        return;

    // Create texture
    QImage img = pixmap.toImage();
    Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream((void*)img.bits(), img.byteCount()));
    std::string tex_name("HoveringTextTexture" + renderer_.lock()->GetUniqueObjectName());
    Ogre::TextureManager &manager = Ogre::TextureManager::getSingleton();
    Ogre::Texture *tex = dynamic_cast<Ogre::Texture *>(manager.create(tex_name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).get());
    assert(tex);

    tex->loadRawData(stream, img.width(), img.height(), Ogre::PF_A8R8G8B8);

    // Set new material with the new texture name in it.
    std::string newMatName = std::string("material") + renderer_.lock()->GetUniqueObjectName(); 
    material_ = OgreRenderer::CloneMaterial("HoveringText", newMatName);
    OgreRenderer::SetTextureUnitOnMaterial(material_, tex_name);
    billboardSet_->setMaterialName(newMatName);
}


