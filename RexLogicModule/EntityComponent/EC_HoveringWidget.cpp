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
#include <QSizeF>
#include <QPushButton>
#include <QPainter>
#include <QTimer>
#include <QTimeLine>
#include <QDebug>


//#include "MemoryLeakCheck.h"
namespace RexLogic
{

    EC_HoveringWidget::EC_HoveringWidget(Foundation::ModuleInterface* module) :
    Foundation::ComponentInterface(module->GetFramework()),
        namebillboardSet_(0),
        buttonsbillboardSet_(0),
        namebillboard_(0),
        buttonsbillboard_(0),
        visibility_animation_timeline_(new QTimeLine(1000, this)),
        visibility_timer_(new QTimer(this)),
        namewidget_(new HoveringNameController()),
        buttonswidget_(new HoveringButtonsController()),
        disabled_(false),
        buttons_disabled_(false),
        buttons_visible_(false),
        bb_name_size_view(0.25f ,0.15f),
        bb_buttons_size_view(0.1f ,0.4f),
        bb_rel_posy(1.3f),
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
        SAFE_DELETE(namewidget_);
        SAFE_DELETE(buttonswidget_);
    }

    void EC_HoveringWidget::SetPosition(const Vector3df& position)
    {
       /* if (namebillboard_)
            namebillboard_->setPosition(Ogre::Vector3(position.x, position.y, position.z));*/
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
                buttonsbillboardSet_->setVisible(true);
            }
            else
            {
                buttonsbillboardSet_->setVisible(false);
            }
        }

    }

    void EC_HoveringWidget::SetCameraDistance(Real dist)
    {
        cam_distance_ = dist;
        AdjustWidgetinfo();
        if(IsVisible())
        {
            ScaleWidget(*namebillboardSet_, *namebillboard_, bb_name_size_view);
            ScaleWidget(*buttonsbillboardSet_, *buttonsbillboard_, bb_buttons_size_view,true);
        }
    }

    void EC_HoveringWidget::ScaleWidget(Ogre::BillboardSet& bset, Ogre::Billboard& b, QSizeF& size,bool next_to_name_tag)
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
        
        Ogre::Vector3 pos1(mid_pos.x,mid_pos.y, mid_pos.z);
        Ogre::Vector3 pos2(mid_pos.x + size.width(), mid_pos.y+size.height(), mid_pos.z);

        pos1 = viewproj.inverse() * pos1;
        pos2 = viewproj.inverse() * pos2;

        Ogre::Vector3 cam_up = camera->getDerivedUp();
        Ogre::Vector3 cam_right = camera->getDerivedRight();

        //qDebug() << "( " << pos1.x  << "," << pos1.y << "," << pos1.z <<") , ("<< pos2.x << "," << pos2.y << "," << pos2.z << ")";
        
        Ogre::Vector3 diagonal = pos1 - pos2;
        
        cam_up.normalise();
        cam_right.normalise();

        Ogre::Vector3 width_vec = cam_right.absDotProduct(diagonal) * cam_right;
        Ogre::Vector3 height_vec = cam_up.absDotProduct(diagonal) * cam_up;

        
        b.setDimensions(width_vec.length(), height_vec.length());
        
        if(next_to_name_tag)
        {
            Ogre::Vector3 pos(namebillboard_->getPosition());
            pos.y += ((-namebillboard_->getOwnWidth()*0.5) + (-b.getOwnWidth()*0.5));
            pos.z -= b.getOwnHeight()*0.5;
            b.setPosition(pos.x ,pos.y ,pos.z);
        }
        else
            b.setPosition(b.getPosition().x  ,b.getPosition().y  ,bb_rel_posy +  (b.getOwnHeight()/2) );

    }

    void EC_HoveringWidget::AdjustWidgetinfo()
    {
   

        if(cam_distance_<10)
        {
            ShowButtons(true);

            if(!IsVisible())
                Show();
        }else if(cam_distance_<30)
        {
            ShowButtons(false);

            if(!IsVisible())
                Show();

        }else if(cam_distance_>=30)
        {

            if(IsVisible())
                Hide();
        }

    }


    void EC_HoveringWidget::Show()
    {
        if (namebillboardSet_ && !disabled_)
            namebillboardSet_->setVisible(true);
        if (buttonsbillboardSet_ && !disabled_ && buttons_visible_)
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

    void EC_HoveringWidget::WidgetClicked(Real x, Real y)
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

        if(IsVisible())
            Hide();
        else
            Show();
    }


    void EC_HoveringWidget::Hide()
    {
        if (namebillboardSet_)
            namebillboardSet_->setVisible(false);
        if(buttonsbillboardSet_)
            buttonsbillboardSet_->setVisible(false);
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

<<<<<<< HEAD
            namebillboardSet_->setBillboardType(Ogre::BBT_ORIENTED_COMMON);
            buttonsbillboardSet_->setBillboardType(Ogre::BBT_ORIENTED_COMMON);

            namebillboardSet_->setCommonUpVector(Ogre::Vector3::UNIT_Z);
            buttonsbillboardSet_->setCommonUpVector(Ogre::Vector3::UNIT_Z);

=======
>>>>>>> 5aa6bd5698a8734b6fe9ce7cb684b848eb04a506
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
        Redraw();

    }

    void EC_HoveringWidget::AddButton(QPushButton &button)
    {
            buttonswidget_->AddButton(&button);
            Redraw();
    }

    void EC_HoveringWidget::Redraw()
    {
        if (renderer_.expired() || !namebillboardSet_ || !namebillboard_)
            return;

        // Get pixmap with text rendered to it.
        QPixmap pixmap1 = GetPixmap(*namewidget_, QRect(0,0,400,200));
        QPixmap pixmap2 = GetPixmap(*buttonswidget_, QRect(0,0,200,400));
        if (pixmap1.isNull()||pixmap2.isNull())
            return;

        // Create texture
        QImage img1 = pixmap1.toImage();
        QImage img2 = pixmap2.toImage();

        Ogre::DataStreamPtr stream1(new Ogre::MemoryDataStream((void*)img1.bits(), img1.byteCount()));
        Ogre::DataStreamPtr stream2(new Ogre::MemoryDataStream((void*)img2.bits(), img2.byteCount()));

        std::string tex_name1("HoveringTextTexture" + renderer_.lock()->GetUniqueObjectName());
        std::string tex_name2("HoveringTextTexture" + renderer_.lock()->GetUniqueObjectName());

        Ogre::TextureManager &manager = Ogre::TextureManager::getSingleton();

        Ogre::Texture *tex1 = checked_static_cast<Ogre::Texture *>(manager.create(
            tex_name1, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).get());
        Ogre::Texture *tex2 = checked_static_cast<Ogre::Texture *>(manager.create(
            tex_name2, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).get());

        assert(tex1);
        assert(tex2);

        tex1->loadRawData(stream1, img1.width(), img1.height(), Ogre::PF_A8R8G8B8);
        tex2->loadRawData(stream2, img2.width(), img2.height(), Ogre::PF_A8R8G8B8);

        // Set new texture for the material
        assert(!namematerialName_.empty() || !buttonsmaterialName_.empty());
        if (!namematerialName_.empty() && !buttonsmaterialName_.empty())
        {
            Ogre::MaterialManager &mgr = Ogre::MaterialManager::getSingleton();

            Ogre::MaterialPtr material = mgr.getByName(namematerialName_);
            material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureFiltering(Ogre::TFO_ANISOTROPIC);
            assert(material.get()); 
            OgreRenderer::SetTextureUnitOnMaterial(material, tex_name1);


            material = mgr.getByName(buttonsmaterialName_);
            material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureFiltering(Ogre::TFO_ANISOTROPIC);
            assert(material.get());
            OgreRenderer::SetTextureUnitOnMaterial(material, tex_name2);
        }
    }

    QPixmap EC_HoveringWidget::GetPixmap(QWidget& w, QRect dimensions)
    {


        if (renderer_.expired() )
            return 0;



        // Create transparent pixmap
        QPixmap pixmap(dimensions.size());
        pixmap.fill(Qt::transparent);
        w.setGeometry(dimensions);
        w.setMaximumSize(dimensions.size());
        w.setMinimumSize(dimensions.size());
        // Init painter with pixmap as the paint device
        QPainter painter(&pixmap);

        w.render(&painter, QPoint(0,0), w.rect());

        return pixmap;
    }
}
