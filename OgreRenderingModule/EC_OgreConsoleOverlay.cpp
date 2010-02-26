// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include <Ogre.h>
#include <OgreTextAreaOverlayElement.h>

#include "EC_OgreConsoleOverlay.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "ConfigurationManager.h"

namespace OgreRenderer
{

EC_OgreConsoleOverlay::EC_OgreConsoleOverlay(Foundation::ModuleInterface* module) :
    EC_OgreConsoleOverlayInterface(module->GetFramework()),
    overlay_element_(0),
    container_(0),
    overlay_(0),
    renderer_(checked_static_cast<OgreRenderingModule*>(module)->GetRenderer()),
    height_(module->GetFramework()->GetDefaultConfig().DeclareSetting("DebugConsole", "height", 0.4f)),
//        , char_height_(0.0175f),
    char_height_(module->GetFramework()->GetDefaultConfig().DeclareSetting("DebugConsole", "font_height", 0.0175f)),
    visible_(false),
    max_visible_lines_( height_ / char_height_ * 0.91876f ),
    position_(0),
    speed_(module->GetFramework()->GetDefaultConfig().DeclareSetting("DebugConsole", "appear_speed", 3.0f)),
    active_(visible_)
{
    CreateOverlay();
}

EC_OgreConsoleOverlay::EC_OgreConsoleOverlay(const EC_OgreConsoleOverlay &other) : 
    EC_OgreConsoleOverlayInterface(other),
    height_(other.height_),
    char_height_(other.char_height_),
    max_visible_lines_(other.max_visible_lines_),
    speed_(3.0f)
{
    // not meant to be used but still needed, since components are generally meant to be copyable
    assert (false);
}

EC_OgreConsoleOverlay::~EC_OgreConsoleOverlay()
{
    // Funny thing, Ogre manual says not to destroy overlay elements while
    // they are still contained in some Overlay but doing this in any other
    // way seems to cause crashes.
    if (renderer_.expired() == false)
    {
        Ogre::OverlayManager::getSingleton().destroyOverlayElement("ConsoleText");
        Ogre::OverlayManager::getSingleton().destroyOverlayElement("ConsoleContainer");
        Ogre::OverlayManager::getSingleton().destroy("Console");
    }
}

void EC_OgreConsoleOverlay::CreateOverlay()
{
    // Creating second console overlay component is a programming error in any case,
    // so we let Ogre throw exception if that happens.

    if (renderer_.expired())
        return;

    overlay_element_ = checked_static_cast<Ogre::TextAreaOverlayElement*>(
        Ogre::OverlayManager::getSingleton().createOverlayElement("TextArea", "ConsoleText"));
    overlay_element_->setCaption("Console");
    overlay_element_->setColour(Ogre::ColourValue::White);
    overlay_element_->setDimensions(0.98f, height_);

    overlay_element_->setPosition(0.01f, 0.01f);
    overlay_element_->setFontName("Console");
    overlay_element_->setCharHeight(char_height_);
    overlay_element_->show();

    overlay_ = Ogre::OverlayManager::getSingleton().create("Console");

    container_ = checked_static_cast<Ogre::OverlayContainer*>(
        Ogre::OverlayManager::getSingleton().createOverlayElement("Panel", "ConsoleContainer"));
    container_->setPosition(0.f, -height_);
    container_->setDimensions(1.f, height_);
    container_->setMaterialName("Console");
    container_->addChild(overlay_element_);
    container_->show();

    overlay_->add2D(container_);
    if (visible_)
        overlay_->show();
    else
        overlay_->hide();

    overlay_->setZOrder(501);
}

void EC_OgreConsoleOverlay::Display(const std::string &text)
{
    //! \todo render mutex here -cm

    if (renderer_.expired())
        return;
    overlay_element_->setCaption(text);
}

void EC_OgreConsoleOverlay::SetVisible(bool visible)
{
    visible_ = visible;
    if (visible && !renderer_.expired())
        overlay_->show();

    active_ = visible;
}

bool EC_OgreConsoleOverlay::IsVisible() const
{
    if (renderer_.expired())
        return false;

    return overlay_->isVisible();
}

void EC_OgreConsoleOverlay::Update(f64 frametime)
{
    if (renderer_.expired())
        return;

    if (visible_ && position_ < Ogre::Math::HALF_PI)
    {
        position_ += frametime * speed_;
        if (position_ > Ogre::Math::HALF_PI) 
            position_ = Ogre::Math::HALF_PI;

        container_->setTop( -height_ + Ogre::Math::Sin(position_) * height_);
    }
    else if (visible_ == false && abs(position_) > 0.001f)
    {
        position_ -= frametime * speed_;
        if (position_ <= 0.f)
        {
            position_ = 0.f;
            overlay_->hide();
        }

        container_->setTop( -height_ + Ogre::Math::Sin(position_) * height_);
    }
}

}
