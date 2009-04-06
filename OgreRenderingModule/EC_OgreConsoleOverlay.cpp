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
        , renderer_(static_cast<OgreRenderingModule*>(module)->GetRenderer())
        , height_(0.4f)
    {
        // Creating second console overlay component is a programming error in any case,
        // so we let Ogre throw exception if that happens.

        overlay_element_ = static_cast<Ogre::TextAreaOverlayElement*>(Ogre::OverlayManager::getSingleton().createOverlayElement("TextArea", "ConsoleText"));
        overlay_element_->setCaption("Console");
        overlay_element_->setColour(Ogre::ColourValue::Black);
        overlay_element_->setDimensions(0.98f, height_);

        overlay_element_->setPosition(0.01f, 0.01f);
        overlay_element_->setFontName("BlueHighway");
        overlay_element_->setCharHeight(0.018f);
        overlay_element_->show();

        overlay_ = Ogre::OverlayManager::getSingleton().create("Console");

        container_ = static_cast<Ogre::OverlayContainer*>(Ogre::OverlayManager::getSingleton().createOverlayElement("Panel", "ConsoleContainer"));
        container_->setPosition(0.f, -height_);
        container_->setPosition(0.f, 0);
        container_->setDimensions(1.f, height_);
        container_->setMaterialName("Console");
        container_->addChild(overlay_element_);
        container_->show();

        overlay_->add2D(container_);
        overlay_->show();
        overlay_->hide();
        overlay_->setZOrder(500);
    }

    EC_OgreConsoleOverlay::~EC_OgreConsoleOverlay()
    {
    }

    void EC_OgreConsoleOverlay::Print(const std::string &text)
    {
        overlay_element_->setCaption(text);
    }
}
