/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   EC_HoveringText.cpp
    @brief  Shows a hovering text attached to an entity. */

#define MATH_OGRE_INTEROP

#include "DebugOperatorNew.h"

#include "EC_HoveringText.h"
#include "Renderer.h"
#include "EC_Placeable.h"
#include "Entity.h"
#include "LoggingFunctions.h"
#include "Scene/Scene.h"
#include "Framework.h"
#include "OgreRenderingModule.h"
#include "OgreMaterialAsset.h"
#include "OgreWorld.h"
#include "OgreMaterialUtils.h"
#include "AssetAPI.h"
#include "TextureAsset.h"

#include <Ogre.h>
#include <QFile>
#include <QPainter>
#include <QTimer>
#include <QTimeLine>

#include "MemoryLeakCheck.h"

EC_HoveringText::EC_HoveringText(Scene* scene) :
    IComponent(scene),
    font_(QFont("Arial", 100)),
    textColor_(Qt::black),
    billboardSet_(0),
    billboard_(0),
    usingGrad(this, "Use Gradient", false),
    text(this, "Text"),
    font(this, "Font", "Arial"),
    fontColor(this, "Font Color"),
    fontSize(this, "Font Size", 100),
    backgroundColor(this, "Background Color", Color(1.0f,1.0f,1.0f,0.0f)),
    position(this, "Position", float3(0.0f, 0.0f, 0.0f)),
    gradStart(this, "Gradient Start", Color(0.0f,0.0f,0.0f,1.0f)),
    gradEnd(this, "Gradient End", Color(1.0f,1.0f,1.0f,1.0f)),
    borderColor(this, "Border Color", Color(0.0f,0.0f,0.0f,0.0f)),
    borderThickness(this, "Border Thickness", 0.0),
    overlayAlpha(this, "Overlay Alpha", 1.0),
    width(this, "Width", 1.0),
    height(this, "Height", 1.0),
    texWidth(this, "Texture Width", 256),
    texHeight(this, "Texture Height", 256),
    cornerRadius(this, "Corner Radius", float2(20.0, 20.0)),
    enableMipmapping(this, "Enable Mipmapping", true),
    material(this, "Material", AssetReference("local://HoveringText.material", ""))
{
    if (scene)
        world_ = scene->GetWorld<OgreWorld>();

    connect(&materialAsset, SIGNAL(Loaded(AssetPtr)), this, SLOT(OnMaterialAssetLoaded(AssetPtr)), Qt::UniqueConnection);
    connect(&materialAsset, SIGNAL(TransferFailed(IAssetTransfer*, QString)), this, SLOT(OnMaterialAssetFailed(IAssetTransfer*, QString)), Qt::UniqueConnection);
}

EC_HoveringText::~EC_HoveringText()
{
    if (texture_.get() != 0)
        framework->Asset()->ForgetAsset(texture_,false);
    Destroy();
}

void EC_HoveringText::Destroy()
{
    if (!ViewEnabled())
        return;

    if (!world_.expired())
    {
        Ogre::SceneManager* sceneMgr = world_.lock()->OgreSceneManager();
        
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
            sceneMgr->destroyBillboardSet(billboardSet_);
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

void EC_HoveringText::SetPosition(const float3& position)
{
    if (!ViewEnabled())
        return;

    if (billboard_)
    {
        billboard_->setPosition(position);
        billboardSet_->_updateBounds(); // Documentation of Ogre::BillboardSet says the update is never called automatically, so now do it manually.
    }
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

void EC_HoveringText::SetBackgroundGradient(const QColor &start_color, const QColor &end_color)
{
    bg_grad_.setColorAt(0.0, start_color);
    bg_grad_.setColorAt(1.0, end_color);
}

void EC_HoveringText::Show()
{
    if (!ViewEnabled())
        return;

    if (billboardSet_)
        billboardSet_->setVisible(true);
}

void EC_HoveringText::Hide()
{
    if (!ViewEnabled())
        return;

    if (billboardSet_)
        billboardSet_->setVisible(false);
}

void EC_HoveringText::SetOverlayAlpha(float alpha)
{
    Ogre::MaterialManager &mgr = Ogre::MaterialManager::getSingleton();
    Ogre::MaterialPtr material = mgr.getByName(materialName_);
    if (!material.get() || material->getNumTechniques() < 1 || material->getTechnique(0)->getNumPasses() < 1 || material->getTechnique(0)->getPass(0)->getNumTextureUnitStates() < 1)
        return;

    material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setAlphaOperation(
        Ogre::LBX_BLEND_MANUAL, Ogre::LBS_TEXTURE, Ogre::LBS_MANUAL, 1.0, 0.0, alpha);
}

void EC_HoveringText::SetBillboardSize(float width, float height)
{
    if (billboard_)
    {
        billboard_->setDimensions(width, height);
        // Bug in OGRE: It does not use the actual billboard bounding box size in the computation, but instead guesses it from the "default size", so
        // also set the default size to the size of our billboard (fortunately we only have one billboard in the set)
        billboardSet_->setDefaultDimensions(width*0.5f, height*0.5f); // Another bug in OGRE: It computes the billboard AABB padding to 2*width and 2*height, not width & height.
        billboardSet_->_updateBounds(); // Documentation of Ogre::BillboardSet says the update is never called automatically, so now do it manually.
    }
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
    if (world_.expired())
        return;
    
    OgreWorldPtr world = world_.lock();
    Ogre::SceneManager *scene = world->OgreSceneManager();
    assert(scene);
    if (!scene)
        return;

    Entity* entity = ParentEntity();
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
    if (!billboardSet_)
    {
        billboardSet_ = scene->createBillboardSet(world->GetUniqueObjectName("EC_HoveringText"), 1);
        assert(billboardSet_);
        billboardSet_->Ogre::MovableObject::setUserAny(Ogre::Any(static_cast<IComponent *>(this)));
        billboardSet_->Ogre::Renderable::setUserAny(Ogre::Any(static_cast<IComponent *>(this)));
        sceneNode->attachObject(billboardSet_);
    }

    if (billboardSet_ && !billboard_)
    {
        billboard_ = billboardSet_->createBillboard(Ogre::Vector3(0, 0, 0.7f));

        SetBillboardSize(width.Get(), height.Get());
        SetPosition(position.Get());
    }

    Redraw();
}

void EC_HoveringText::Redraw()
{
    if (!ViewEnabled())
        return;

    if (world_.expired() || !billboardSet_ || !billboard_ || materialName_.empty())
        return;

    bool textEmpty = text.Get().isEmpty();

    billboardSet_->setVisible(!textEmpty);
    if (textEmpty)
        return;

    try
    {
        if (texture_.get() == 0)
        {
            textureName_ = framework->Asset()->GenerateUniqueAssetName("tex", "EC_HoveringText_").toStdString();
            QString name(textureName_.c_str());
            texture_  = dynamic_pointer_cast<TextureAsset>(framework->Asset()->CreateNewAsset("Texture", name));
            assert(texture_);
            if (texture_ == 0)
            {
                LogError("Failed to create texture " + textureName_);
                return;
            }
        }
       
        QBrush brush(backgroundColor.Get());
       
        if (usingGrad.Get())
        {
            QRect rect(0,0,texWidth.Get(), texHeight.Get());
            bg_grad_.setStart(QPointF(0,rect.top()));
            bg_grad_.setFinalStop(QPointF(0,rect.bottom()));
            brush = QBrush(bg_grad_);
        }

        QColor borderCol;
        Color col = borderColor.Get();
        borderCol.setRgbF(col.r, col.g, col.b, col.a);

        QPen borderPen;
        borderPen.setColor(borderCol);
        borderPen.setWidthF(borderThickness.Get());
        
        float2 corners =  cornerRadius.Get();

        texture_->SetContentsDrawText(texWidth.Get(), 
                                texHeight.Get(), 
                                text.Get(), 
                                textColor_, 
                                font_, 
                                brush, 
                                borderPen, Qt::AlignCenter | Qt::TextWordWrap, enableMipmapping.Get(), false, corners.x, corners.y);
    }
    catch(Ogre::Exception &e)
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

void EC_HoveringText::AttributesChanged()
{
    if (font.ValueChanged() || fontSize.ValueChanged())
    {
        SetFont(QFont(font.Get(), fontSize.Get()));
    }
    if (fontColor.ValueChanged())
    {
        Color col = fontColor.Get();
        textColor_.setRgbF(col.r, col.g, col.b, col.a);
    }
    if (position.ValueChanged())
    {
        SetPosition(position.Get());
    }
    if (gradStart.ValueChanged() || gradEnd.ValueChanged())
    {
        QColor colStart;
        QColor colEnd;
        Color col = gradStart.Get();
        colStart.setRgbF(col.r, col.g, col.b);
        col = gradEnd.Get();
        colEnd.setRgbF(col.r, col.g, col.b);
        SetBackgroundGradient(colStart, colEnd);
    }
    if (overlayAlpha.ValueChanged())
        SetOverlayAlpha(overlayAlpha.Get());
    if (width.ValueChanged() || height.ValueChanged())
        SetBillboardSize(width.Get(), height.Get());

    if (material.ValueChanged())
    {
        // Don't render the HoveringText if it's not using a material.
        if (billboardSet_)
        {
            bool isVisible = !material.Get().ref.isEmpty();
            billboardSet_->setVisible(isVisible);
        }

        // If the material was cleared, erase the material from Ogre billboard as well. (we might be deleting the material in Tundra Asset API)
        if (material.Get().ref.isEmpty() && billboardSet_)
#if OGRE_VERSION_MAJOR <= 1 && OGRE_VERSION_MINOR <= 7 && OGRE_VERSION_PATCH <= 2
            billboardSet_->setMaterialName("BaseWhite"); // BaseWhite is an Ogre-internal default material which always exists.
#else // Ogre::BillboardSet::setMaterial() only available at Ogre 1.7.3 and newer.
            billboardSet_->setMaterial(Ogre::MaterialPtr());
#endif
        else
            materialAsset.HandleAssetRefChange(&material);
    }

    // Changes to the following attributes require a (expensive) repaint of the texture on the CPU side.
    bool repaint = text.ValueChanged() || font.ValueChanged() || fontSize.ValueChanged() || fontColor.ValueChanged()
        || backgroundColor.ValueChanged() || borderColor.ValueChanged() || borderThickness.ValueChanged() || usingGrad.ValueChanged()
        || gradStart.ValueChanged() || gradEnd.ValueChanged() || texWidth.ValueChanged() || texHeight.ValueChanged()
        || cornerRadius.ValueChanged() || enableMipmapping.ValueChanged();

    // Changes to the following attributes do not alter the texture contents, and don't require a repaint:
    // position, overlayAlpha, width, height.

    // Repaint the new text with new appearance.
    if (repaint)
        ShowMessage(text.Get());
}

void EC_HoveringText::OnMaterialAssetLoaded(AssetPtr asset)
{
    OgreMaterialAsset *ogreMaterial = dynamic_cast<OgreMaterialAsset*>(asset.get());
    if (!ogreMaterial)
    {
        LogError("OnMaterialAssetLoaded: Material asset load finished for asset \"" +
            asset->Name() + "\", but downloaded asset was not of type OgreMaterialAsset!");
        return;
    }

    // Make a clone of the material we loaded, since the same material may be used by some other entities in the scene,
    // and this EC_HoveringText must customize the material to show its own texture on it.
    RecreateMaterial();
}

void EC_HoveringText::OnMaterialAssetFailed(IAssetTransfer* transfer, QString reason)
{
    DeleteMaterial();

    if (billboardSet_)
        billboardSet_->setMaterialName("AssetLoadError");
}

void EC_HoveringText::RecreateMaterial()
{
    shared_ptr<OgreMaterialAsset> materialAsset = dynamic_pointer_cast<OgreMaterialAsset>(framework->Asset()->GetAsset(material.Get().ref));
    if (!materialAsset)
        return;

    DeleteMaterial(); // If we had an old material, free it up to not leak in Ogre.

    OgreRenderer::RendererPtr renderer = framework->GetModule<OgreRenderer::OgreRenderingModule>()->GetRenderer();

    materialName_ = renderer->GetUniqueObjectName("EC_HoveringText_material");
    try
    {
        OgreRenderer::CloneMaterial(materialAsset->ogreAssetName.toStdString(), materialName_);
        if (billboardSet_)
        {
            billboardSet_->setMaterialName(materialName_);
            billboardSet_->setCastShadows(false); ///\todo Is this good here?
        }
        Redraw();
    }
    catch(...)
    {
    }
}

void EC_HoveringText::DeleteMaterial()
{
    if (!materialName_.empty())
    {
        try
        {
            Ogre::MaterialManager::getSingleton().remove(materialName_);
        }catch(...)
        {
        }
        materialName_ = "";
    }
}
