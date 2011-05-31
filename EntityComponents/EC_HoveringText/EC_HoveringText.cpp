/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_HoveringText.cpp
 *  @brief  EC_HoveringText shows a hovering text attached to an entity.
 *  @note   The entity must EC_Placeable available in advance.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EC_HoveringText.h"
#include "IModule.h"
#include "Renderer.h"
#include "EC_Placeable.h"
#include "Entity.h"
#include "OgreMaterialUtils.h"
#include "LoggingFunctions.h"
#include "SceneManager.h"

DEFINE_POCO_LOGGING_FUNCTIONS("EC_HoveringText");

#include "Framework.h"
#include "OgreRenderingModule.h"
#include "AssetApi.h"
#include "TextureAsset.h"
#include <Ogre.h>
#include <OgreBillboardSet.h>
#include <OgreTextureManager.h>
#include <OgreResource.h>
#include <QFile>
#include <QPainter>
#include <QTimer>
#include <QTimeLine>

#include "MemoryLeakCheck.h"

EC_HoveringText::EC_HoveringText(IModule *module) :
    IComponent(module->GetFramework()),
    font_(QFont("Arial", 100)),
    backgroundColor_(Qt::transparent),
    textColor_(Qt::black),
    billboardSet_(0),
    billboard_(0),
    visibility_animation_timeline_(new QTimeLine(1000, this)),
    visibility_timer_(new QTimer(this)),
    usingGrad(this, "Use Gradiant", false),
    text(this, "Text"),
    font(this, "Font", "Arial"),
    fontColor(this, "Font Color"),
    fontSize(this, "Font Size", 100),
    backgroundColor(this, "Background Color", Color(1.0f,1.0f,1.0f,0.0f)),
    position(this, "Position", Vector3df(0.0f, 0.0f, 0.0f)),
    gradStart(this, "Gradient Start", Color(0.0f,0.0f,0.0f,1.0f)),
    gradEnd(this, "Gradient End", Color(1.0f,1.0f,1.0f,1.0f)),
    borderColor(this, "Border Color", Color(0.0f,0.0f,0.0f,0.0f)),
    borderThickness(this, "Border Thickness", 0.0)
{
    renderer_ = module->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Service::ST_Renderer);
    visibility_animation_timeline_->setFrameRange(0,100);
    visibility_animation_timeline_->setEasingCurve(QEasingCurve::InOutSine);
    visibility_timer_->setSingleShot(true);

    connect(visibility_animation_timeline_, SIGNAL(frameChanged(int)), SLOT(UpdateAnimationStep(int)));
    connect(visibility_animation_timeline_, SIGNAL(finished()), SLOT(AnimationFinished()));

    connect(this, SIGNAL(ParentEntitySet()), this, SLOT(UpdateSignals()));
}

EC_HoveringText::~EC_HoveringText()
{
    Destroy();
}

void EC_HoveringText::Destroy()
{
    if (!ViewEnabled())
        return;

    OgreRenderer::RendererPtr renderer = renderer_.lock();
    if (renderer)
    {
      
        try{
        Ogre::MaterialManager::getSingleton().remove(materialName_);
        } catch(...)
        {
        }
        try{
        if (billboardSet_ && billboard_)
            billboardSet_->removeBillboard(billboard_);
        } catch(...)
        {
        }
        try{
        
        if (billboardSet_)
        {
            Ogre::SceneManager *scene = renderer->GetSceneManager();
            if (scene)
                scene->destroyBillboardSet(billboardSet_);
        }

        } catch(...)
        {
        }
    }

    billboard_ = 0;
    billboardSet_ = 0;
    textureName_ = "";
    materialName_ = "";
}

void EC_HoveringText::SetPosition(const Vector3df& position)
{
    if (!ViewEnabled())
        return;

    if (billboard_)
        billboard_->setPosition(Ogre::Vector3(position.x, position.y, position.z));
}

void EC_HoveringText::SetPosition(const QVector3D &position)
{
    SetPosition(Vector3df(position.x(), position.y(), position.z()));
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
    //using_gradient_ = false;
    Redraw();
}

void EC_HoveringText::SetBackgroundGradient(const QColor &start_color, const QColor &end_color)
{
    bg_grad_.setColorAt(0.0, start_color);
    bg_grad_.setColorAt(1.0, end_color);
    //using_gradient_ = true;
}

void EC_HoveringText::Show()
{
    if (!ViewEnabled())
        return;

    if (billboardSet_)
        billboardSet_->setVisible(true);
}

void EC_HoveringText::AnimatedShow()
{
    if (!ViewEnabled())
        return;

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
    if (!ViewEnabled())
        return;

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
    if (!ViewEnabled())
        return;

    if (billboardSet_)
        billboardSet_->setVisible(false);
}

void EC_HoveringText::AnimatedHide()
{
    if (!ViewEnabled())
        return;

    if (visibility_animation_timeline_->state() == QTimeLine::Running ||
        visibility_timer_->isActive() || !IsVisible())
        return;

    UpdateAnimationStep(100);
    visibility_animation_timeline_->setDirection(QTimeLine::Backward);
    visibility_animation_timeline_->start();
}

void EC_HoveringText::UpdateAnimationStep(int step)
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

void EC_HoveringText::AnimationFinished()
{
    if (visibility_animation_timeline_->direction() == QTimeLine::Backward && IsVisible())
        Hide();
}

bool EC_HoveringText::IsVisible() const
{
    if (!ViewEnabled())
        return false;

    if (billboardSet_)
        return billboardSet_->isVisible();
    else
        return false;
}

void EC_HoveringText::ShowMessage(const QString &text)
{
    if (!ViewEnabled())
        return;
    if (renderer_.expired())
        return;
    
    // Moved earlier to prevent gray opaque box artifact if text is empty. Original place was just before Redraw().
    if (text.isNull() || text.isEmpty())
        return;
    
    Ogre::SceneManager *scene = renderer_.lock()->GetSceneManager();
    assert(scene);
    if (!scene)
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
    if (!billboardSet_ && !billboard_)
    {
        billboardSet_ = scene->createBillboardSet(renderer_.lock()->GetUniqueObjectName("EC_HoveringText"), 1);
        assert(billboardSet_);

        materialName_ = renderer_.lock()->GetUniqueObjectName("EC_HoveringText_material");
        OgreRenderer::CloneMaterial("HoveringText", materialName_);
        billboardSet_->setMaterialName(materialName_);
        billboardSet_->setCastShadows(false);

        billboard_ = billboardSet_->createBillboard(Ogre::Vector3(0, 0, 0.7f));
        assert(billboard_);
        billboardSet_->setDefaultDimensions(2, 1);

        sceneNode->attachObject(billboardSet_);
    }

    //textAttr.Set(text);
    Redraw();
}

void EC_HoveringText::Redraw()
{
    if (!ViewEnabled())
        return;

    if (renderer_.expired() || !billboardSet_ || !billboard_)
        return;

    try
    {
        if (texture_.get() == 0)
        {
       
            AssetAPI* asset = framework_->Asset();

            textureName_ = asset->GenerateUniqueAssetName("tex", "EC_HoveringText_").toStdString();
            QString name(textureName_.c_str());
            texture_  = boost::dynamic_pointer_cast<TextureAsset>(asset->CreateNewAsset("Texture", name));  
            
            assert(texture_);
            
            if (texture_ == 0)
            {
                LogError("Failed to create texture " + textureName_);
                return;
            }
        }
       

        QFontMetrics metric(font_); 
        int width = metric.width(text.Get()) + metric.averageCharWidth();
        int height = metric.height() + 100;
        
        if ( width > 800 )
        {
            int s = width % 800;
            int lines = (width - s) / 800;
            height = height * lines;
            width = 800 + s;
        }
       
        QBrush* brush = 0;

        if (usingGrad.Get())
        {   
            QRect rect(0,0,width, height);
            bg_grad_.setStart(QPointF(0,rect.top()));
            bg_grad_.setFinalStop(QPointF(0,rect.bottom()));
            brush = new QBrush(bg_grad_);
        }
        else
        {
            brush = new QBrush(backgroundColor_);
        }

        QColor borderCol;
        Color col = borderColor.Get();
        borderCol.setRgbF(col.r, col.g, col.b, col.a);

        QPen borderPen;
        borderPen.setColor(borderCol);
        borderPen.setWidthF(borderThickness.Get());
        
        
        texture_->SetTextContent(width, 
                                height, 
                                text.Get(), 
                                textColor_, 
                                font_, 
                                *brush, 
                                borderPen);

         delete brush;
         brush = 0;

    }
    catch (Ogre::Exception &e)
    {
        LogError("Failed to create texture " + textureName_  + ": " + std::string(e.what()));
        return;
    }

    // Set new texture for the material
    assert(!materialName_.empty());
    if (!materialName_.empty())
    {
        Ogre::MaterialManager &mgr = Ogre::MaterialManager::getSingleton();
        Ogre::MaterialPtr material = mgr.getByName(materialName_);
        assert(material.get());
        OgreRenderer::SetTextureUnitOnMaterial(material, textureName_);
    }
}

void EC_HoveringText::UpdateSignals()
{
    disconnect(this, SLOT(OnAttributeUpdated(IComponent *, IAttribute *)));
    if(GetParentEntity())
    {
        Scene::SceneManager *scene = GetParentEntity()->GetScene();
        if(scene)
        connect(scene, SIGNAL(AttributeChanged(IComponent*, IAttribute*, AttributeChange::Type)),
                this, SLOT(OnAttributeUpdated(IComponent*, IAttribute*))); 
    }
}

void EC_HoveringText::OnAttributeUpdated(IComponent *component, IAttribute *attribute)
{
    if(component != this)
        return;

    QString attrName = QString::fromStdString(attribute->GetNameString());
    if(QString::fromStdString(font.GetNameString()) == attrName ||QString::fromStdString(fontSize.GetNameString()) == attrName)
    {
        SetFont(QFont(font.Get(), fontSize.Get()));
    }
    else if(QString::fromStdString(backgroundColor.GetNameString()) == attrName)
    {
        Color col = backgroundColor.Get();
        backgroundColor_.setRgbF(col.r, col.g, col.b, col.a);
    }
    else if(QString::fromStdString(fontColor.GetNameString()) == attrName)
    {
        Color col = fontColor.Get();
        textColor_.setRgbF(col.r, col.g, col.b, col.a);
    }
    else if(QString::fromStdString(position.GetNameString()) == attrName)
    {
        SetPosition(position.Get());
    }
    else if(QString::fromStdString(gradEnd.GetNameString()) == attrName ||
        QString::fromStdString(gradStart.GetNameString()) == attrName ||
        QString::fromStdString(gradEnd.GetNameString()) == attrName)
    {
        QColor colStart;
        QColor colEnd;
        Color col = gradStart.Get();
        colStart.setRgbF(col.r, col.g, col.b);
        col = gradEnd.Get();
        colEnd.setRgbF(col.r, col.g, col.b);
        SetBackgroundGradient(colStart, colEnd);
    }

    // Repaint the new text with new appearance.
    ShowMessage(text.Get());
}

