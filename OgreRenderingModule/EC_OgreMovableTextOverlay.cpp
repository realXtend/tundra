// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_OgreMovableTextOverlay.h"
#include "EC_OgrePlaceable.h"
#include "Renderer.h"
#include "OgreRenderingModule.h"
#include "OgreConversionUtils.h"

#include <Ogre.h>
#include <OgreTextAreaOverlayElement.h>
#include <OgreFontManager.h>

// Code partly adapted from http://www.ogre3d.org/wiki/index.php/ObjectTextDisplay and
// http://www.ogre3d.org/wiki/index.php/ObjectTitle

static const float MAX_VISIBILITY_DISTANCE = 50.f;

namespace OgreRenderer
{

EC_OgreMovableTextOverlay::EC_OgreMovableTextOverlay(Foundation::ModuleInterface* module) :
    Foundation::ComponentInterface(module->GetFramework()),
    text_element_(0),
    container_(0),
    overlay_(0),
    font_(0),
    node_(0),
    renderer_(checked_static_cast<OgreRenderingModule*>(module)->GetRenderer()),
//        char_height_(2*0.0175f),
    visible_(false),
    overlayName_(""),
    containerName_(""),
    baseMaterialName_("BlueTransparent"),
    materialHasAlpha_(false),
    text_(""),
    attached_(false)
{
    Ogre::Viewport* viewport = renderer_.lock()->GetViewport();
    windowWidth_ = viewport->getActualWidth();
    windowHeight_ = viewport->getActualHeight();
}

// virtual
EC_OgreMovableTextOverlay::~EC_OgreMovableTextOverlay()
{
    if (renderer_.expired())
        return;

    if (overlay_)
    {
        overlay_->hide();
        container_->removeChild(overlayName_);
        overlay_->remove2D(container_);

        Ogre::OverlayManager *overlayManager = Ogre::OverlayManager::getSingletonPtr();
        overlayManager->destroyOverlayElement(text_element_);
        overlayManager->destroyOverlayElement(container_);
        overlayManager->destroy(overlay_);
    }

    if (node_)
    {
        DetachNode();
        Ogre::SceneManager *scene_mgr = renderer_.lock()->GetSceneManager();
        scene_mgr->destroySceneNode(node_);
        node_ = 0;
    }
}

void EC_OgreMovableTextOverlay::Update()
{
    if (!node_ || !visible_ || !placeable_ || renderer_.expired())
        return;

    if(!node_->isInSceneGraph())
    {
        overlay_->hide();
        return;
    }

    Ogre::Camera* camera = renderer_.lock()->GetCurrentCamera();
    if (!camera)
        return;

    Ogre::Viewport* viewport = camera->getViewport();
    Ogre::Vector3 point = node_->_getDerivedPosition();

    // Is the camera facing that point? If not, hide the overlay and return.
    Ogre::Plane cameraPlane = Ogre::Plane(Ogre::Vector3(camera->getDerivedOrientation().zAxis()), camera->getDerivedPosition());
    if(cameraPlane.getSide(point) != Ogre::Plane::NEGATIVE_SIDE)
    {
        overlay_->hide();
        return;
    }

    // Hide the overlay if it's too far.
    Ogre::Vector3 res = camera->getDerivedPosition() - point;
    float distance = sqrt(res.x * res.x + res.y * res.y + res.z * res.z);

    if (distance > MAX_VISIBILITY_DISTANCE)
    {
        overlay_->hide();
        return;
    }

    // Set the alpha channel for the overlay.
    if (materialHasAlpha_)
        SetAlphaChannelIntensity(distance);

    // Derive the 2D screen-space coordinates for node point.
    point = camera->getProjectionMatrix() * (camera->getViewMatrix() * point);

    // Transform from coordinate space [-1, 1] to [0, 1]
    float x = (point.x / 2) + 0.5f;
    float y = 1 - ((point.y / 2) + 0.5f);

    // Update the position (centering the text)
    container_->setPosition(x - (textDim_.x / 2), y);

    // Update the dimensions also if the window is resized.
    if (windowWidth_ != viewport->getActualWidth() ||
        windowHeight_ != viewport->getActualHeight())
    {
        windowWidth_ = viewport->getActualWidth();
        windowHeight_ = viewport->getActualHeight();
        
        textDim_ = GetTextDimensions(text_);
        container_->setDimensions(textDim_.x, textDim_.y);
    }

    ///\todo Scale the text and width and height of the container?
//        text_element_->setMetricsMode(Ogre::GMM_RELATIVE);
//        text_element_->setPosition(textDim_.x, textDim_.y);
//        text_element_->setPosition(textDim_.x / 10, 0.01);
//        text_element_->setCharHeight(max_x - min_x/*2*0.0175f*///);

    overlay_->show();
}

void EC_OgreMovableTextOverlay::SetVisible(bool visible)
{
    if (!node_)
        return;

    visible_ = visible;
    if (visible)
        overlay_->show();
    else
        overlay_->hide();
}

void EC_OgreMovableTextOverlay::SetPlaceable(Foundation::ComponentPtr placeable)
{
    if (!node_)
        return;

    if (!placeable)
    {
        OgreRenderingModule::LogError("Null placeable for overlay");
        return;
    }

    EC_OgrePlaceable* placeableptr = dynamic_cast<EC_OgrePlaceable*>(placeable.get());
    if (!placeableptr)
    {
        OgreRenderingModule::LogError("Placeable is not" + EC_OgrePlaceable::TypeNameStatic());
        return;
    }

    DetachNode();
    placeable_  = placeable;
    AttachNode();
}

void EC_OgreMovableTextOverlay::AttachNode()
{
    // Using the link scene node is necessary here so that the overlays can use the _update() trick to avoid stutter
    if ((node_) && (!attached_) && (placeable_))
    {
        Ogre::SceneNode* parent =
            checked_static_cast<EC_OgrePlaceable*>(placeable_.get())->GetLinkSceneNode();
        parent->addChild(node_);
        attached_ = true;
    }
}

void EC_OgreMovableTextOverlay::DetachNode()
{
    if ((node_) && (attached_) && (placeable_))
    {
        Ogre::SceneNode* parent = checked_static_cast<EC_OgrePlaceable*>(placeable_.get())->GetLinkSceneNode();
        parent->removeChild(node_);
        attached_ = false;
    }
}

void EC_OgreMovableTextOverlay::SetText(const std::string& text)
{
    if (!node_)
        return;

    text_ = text;
    boost::trim(text_);
    // Replace few common scandic letters with a and o.
    ReplaceCharInplace(text_, 'Ä', 'A');
    ReplaceCharInplace(text_, 'ä', 'a');
    ReplaceCharInplace(text_, 'Ö', 'O');
    ReplaceCharInplace(text_, 'ö', 'o');
    ReplaceCharInplace(text_, 'Å', 'A');
    ReplaceCharInplace(text_, 'å', 'a');

    try
    {
        text_element_->setCaption(text_);
    }
    catch(...)
    {
        text_ = "";
    }

    textDim_ = GetTextDimensions(text_);
    container_->setDimensions(textDim_.x, textDim_.y);
}

void EC_OgreMovableTextOverlay::SetMaterial(const std::string& new_base_material)
{
    ///\todo Make this work for other materials!
    // Clone own copy of the material for this overlay.
    Ogre::MaterialManager &mm = Ogre::MaterialManager::getSingleton();
    Ogre::MaterialPtr material = mm.getByName(new_base_material);
    if (material.get())
    {
        baseMaterialName_ = new_base_material;
        Ogre::MaterialPtr baseMaterial = mm.getByName(baseMaterialName_);
        materialName_ = renderer_.lock()->GetUniqueObjectName();
        material = baseMaterial->clone(materialName_);

        //todo Check that the the material alpha channel?
        const Ogre::LayerBlendModeEx &blend = baseMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getAlphaBlendMode();
        if (blend.alphaArg1 > 0)
            materialHasAlpha_ = true;
        else
            materialHasAlpha_ = false;

        // Set the max alpha for the material.
        materialMaxAlpha_ = blend.alphaArg1;

        container_->setMaterialName(materialName_);
    }
    else
        OgreRenderingModule::LogError("Invalid material name!");
}

void EC_OgreMovableTextOverlay::SetOffset(const Vector3df& offset)
{
    if (!node_)
        return;

    node_->setPosition(ToOgreVector3(offset));
}

void EC_OgreMovableTextOverlay::CreateOverlay(const Vector3df& offset)
{
    if (renderer_.expired())
        return;

    // Return if already created
    if (node_)
        return;

    // Create SceneNode
    Ogre::SceneManager *scene_mgr = renderer_.lock()->GetSceneManager();
    node_ = scene_mgr->createSceneNode();

    // Set the node position to an user-specified offset
    node_->setPosition(ToOgreVector3(offset));

    // Overlay
    overlayName_ = renderer_.lock()->GetUniqueObjectName();
    overlay_ = Ogre::OverlayManager::getSingleton().create(overlayName_);

    // Container
    containerName_ = renderer_.lock()->GetUniqueObjectName();
    container_ = static_cast<Ogre::OverlayContainer*>
        (Ogre::OverlayManager::getSingleton().createOverlayElement("Panel", containerName_));
    overlay_->add2D(container_);

    // Font
    ///\todo user-defined font
    std::string fontName = "Console";
    Ogre::FontManager::getSingleton().load(fontName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    font_ = (Ogre::Font*)Ogre::FontManager::getSingleton().getByName(fontName).getPointer();
    font_->setParameter("size", "16");

    // Overlay text
    text_element_ = checked_static_cast<Ogre::TextAreaOverlayElement*>
        (Ogre::OverlayManager::getSingleton().createOverlayElement("TextArea", overlayName_));
//        text_element_ = Ogre::OverlayManager::getSingleton().createOverlayElement("TextArea", "shapeNameText");
    text_element_->setDimensions(0.8, 0.8);
    text_element_->setMetricsMode(Ogre::GMM_PIXELS);
    text_element_->setPosition(1, 2);
    text_element_->setParameter("font_name", fontName);
    text_element_->setParameter("char_height", font_->getParameter("size"));
//        text_element_->setCharHeight(0.035f);
    text_element_->setParameter("horz_align", "left");
    fontColor_ = Color(0, 0, 0, 1);
    text_element_->setColour(Ogre::ColourValue::Black);
    container_->addChild(text_element_);

    if(text_ != "")
    {
        textDim_ = GetTextDimensions(text_);
        container_->setDimensions(textDim_.x, textDim_.y);
    }

    SetMaterial("BlueTransparent"/*baseMaterialName_*/);

    if (visible_)
        overlay_->show();
    else
        overlay_->hide();

    overlay_->setZOrder(100);
}

Ogre::Vector2 EC_OgreMovableTextOverlay::GetTextDimensions(const std::string &text)
{
    Ogre::Vector2 result(0, 0);
    if (renderer_.expired())
        return result;

    for(std::string::const_iterator it = text.begin(); it < text.end(); ++it)
    {
        if (*it == 0x0020)
            result.x += font_->getGlyphAspectRatio(0x0030);
        else
            result.x += font_->getGlyphAspectRatio(*it);
    }

    float charHeight = Ogre::StringConverter::parseReal(font_->getParameter("size"));
    Ogre::Viewport* viewport = renderer_.lock()->GetViewport();

    result.x = (result.x * charHeight) / (float)viewport->getActualWidth();
    result.y = charHeight / (float)viewport->getActualHeight();

    return result;
}

void EC_OgreMovableTextOverlay::SetAlphaChannelIntensity(const float &distance)
{
    float materialAlpha, textAlpha;

    textAlpha = (MAX_VISIBILITY_DISTANCE - distance) / MAX_VISIBILITY_DISTANCE;
    materialAlpha = (MAX_VISIBILITY_DISTANCE - distance) / MAX_VISIBILITY_DISTANCE;
    if (materialAlpha > materialMaxAlpha_)
        materialAlpha = materialMaxAlpha_;

    Ogre::MaterialManager &mm = Ogre::MaterialManager::getSingleton();
    Ogre::MaterialPtr material = mm.getByName(materialName_);
    if (material.get())
        material->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setAlphaOperation(
            Ogre::LBX_SOURCE1, Ogre::LBS_MANUAL, Ogre::LBS_CURRENT, materialAlpha, materialAlpha, 0);

    text_element_->setColour(Ogre::ColourValue(fontColor_.r, fontColor_.g, fontColor_.b, textAlpha));
}

}
