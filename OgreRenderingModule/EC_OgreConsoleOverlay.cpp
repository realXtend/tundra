// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include <Ogre.h>
#include <OgreTextAreaOverlayElement.h>

#include "EC_OgreConsoleOverlay.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"

namespace OgreRenderer
{
    EC_OgreConsoleOverlay::EC_OgreConsoleOverlay(Foundation::ModuleInterface* module) : 
        Foundation::ComponentInterface(module->GetFramework())
        , overlay_element_(NULL)
        , container_(NULL)
        , overlay_(NULL)
        , renderer_(checked_static_cast<OgreRenderingModule*>(module)->GetRenderer())
        , height_(0.4f)
        , char_height_(0.018f)
        , visible_(false)
        , max_visible_lines_(21)
    {
        CreateOverlay();
    }

    EC_OgreConsoleOverlay::EC_OgreConsoleOverlay(const EC_OgreConsoleOverlay &other) : 
        Foundation::ComponentInterface(other)
        , height_(other.height_)
        , char_height_(other.char_height_)
        , max_visible_lines_(other.max_visible_lines_)
    {
        // not meant to be used but still needed, since components are generally meant to be copyable
        assert (false);
    }

    EC_OgreConsoleOverlay::~EC_OgreConsoleOverlay()
    {
        // Funny thing, Ogre manual says not to destroy overlay elements while
        // they are still contained in some Overlay but doing this in any other
        // way seems to cause crashes.
        Ogre::OverlayManager::getSingleton().destroyOverlayElement("ConsoleText");
        Ogre::OverlayManager::getSingleton().destroyOverlayElement("ConsoleContainer");
        Ogre::OverlayManager::getSingleton().destroy("Console");
    }


    void EC_OgreConsoleOverlay::CreateOverlay()
    {
        // Creating second console overlay component is a programming error in any case,
        // so we let Ogre throw exception if that happens.

        overlay_element_ = checked_static_cast<Ogre::TextAreaOverlayElement*>(Ogre::OverlayManager::getSingleton().createOverlayElement("TextArea", "ConsoleText"));
        overlay_element_->setCaption("Console");
        overlay_element_->setColour(Ogre::ColourValue::Black);
        overlay_element_->setDimensions(0.98f, height_);

        overlay_element_->setPosition(0.01f, 0.01f);
        overlay_element_->setFontName("Console");
        overlay_element_->setCharHeight(char_height_);
        overlay_element_->show();

        overlay_ = Ogre::OverlayManager::getSingleton().create("Console");

        container_ = checked_static_cast<Ogre::OverlayContainer*>(Ogre::OverlayManager::getSingleton().createOverlayElement("Panel", "ConsoleContainer"));
        container_->setPosition(0.f, -height_);
        container_->setPosition(0.f, 0);
        container_->setDimensions(1.f, height_);
        container_->setMaterialName("Console");
        container_->addChild(overlay_element_);
        container_->show();

        overlay_->add2D(container_);
        if (visible_)
            overlay_->show();
        else
            overlay_->hide();

        overlay_->setZOrder(500);
    }

    void EC_OgreConsoleOverlay::Display(const std::string &text)
    {
        //! \todo render mutex
        overlay_element_->setCaption(text);
    }


    void EC_OgreConsoleOverlay::SetVisible(bool visible)
    {
        if (visible)
            overlay_->show();
        else
            overlay_->hide();
    }

    bool EC_OgreConsoleOverlay::IsVisible() const
    {
        return overlay_->isVisible();
    }
}
