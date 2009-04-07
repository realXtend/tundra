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
        , max_visible_lines_(3)
        , max_lines_(4)
    {
        CreateOverlay();
    }

    EC_OgreConsoleOverlay::EC_OgreConsoleOverlay(const EC_OgreConsoleOverlay &other) : 
        Foundation::ComponentInterface(other)
        , height_(other.height_)
        , char_height_(other.char_height_)
        , max_visible_lines_(other.max_visible_lines_)
        , max_lines_(other.max_lines_)
    {
        // not meant to be used
        assert (false);
    }

    EC_OgreConsoleOverlay::~EC_OgreConsoleOverlay()
    {
        //! \todo unsurprisingly causes crash when Ogre Root / renderer is destroyed
        // Ogre::OverlayManager::getSingleton().destroy("Console");
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

    void EC_OgreConsoleOverlay::Print(const std::string &text)
    {
        {
            Core::MutexLock lock(mutex_);

            message_lines_.push_front(text);
            if (message_lines_.size() >= max_lines_)
                message_lines_.pop_back();
        }

        DisplayCurrentBuffer();
    }

    void EC_OgreConsoleOverlay::DisplayCurrentBuffer()
    {
        Core::MutexLock lock(mutex_);

        std::string page;
        size_t num_lines = 0;
        for (Core::StringList::const_iterator line = message_lines_.begin() ;
             line != message_lines_.end() ; ++line)
        {
            num_lines++;
            page = *line + '\n' + page;

            if (num_lines >= max_visible_lines_)
                break;
        }

        overlay_element_->setCaption(page);
    }

    void EC_OgreConsoleOverlay::UpdateInternal(Core::f64 frametime)
    {
    }
}
