/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_HoveringText.cpp
 *  @brief  EC_HoveringText shows a hovering text attached to an entity.
 *  @note   The entity must EC_OgrePlaceable available in advance.
 */

#include "StableHeaders.h"
//#include "DebugOperatorNew.h"
#include "EC_HoveringText.h"
#include "ModuleInterface.h"
#include "Renderer.h"
#include "EC_OgrePlaceable.h"
#include "Entity.h"
#include "OgreMaterialUtils.h"

#include <Ogre.h>
#include <OgreBillboardSet.h>
#include <OgreTextureManager.h>
#include <OgreResource.h>

#include <QFile>
#include <QPainter>
#include <QTimer>
#include <QLinearGradient>

//#include "MemoryLeakCheck.h"

EC_HoveringText::EC_HoveringText(Foundation::ModuleInterface *module) :
    Foundation::ComponentInterface(module->GetFramework()),
    font_(QFont("Arial", 100)),
    backgroundColor_(Qt::transparent),
    textColor_(Qt::black),
    billboardSet_(0),
    billboard_(0),
    text_(""),
    visibility_animation_timeline_(new QTimeLine(1000, this)),
    visibility_timer_(new QTimer(this)),
    using_gradient_(false)
{
    renderer_ = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);

    visibility_animation_timeline_->setFrameRange(0,100);
    visibility_animation_timeline_->setEasingCurve(QEasingCurve::InOutSine);
    visibility_timer_->setSingleShot(true);

    connect(visibility_animation_timeline_, SIGNAL(frameChanged(int)),
            SLOT(UpdateAnimationStep(int)));
    connect(visibility_animation_timeline_, SIGNAL(finished()),
            SLOT(AnimationFinished()));
}

EC_HoveringText::~EC_HoveringText()
{
}

void EC_HoveringText::SetPosition(const Vector3df& position)
{
    if (billboard_)
        billboard_->setPosition(Ogre::Vector3(position.x, position.y, position.z));
}

void EC_HoveringText::SetFont(const QFont &font)
{
    font_ = font;
    Redraw();
}

void EC_HoveringText::SetTextColor(const QColor &color)
{
    textColor_ = color;
    Redraw();
}

void EC_HoveringText::SetBackgroundColor(const QColor &color)
{
    backgroundColor_ = color;
    Redraw();
}

void EC_HoveringText::SetBackgroundGradient(const QColor &color1, const QColor &color2)
{
    bg_grad_.setColorAt(0.0, color1);
    bg_grad_.setColorAt(1.0, color2);
    using_gradient_ = true;
}

void EC_HoveringText::Show()
{
    if (billboardSet_)
        billboardSet_->setVisible(true);
}

void EC_HoveringText::AnimatedShow()
{
    if (visibility_animation_timeline_->state() == QTimeLine::Running ||
        visibility_timer_->isActive() || IsVisible())
        return;

    UpdateAnimationStep(0);
    Show();

    visibility_animation_timeline_->setDirection(QTimeLine::Forward);
    visibility_animation_timeline_->start();
}

void EC_HoveringText::Clicked(int msec_to_show)
{
    if (visibility_timer_->isActive())
        visibility_timer_->stop();
    else
    {
        AnimatedShow();
        visibility_timer_->start(msec_to_show);
    }
}

void EC_HoveringText::Hide()
{
    if (billboardSet_)
        billboardSet_->setVisible(false);
}

void EC_HoveringText::AnimatedHide()
{
    if (visibility_animation_timeline_->state() == QTimeLine::Running ||
        visibility_timer_->isActive() || !IsVisible())
        return;

    UpdateAnimationStep(100);
    visibility_animation_timeline_->setDirection(QTimeLine::Backward);
    visibility_animation_timeline_->start();
} 

void EC_HoveringText::UpdateAnimationStep(int step)
{
    if (!material_.get())
        return;
    
    float alpha = step;
    alpha /= 100;
    material_->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setAlphaOperation(
        Ogre::LBX_BLEND_MANUAL, Ogre::LBS_TEXTURE, Ogre::LBS_MANUAL, 1.0, 0.0, alpha);
}

void EC_HoveringText::AnimationFinished()
{
    if (visibility_animation_timeline_->direction() == QTimeLine::Backward && IsVisible())
        Hide();
} 

bool EC_HoveringText::IsVisible() const
{
    if (billboardSet_)
        return billboardSet_->isVisible();
    else
        return false;
}

void EC_HoveringText::ShowMessage(const QString &text)
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

        billboard_ = billboardSet_->createBillboard(Ogre::Vector3(0, 0, 0.7f));
        assert(billboard_);
        billboard_->setDimensions(2, 1);

        sceneNode->attachObject(billboardSet_);
    }

    if (text.isNull() || text.isEmpty())
        return;

    text_ = text;
    Redraw();
}

void EC_HoveringText::Redraw()
{
    if (renderer_.expired() || !billboardSet_ || !billboard_)
        return;

    // Get pixmap with text rendered to it.
    QPixmap pixmap = GetTextPixmap();
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

QPixmap EC_HoveringText::GetTextPixmap()
{
// TOODO
// Resize the font size according to the render window size and distance
// avatar's distance from the camera
//
//    const int minWidth =
//    const int minHeight =
//    Ogre::Viewport* viewport = renderer_.lock()->GetViewport();
//    const int max_width = viewport->getActualWidth()/4;
//    int max_height = viewport->getActualHeight()/10;

    if (renderer_.expired() || text_.isEmpty() || text_ == " ")
        return 0;

    QRect max_rect(0, 0, 1500, 800);

    // Create transparent pixmap
    QPixmap pixmap(max_rect.size());
    pixmap.fill(Qt::transparent);

    // Init painter with pixmap as the paint device
    QPainter painter(&pixmap);

    // Ask painter the rect for the text
    painter.setFont(font_);
    QRect rect = painter.boundingRect(max_rect, Qt::AlignCenter | Qt::TextWordWrap, text_);
    
    // Add some padding to it
    QFontMetrics metric(font_); 
    int width = metric.width(text_) + metric.averageCharWidth();
    int height = metric.height() + 20;
    rect.setWidth(width);
    rect.setHeight(height);

    // Set background brush
    if (using_gradient_)
    {
        bg_grad_.setStart(QPointF(0,rect.top()));
        bg_grad_.setFinalStop(QPointF(0,rect.bottom()));
        painter.setBrush(QBrush(bg_grad_));
    }
    else
        painter.setBrush(backgroundColor_);

    // Draw background rect
    painter.setPen(QColor(255,255,255,150));
    painter.drawRoundedRect(rect, 20.0, 20.0);

    // Draw text
    painter.setPen(textColor_);
    painter.drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, text_);

    return pixmap;
}

