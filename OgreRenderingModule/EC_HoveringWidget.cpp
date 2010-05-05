/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_HoveringWidget.cpp
 *  @brief  EC_HoveringWidget shows a hovering Widget attached to an entity.
 *  @note   The entity must EC_OgrePlaceable available in advance.
 */

#include "StableHeaders.h"
#include "EC_HoveringWidget.h"
#include "ModuleInterface.h"
#include "Renderer.h"
#include "EC_OgrePlaceable.h"
#include "Entity.h"
#include "OgreMaterialUtils.h"

#include <Ogre.h>
#include <OgreBillboardSet.h>
#include <OgreTextureManager.h>
#include <OgreResource.h>

#include <QPushButton>
#include <QFile>
#include <QPainter>
#include <QTimer>
#include <QTimeLine>


//#include "MemoryLeakCheck.h"
namespace OgreRenderer
{

    EC_HoveringWidget::EC_HoveringWidget(Foundation::ModuleInterface* module) :
    Foundation::ComponentInterface(module->GetFramework()),
        billboardSet_(0),
        billboard_(0),
        visibility_animation_timeline_(new QTimeLine(1000, this)),
        visibility_timer_(new QTimer(this)),
        widget_(new HoveringWidgetController()),
        disabled_(false),
        buttons_disabled_(false),
        buttons_visible_(true),
        cam_distance_(0.0f)
    {
        renderer_ = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);

        visibility_animation_timeline_->setFrameRange(0,100);
        visibility_animation_timeline_->setEasingCurve(QEasingCurve::InOutSine);
        visibility_timer_->setSingleShot(true);

        connect(visibility_animation_timeline_, SIGNAL(frameChanged(int)), SLOT(UpdateAnimationStep(int)));
        connect(visibility_animation_timeline_, SIGNAL(finished()), SLOT(AnimationFinished()));
        visibility_timer_->setSingleShot(true);

        connect(visibility_animation_timeline_, SIGNAL(frameChanged(int)),
                SLOT(UpdateAnimationStep(int)));
        connect(visibility_animation_timeline_, SIGNAL(finished()),
                SLOT(AnimationFinished()));
    }

    EC_HoveringWidget::~EC_HoveringWidget()
    {
        SAFE_DELETE(widget_);
    }

    void EC_HoveringWidget::SetPosition(const Vector3df& position)
    {
        if (billboard_)
            billboard_->setPosition(Ogre::Vector3(position.x, position.y, position.z));
    }


    void EC_HoveringWidget::SetButtonsDisabled(bool val)
    {
        buttons_disabled_ = val;
        if(buttons_disabled_)
        {
            ShowButtons(false);
        }
        
    }

    void EC_HoveringWidget::ShowButtons(bool val)
    {
        bool orig = buttons_visible_;
        buttons_visible_ = val;
        if(orig!=buttons_visible_)
        {
            if(buttons_visible_)
            {
                billboard_->setTexcoordRect(0,0.5,1,1);
            }
            else
            {
                billboard_->setTexcoordRect(0,0,1,0.5);
            }
        }

    }

    void EC_HoveringWidget::SetCameraDistance(Real dist)
    {
        cam_distance_ = dist;
        AdjustWidget();
    }
    void EC_HoveringWidget::AdjustWidget()
    {
        if(cam_distance_<10)
        {
            ShowButtons(true);
            if(!IsVisible())
                AnimatedShow();
        }else if(cam_distance_<15)
        {
            ShowButtons(false);
            if(!IsVisible())
                AnimatedShow();

        }else if(cam_distance_>=15)
        {
            if(IsVisible())
                AnimatedHide();
        }

    }


    void EC_HoveringWidget::Show()
    {
        if (billboardSet_ && !disabled_)
            billboardSet_->setVisible(true);
    }

    void EC_HoveringWidget::AnimatedShow()
    {
        if(!disabled_)
        {
            if (visibility_animation_timeline_->state() == QTimeLine::Running ||
                visibility_timer_->isActive() || IsVisible())
                return;

            UpdateAnimationStep(0);
            Show();

            visibility_animation_timeline_->setDirection(QTimeLine::Forward);
            visibility_animation_timeline_->start();
        }
    }

    void EC_HoveringWidget::WidgetClicked(Real x, Real y)
    {
        if(buttons_visible_ && !buttons_disabled_)
            widget_->ForwardMouseClickEvent(x,y);
    }

    void EC_HoveringWidget::EntityClicked(int msec_to_show)
    {
        /*if (visibility_timer_->isActive())
            visibility_timer_->stop();
        else
        {
            AnimatedShow();
            visibility_timer_->start(msec_to_show);
        }*/

        if(IsVisible())
            Hide();
        else
            Show();
    }


    void EC_HoveringWidget::Hide()
    {
        if (billboardSet_)
            billboardSet_->setVisible(false);
    }

    void EC_HoveringWidget::AnimatedHide()
    {
        if (visibility_animation_timeline_->state() == QTimeLine::Running ||
            visibility_timer_->isActive() || !IsVisible())
            return;

        UpdateAnimationStep(100);
        visibility_animation_timeline_->setDirection(QTimeLine::Backward);
        visibility_animation_timeline_->start();
    }

    void EC_HoveringWidget::UpdateAnimationStep(int step)
    {
        if (materialName_.empty())
            return;

        float alpha = step;
        alpha /= 100;

        Ogre::MaterialManager &mgr = Ogre::MaterialManager::getSingleton();
        Ogre::MaterialPtr material = mgr.getByName(materialName_);
        assert(material.get());

        material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setAlphaOperation(
            Ogre::LBX_BLEND_MANUAL, Ogre::LBS_TEXTURE, Ogre::LBS_MANUAL, 1.0, 0.0, alpha);
    }

    void EC_HoveringWidget::AnimationFinished()
    {
        if (visibility_animation_timeline_->direction() == QTimeLine::Backward && IsVisible())
            Hide();
    }

    bool EC_HoveringWidget::IsVisible() const
    {
        if (billboardSet_)
            return billboardSet_->isVisible();
        else
            return false;
    }

    void EC_HoveringWidget::InitializeBillboards()
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

            materialName_ = std::string("material") + renderer_.lock()->GetUniqueObjectName();
            OgreRenderer::CloneMaterial("HoveringText", materialName_);
            billboardSet_->setMaterialName(materialName_);
            billboardSet_->setCastShadows(false);

            billboard_ = billboardSet_->createBillboard(Ogre::Vector3(0, 0, 1.5f));
            assert(billboard_);
            billboardSet_->setDefaultDimensions(1.5f, 0.55f);
            billboard_->setTexcoordRect(0,0,1,0.5);

            sceneNode->attachObject(billboardSet_);
        }
        billboardSet_->setVisible(false);
        Redraw();
    }

    void EC_HoveringWidget::SetText(const QString &text)
    {
        if (text.isNull() || text.isEmpty())
            return;
        widget_->SetText(text);
        Redraw();

    }

    void EC_HoveringWidget::AddButton(QPushButton &button)
    {
            widget_->AddButton(&button);
            Redraw();
    }

    void EC_HoveringWidget::Redraw()
    {
        if (renderer_.expired() || !billboardSet_ || !billboard_)
            return;

        // Get pixmap with text rendered to it.
        QPixmap pixmap = GetPixmap();
        if (pixmap.isNull())
            return;

        // Create texture
        QImage img = pixmap.toImage();
        Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream((void*)img.bits(), img.byteCount()));
        std::string tex_name("HoveringTextTexture" + renderer_.lock()->GetUniqueObjectName());
        Ogre::TextureManager &manager = Ogre::TextureManager::getSingleton();
        Ogre::Texture *tex = checked_static_cast<Ogre::Texture *>(manager.create(
            tex_name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).get());
        assert(tex);
        tex->loadRawData(stream, img.width(), img.height(), Ogre::PF_A8R8G8B8);
        // Set new texture for the material
        assert(!materialName_.empty());
        if (!materialName_.empty())
        {
            Ogre::MaterialManager &mgr = Ogre::MaterialManager::getSingleton();
            Ogre::MaterialPtr material = mgr.getByName(materialName_);
            assert(material.get());
            
            OgreRenderer::SetTextureUnitOnMaterial(material, tex_name);
        }
    }

    QPixmap EC_HoveringWidget::GetPixmap()
    {
    ///\todo Resize the font size according to the render window size and distance
    /// avatar's distance from the camera
    //    const int minWidth =
    //    const int minHeight =
    //    Ogre::Viewport* viewport = renderer_.lock()->GetViewport();
    //    const int max_width = viewport->getActualWidth()/4;
    //    int max_height = viewport->getActualHeight()/10;

        if (renderer_.expired() )
            return 0;

        QRect max_rect(0, 0, 800, 800);
        QRect higher(0,0, max_rect.width(), max_rect.height()/2);
        QRect lower(0,max_rect.height()/2, max_rect.width(), max_rect.height());

        // Create transparent pixmap
        QPixmap pixmap(max_rect.size());
        pixmap.fill(Qt::transparent);
        widget_->HideButtons();
        widget_->setGeometry(higher);
        widget_->setMaximumSize(higher.size());
        widget_->setMinimumSize(higher.size());
        // Init painter with pixmap as the paint device
        QPainter painter(&pixmap);

        widget_->render(&painter, QPoint(0,0), widget_->rect());
        widget_->ShowButtons();

        widget_->render(&painter, lower.topLeft(), widget_->rect());


        return pixmap;
    }
}
