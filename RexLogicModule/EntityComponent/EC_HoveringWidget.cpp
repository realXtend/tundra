/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_HoveringWidget.cpp
 *  @brief  EC_HoveringWidget shows a hovering widget attached to an entity.
 *  @note   The entity must have EC_OgrePlaceable component available in advance.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EC_HoveringWidget.h"
#include "HoveringNameController.h"
#include "HoveringButtonsController.h"
#include "DetachedWidgetController.h"

#include "IModule.h"
#include "Renderer.h"
#include "EC_OgrePlaceable.h"
#include "Entity.h"
#include "OgreMaterialUtils.h"
#include "UiServiceInterface.h"
#include "UiProxyWidget.h"
#include "ModuleManager.h"

#include <Ogre.h>
#include <OgreBillboardSet.h>
#include <OgreTextureManager.h>
#include <OgreResource.h>

#include <QGraphicsScene>
#include <QPushButton>
#include <QPainter>

#include "MemoryLeakCheck.h"

namespace RexLogic
{
    EC_HoveringWidget::EC_HoveringWidget(IModule* module) :
        IComponent(module->GetFramework()),
        namebillboardSet_(0),
        buttonsbillboardSet_(0),
        namebillboard_(0),
        buttonsbillboard_(0),
        visibility_animation_timeline_(new QTimeLine(1000, this)),
        visibility_timer_(new QTimer(this)),
        namewidget_(new HoveringNameController()),
        detachedwidget_(new DetachedWidgetController()),
        buttonswidget_(new HoveringButtonsController()),
        hovering_time_(4000),
        disabled_(false),
        buttons_disabled_(false),
        buttons_visible_(false),
        detached_(false),
        bb_name_size_view(0.25f, 0.08f),
        bb_buttons_size_view(0.1f, 0.25f),
        bb_rel_posy(1.3f),
        cam_distance_(0.0f)
    {
        hovering_timer_ = new QTimer(this);
        hovering_timer_->setSingleShot(true);

        renderer_ = module->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);

        visibility_animation_timeline_->setFrameRange(0,100);
        visibility_animation_timeline_->setEasingCurve(QEasingCurve::InOutSine);
        visibility_timer_->setSingleShot(true);

        connect(visibility_animation_timeline_, SIGNAL(frameChanged(int)), SLOT(UpdateAnimationStep(int)));
        connect(visibility_animation_timeline_, SIGNAL(finished()), SLOT(AnimationFinished()));

        visibility_timer_->setSingleShot(true);

        connect(visibility_animation_timeline_, SIGNAL(frameChanged(int)), SLOT(UpdateAnimationStep(int)));
        connect(visibility_animation_timeline_, SIGNAL(finished()), SLOT(AnimationFinished()));

        Foundation::UiServiceInterface *ui = module->GetFramework()->GetService<Foundation::UiServiceInterface>();
        if (!ui)
            return;

        proxy_ = ui->GetScene("Inworld")->addWidget(detachedwidget_);
        proxy_->hide();

        QPushButton *b = new QPushButton("Attach");
        detachedwidget_->AddButton(b);
        QObject::connect(b, SIGNAL(pressed()), this, SLOT(Attach()));
        Detach();
        Attach();
    }

    EC_HoveringWidget::~EC_HoveringWidget()
    {
        SAFE_DELETE(namewidget_);
        SAFE_DELETE(buttonswidget_);

        if (!renderer_.expired())
        {
            Ogre::TextureManager::getSingleton().remove(hoveringTexture1Name_);
            Ogre::TextureManager::getSingleton().remove(hoveringTexture2Name_);
        }
    }

    void EC_HoveringWidget::SetDisabled(bool val)
    {
        disabled_ = val;
        Hide();
    }

    void EC_HoveringWidget::SetButtonsDisabled(bool val)
    {
        buttons_disabled_ = val;
        if (buttons_disabled_)
        {
            ShowButtons(false);
            detachedwidget_->DisableButtons(true);
        }
    }

    void EC_HoveringWidget::ShowButtons(bool val)
    {
        buttons_visible_ = val;
        if (buttonsbillboardSet_)
            if (buttons_visible_ && !disabled_ && !buttons_disabled_)
                buttonsbillboardSet_->setVisible(true);
            else
                buttonsbillboardSet_->setVisible(false);
    }

    void EC_HoveringWidget::SetCameraDistance(float dist)
    {
        if (!detached_)
        {
            cam_distance_ = dist;
            AdjustWidgetinfo();
            if (IsVisible())
            {
                ScaleWidget(*namebillboardSet_, *namebillboard_, bb_name_size_view);
                ScaleWidget(*buttonsbillboardSet_, *buttonsbillboard_, bb_buttons_size_view,true);
            }
        }
    }

    void EC_HoveringWidget::ScaleWidget(Ogre::BillboardSet& bset, Ogre::Billboard& b, const QSizeF& size,bool next_to_name_tag)
    {
        if (renderer_.expired())
            return;

        Ogre::Matrix4 worldmat;
        bset.getWorldTransforms(&worldmat);
        Ogre::Camera* camera = renderer_.lock()->GetCurrentCamera();

        Ogre::Vector3 mid_pos = b.getPosition();
        Ogre::Matrix4 viewproj = camera->getViewMatrix();
        viewproj = camera->getProjectionMatrix() * viewproj;
        mid_pos = viewproj * worldmat * mid_pos;
        
        Ogre::Vector3 pos1(mid_pos.x - size.width()/2,mid_pos.y-size.height()/2, mid_pos.z);
        Ogre::Vector3 pos2(mid_pos.x + size.width()/2, mid_pos.y+size.height()/2, mid_pos.z);

        name_scr_pos_ = QPointF((pos1.x+1)*0.5, (pos1.y+1)*0.5); //save for later

        pos1 = viewproj.inverse() * pos1;
        pos2 = viewproj.inverse() * pos2;

        Ogre::Vector3 cam_up = camera->getDerivedUp();
        Ogre::Vector3 cam_right = camera->getDerivedRight();
        Ogre::Vector3 diagonal = pos1 - pos2;
        cam_up.normalise();
        cam_right.normalise();
        
        Ogre::Vector3 width_vec = cam_right.dotProduct(diagonal) * cam_right;
        Ogre::Vector3 height_vec = cam_up.dotProduct(diagonal) * cam_up;
        b.setDimensions(width_vec.length(), height_vec.length());
        
        if (next_to_name_tag)
        {
            Ogre::Vector3 pos(namebillboard_->getPosition());
            pos.y += ((-namebillboard_->getOwnWidth()*0.5)  /*+(-b.getOwnWidth()*0.5)*/);
            pos.z -= (namebillboard_->getOwnHeight()*0.5) + b.getOwnHeight()*0.5;
            b.setPosition(pos.x ,pos.y ,pos.z);
        }
        else
            b.setPosition(b.getPosition().x  ,b.getPosition().y  ,bb_rel_posy +  (b.getOwnHeight()/2) );
    }

    void EC_HoveringWidget::AdjustWidgetinfo()
    {
        qreal distance_factor = 0;
        qreal hover_factor = 0;
        qreal overall_rate = 0;

        if (cam_distance_ < 10)
            distance_factor = 100;
        else if (cam_distance_ < 30)
            distance_factor = 50;
        else if (cam_distance_ >= 25)
            distance_factor = 0;

        if (hovering_timer_->isActive())
            hover_factor = 80;

        overall_rate += hover_factor;
        overall_rate += distance_factor;

        if (overall_rate >= 100)
        {
            if (!IsVisible())
                Show();
            ShowButtons(true);
        }
        else if (overall_rate >= 50)
        {
            if (!IsVisible())
                Show();
            ShowButtons(false);
        }
        else if (overall_rate < 50)
        {
            if (IsVisible())
                Hide();
        }
    }

    void EC_HoveringWidget::Show()
    {
        if (namebillboardSet_ && !disabled_)
            namebillboardSet_->setVisible(true);
        if (buttonsbillboardSet_ && !disabled_ && !buttons_disabled_)
            buttonsbillboardSet_->setVisible(true);
    }

    void EC_HoveringWidget::AnimatedShow()
    {
        /*if(!disabled_)
        {
            if (visibility_animation_timeline_->state() == QTimeLine::Running ||
                visibility_timer_->isActive() || IsVisible())
                return;

            UpdateAnimationStep(0);
            Show();

            visibility_animation_timeline_->setDirection(QTimeLine::Forward);
            visibility_animation_timeline_->start();
        }*/
    }

    void EC_HoveringWidget::WidgetClicked(float x, float y)
    {
        if(buttons_visible_ && !buttons_disabled_)
            buttonswidget_->ForwardMouseClickEvent(x,y);
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

        if (!disabled_)
        {
            detached_ = !detached_;
            if (!detached_)
                Attach();
            else
                Detach();
        }
    }

    void EC_HoveringWidget::Hide()
    {
        if (namebillboardSet_)
            namebillboardSet_->setVisible(false);
        if (buttonsbillboardSet_)
            buttonsbillboardSet_->setVisible(false);
    }

    void EC_HoveringWidget::Detach()
    {
        if(renderer_.expired())
            return;

        Hide();
        qreal scene_w = renderer_.lock()->GetWindowWidth();
        qreal scene_h = renderer_.lock()->GetWindowHeight();

        proxy_->setGeometry(QRectF(name_scr_pos_.x()*scene_w, (1-name_scr_pos_.y())* scene_h,proxy_->preferredWidth(),proxy_->preferredHeight()));
        proxy_->show();
    }

    void EC_HoveringWidget::Attach()
    {
        proxy_->hide();
        Show();
    }

    void EC_HoveringWidget::AnimatedHide()
    {
        /*if (visibility_animation_timeline_->state() == QTimeLine::Running ||
            visibility_timer_->isActive() || !IsVisible())
            return;

        UpdateAnimationStep(100);
        visibility_animation_timeline_->setDirection(QTimeLine::Backward);
        visibility_animation_timeline_->start();*/
    }

    void EC_HoveringWidget::UpdateAnimationStep(int step)
    {
        if (namematerialName_.empty())
            return;

        float alpha = step;
        alpha /= 100;

        Ogre::MaterialManager &mgr = Ogre::MaterialManager::getSingleton();
        Ogre::MaterialPtr material = mgr.getByName(namematerialName_);
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
        if (namebillboardSet_)
            return namebillboardSet_->isVisible();
        else
            return false;
    }

    void EC_HoveringWidget::HoveredOver()
    {
        if (hovering_timer_->isActive())
            hovering_timer_->stop();
        hovering_timer_->start(hovering_time_);
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
        if (!namebillboardSet_ && !namebillboard_ && !buttonsbillboardSet_ && !buttonsbillboard_)
        {
            namebillboardSet_ = scene->createBillboardSet(renderer_.lock()->GetUniqueObjectName(), 1);
            buttonsbillboardSet_ = scene->createBillboardSet(renderer_.lock()->GetUniqueObjectName(), 1);
            assert(namebillboardSet_);
            assert(buttonsbillboardSet_);

            namematerialName_ = std::string("material")+ std::string("name") + renderer_.lock()->GetUniqueObjectName();
            buttonsmaterialName_ = std::string("material")+ std::string("buttons") + renderer_.lock()->GetUniqueObjectName();

            OgreRenderer::CloneMaterial("HoveringText", namematerialName_);
            OgreRenderer::CloneMaterial("HoveringText", buttonsmaterialName_);

            namebillboardSet_->setMaterialName(namematerialName_);
            namebillboardSet_->setCastShadows(false);

            buttonsbillboardSet_->setMaterialName(buttonsmaterialName_);
            buttonsbillboardSet_->setCastShadows(false);

            //namebillboardSet_->setBillboardType(Ogre::BBT_ORIENTED_COMMON);
            //buttonsbillboardSet_->setBillboardType(Ogre::BBT_ORIENTED_COMMON);

            namebillboardSet_->setCommonUpVector(Ogre::Vector3::UNIT_Z);
            buttonsbillboardSet_->setCommonUpVector(Ogre::Vector3::UNIT_Z);

            namebillboard_ = namebillboardSet_->createBillboard(Ogre::Vector3(0, 0, bb_rel_posy));
            assert(namebillboard_);
            namebillboard_->setDimensions(bb_name_size_view.width(), bb_name_size_view.height());
            sceneNode->attachObject(namebillboardSet_);

            buttonsbillboard_ = buttonsbillboardSet_->createBillboard(Ogre::Vector3(0, 0, bb_rel_posy));
            assert(buttonsbillboard_);
            assert(namebillboard_);
            buttonsbillboard_->setDimensions(bb_buttons_size_view.width(), bb_buttons_size_view.height());
            sceneNode->attachObject(buttonsbillboardSet_);
        }

        namebillboardSet_->setVisible(false);
        buttonsbillboardSet_->setVisible(false);

        Redraw();
    }

    void EC_HoveringWidget::SetText(const QString &text)
    {
        if (text.isNull() || text.isEmpty())
            return;
        namewidget_->SetText(text);
        detachedwidget_->SetText(text);

        Redraw();
    }

    void EC_HoveringWidget::AddButton(QPushButton &button)
    {
        QPushButton* copy = new QPushButton(button.text());
        copy->setStyleSheet(button.styleSheet());
        QObject::connect(copy, SIGNAL(pressed()), &button, SLOT(click()));
        detachedwidget_->AddButton(copy);
        buttonswidget_->AddButton(&button);

        Redraw();
    }

    void EC_HoveringWidget::Redraw()
    {
        if (renderer_.expired() || !namebillboardSet_ || !namebillboard_)
            return;

        // Check name tag width
        float name_tag_width = CheckNameTagWidth();
        QRect name_rect(0, 0, 300, 60);
        if (name_tag_width > name_rect.width())
        {
            name_rect.setWidth(name_tag_width);
            qreal jee = name_tag_width / 1200;
            bb_name_size_view = QSizeF(jee, bb_name_size_view.height());
        }

        // Get pixmap with text rendered to it.
        QPixmap pixmap1 = GetPixmap(*namewidget_, name_rect);
        QPixmap pixmap2 = GetPixmap(*buttonswidget_, QRect(0,0,80,120));
        if (pixmap1.isNull()||pixmap2.isNull())
            return;

        QImage img1 = pixmap1.toImage();
        QImage img2 = pixmap2.toImage();

        // Create Ogre textures
        Ogre::TextureManager &manager = Ogre::TextureManager::getSingleton();
        Ogre::TexturePtr texPtr1, texPtr2;
        try
        {
            // If texture don't exist, create them now.
            if (hoveringTexture1Name_.empty() && hoveringTexture2Name_.empty())
            {
                hoveringTexture1Name_ = "HoveringTextTexture" + renderer_.lock()->GetUniqueObjectName();
                hoveringTexture2Name_ = "HoveringTextTexture" + renderer_.lock()->GetUniqueObjectName();

                texPtr1 = manager.createManual(hoveringTexture1Name_, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                    Ogre::TEX_TYPE_2D, img1.width(), img1.height(), Ogre::MIP_DEFAULT, Ogre::PF_A8R8G8B8, Ogre::TU_DEFAULT);

                texPtr2 = manager.createManual(hoveringTexture2Name_, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                    Ogre::TEX_TYPE_2D, img2.width(), img2.height(), Ogre::MIP_DEFAULT, Ogre::PF_A8R8G8B8, Ogre::TU_DEFAULT);

                if (texPtr1.isNull())
                {
                    std::cout << "Failed to create texture " << hoveringTexture1Name_ << std::endl;
                    return;
                }
                if (texPtr2.isNull())
                {
                    std::cout << "Failed to create texture " << hoveringTexture2Name_ << std::endl;
                    return;
                }
            }
            else
            {
                // Textures already exists. Re-use them.
                texPtr1 = manager.getByName(hoveringTexture1Name_);
                texPtr2 = manager.getByName(hoveringTexture2Name_);
                assert(!texPtr1.isNull() || !texPtr2.isNull());

                // See if size/format changed, have to delete/recreate internal resources
                if (img1.width() != texPtr1->getWidth() || img1.height() != texPtr1->getHeight())
                {
                    texPtr1->freeInternalResources();
                    texPtr1->setWidth(img1.width());
                    texPtr1->setHeight(img1.height());
                    texPtr1->setFormat(Ogre::PF_A8R8G8B8);
                    texPtr1->createInternalResources();
                }
                if (img2.width() != texPtr2->getWidth() || img2.height() != texPtr2->getHeight())
                {
                    texPtr2->freeInternalResources();
                    texPtr2->setWidth(img2.width());
                    texPtr2->setHeight(img2.height());
                    texPtr2->setFormat(Ogre::PF_A8R8G8B8);
                    texPtr2->createInternalResources();
                }
            }

            Ogre::Box dimensions1(0,0, img1.width(), img1.height());
            Ogre::PixelBox pixel_box1(dimensions1, Ogre::PF_A8R8G8B8, (void*)img1.bits());
            if (!texPtr1->getBuffer().isNull())
                texPtr1->getBuffer()->blitFromMemory(pixel_box1);

            Ogre::Box dimensions2(0,0, img2.width(), img2.height());
            Ogre::PixelBox pixel_box2(dimensions2, Ogre::PF_A8R8G8B8, (void*)img2.bits());
            if (!texPtr2->getBuffer().isNull())
                texPtr2->getBuffer()->blitFromMemory(pixel_box2);
        }
        catch (Ogre::Exception &e)
        {
            std::cout << "Failed to create textures: " << std::string(e.what()) << std::endl;
            return;
        }

        // Set new texture for the material
        assert(!namematerialName_.empty() || !buttonsmaterialName_.empty());
        if (!namematerialName_.empty() && !buttonsmaterialName_.empty())
        {
            Ogre::MaterialManager &mgr = Ogre::MaterialManager::getSingleton();
            Ogre::MaterialPtr material = mgr.getByName(namematerialName_);
            material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureFiltering(Ogre::TFO_ANISOTROPIC);
            assert(material.get());
            OgreRenderer::SetTextureUnitOnMaterial(material, hoveringTexture1Name_);

            material = mgr.getByName(buttonsmaterialName_);
            material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureFiltering(Ogre::TFO_ANISOTROPIC);
            assert(material.get());
            OgreRenderer::SetTextureUnitOnMaterial(material, hoveringTexture2Name_);
        }
    }

    int EC_HoveringWidget::CheckNameTagWidth() const
    {
        QFontMetrics metric(namewidget_->label->font());
        int label_width = metric.width(namewidget_->label->text());
        label_width += metric.averageCharWidth() * 4;
        return label_width;
    }

    QPixmap EC_HoveringWidget::GetPixmap(QWidget &w, const QRect &dimensions)
    {
        if (renderer_.expired())
            return 0;

        // Create transparent pixmap
        QPixmap pixmap(dimensions.size());
        pixmap.fill(Qt::transparent);

        int x, y;
        if (namewidget_ == &w)
        {
            QFontMetrics metric(namewidget_->label->font());
            int label_width = metric.width(namewidget_->label->text());
            label_width += metric.averageCharWidth()*2;
            int label_height = metric.height() + 10;

            w.setMaximumSize(label_width, label_height);
            w.setMinimumSize(label_width, label_height);

            x = (dimensions.width() - label_width) / 2;
            y = (dimensions.height() - label_height) / 2;
        }
        else
        {
            w.setGeometry(dimensions);
            w.setMaximumSize(dimensions.size());
            w.setMinimumSize(dimensions.size());

            x = dimensions.x();
            y = dimensions.y();
        }

        // Init painter with pixmap as the paint device
        QPainter painter(&pixmap);
        w.render(&painter, QPoint(x,y), w.rect());
        return pixmap;
    }
}
