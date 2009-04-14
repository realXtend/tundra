// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include <OISKeyboard.h>

#include "OgreOverlay.h"
#include "EC_OgreConsoleOverlay.h"
#include "OgreRenderingModule.h"
#include "InputEvents.h"

namespace Console
{
    class LogListener : public Foundation::LogListenerInterface
    {
        LogListener();
        OgreOverlay *console_;

    public:
        LogListener(OgreOverlay *console) : Foundation::LogListenerInterface(), console_(console) {}
        virtual ~LogListener() {}

        virtual void LogMessage(const std::string &message)
        {
            console_->Print(message);
        }
    };

/////////////////////////////////////////////

    OgreOverlay::OgreOverlay(Foundation::ModuleInterface *module) : 
        Console::ConsoleServiceInterface()
            , module_(module)
            , log_listener_(LogListenerPtr(new LogListener(this)))
            , max_lines_(256)
            , max_visible_lines(1)
            , text_position_(0)
            , prompt_timer_(0)
            , update_(false)
            , scroll_line_size_(20)
            , cursor_offset_(0)
    {
        Foundation::Framework *framework = module_->GetFramework();

        if ( framework->GetModuleManager()->HasModule(Foundation::Module::MT_Renderer) )
        {
            OgreRenderer::OgreRenderingModule *rendering_module = 
                framework->GetModuleManager()->GetModule<OgreRenderer::OgreRenderingModule>(Foundation::Module::MT_Renderer);
            if (rendering_module)
                rendering_module->GetRenderer()->SubscribeLogListener(log_listener_);
        }
    }

    OgreOverlay::~OgreOverlay()
    {
        Foundation::Framework *framework = module_->GetFramework();

        console_overlay_.reset();
        if ( framework->GetServiceManager()->IsRegistered(Foundation::Service::ST_SceneManager))
        {
            Foundation::SceneManagerServiceInterface *scene_manager =
                framework->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager);

            if (scene_manager->HasScene("Console"))
            {
                scene_manager->DeleteScene("Console");
            }
        }

        if ( framework->GetModuleManager()->HasModule(Foundation::Module::MT_Renderer) )
        {
            OgreRenderer::OgreRenderingModule *rendering_module = 
                framework->GetModuleManager()->GetModule<OgreRenderer::OgreRenderingModule>(Foundation::Module::MT_Renderer);
            if (rendering_module)
                rendering_module->GetRenderer()->UnsubscribeLogListener(log_listener_);
        }
    }

    void OgreOverlay::Create()
    {
        Foundation::Framework *framework = module_->GetFramework();

        if ( framework->GetServiceManager()->IsRegistered(Foundation::Service::ST_SceneManager) &&
             framework->GetComponentManager()->CanCreate("EC_OgreConsoleOverlay") )
        {
            Foundation::SceneManagerServiceInterface *scene_manager =
                framework->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager);

            if (scene_manager->HasScene("Console"))
                throw Core::Exception("Scene for console already exists.");

            Foundation::ScenePtr scene = scene_manager->CreateScene("Console");
            Foundation::EntityPtr entity = scene->CreateEntity(scene->GetNextFreeId());

            console_overlay_ = framework->GetComponentManager()->CreateComponent("EC_OgreConsoleOverlay");
            entity->AddEntityComponent(console_overlay_);

            max_visible_lines = checked_static_cast<OgreRenderer::EC_OgreConsoleOverlay*>
                (console_overlay_.get())->GetMaxVisibleLines();

            update_ = true;
        }
    }
    
    // virtual
    void OgreOverlay::Print(const std::string &text)
    {
        {
            Core::MutexLock lock(mutex_);

            message_lines_.push_front(text);
            if (message_lines_.size() >= max_lines_)
                message_lines_.pop_back();
        }

        update_ = true;
    }

    // virtual
    void OgreOverlay::Scroll(int rel)
    {   
        {
            Core::MutexLock lock(mutex_);
            int lines = rel / scroll_line_size_;
            if (static_cast<int>(text_position_) + lines < 0)
                lines = -(lines - (lines - static_cast<int>(text_position_)));

            if (text_position_ + lines > message_lines_.size() - max_visible_lines - 1)
                lines -= (lines - ((message_lines_.size() - max_visible_lines - 1) - text_position_));

            text_position_ += lines;
        }

        update_ = true;
    }

    void OgreOverlay::SetVisible(bool visible)
    {
        if (console_overlay_)
        {
            checked_static_cast<OgreRenderer::EC_OgreConsoleOverlay*>
                (console_overlay_.get())->SetVisible(visible);
        }
        if (visible)
        {
            update_ = true;
        }
    }

    bool OgreOverlay::IsVisible() const
    {
        if (console_overlay_)
        {
            return checked_static_cast<OgreRenderer::EC_OgreConsoleOverlay*>
                (console_overlay_.get())->IsVisible();
        }
        return false;
    }

    bool OgreOverlay::IsActive() const
    {
        if (console_overlay_)
        {
            return checked_static_cast<OgreRenderer::EC_OgreConsoleOverlay*>
                (console_overlay_.get())->IsActive();
        }
        return false;
    }

    void OgreOverlay::Update(Core::f64 frametime)
    {
        checked_static_cast<OgreRenderer::EC_OgreConsoleOverlay*>
                (console_overlay_.get())->Update(frametime);


        // blink prompt cursor
        static bool show_cursor = false;
        prompt_timer_ += frametime;
        if (prompt_timer_ > 0.5f)
        {
            show_cursor = !show_cursor;
            prompt_timer_ = 0.f;
            update_ = true;
        }

        if (update_)
        {
            std::string page;
            FormatPage(page);

            std::string prompt(command_line_);

            // add cursor
            if (show_cursor)
                prompt.insert(prompt.size() - cursor_offset_, "_");
            else if (cursor_offset_ > 0)
                prompt.insert(prompt.size() - cursor_offset_, " ");

            //add the prompt
            page += ">" + prompt;

            Display(page);

            update_ = false;
        }
    }

    bool OgreOverlay::HandleKeyDown(int code, unsigned int text)
    {
        bool result = true;

        switch (code)
        {
        case OIS::KC_PGUP:
            Scroll(scroll_line_size_ * max_visible_lines - 2);
            break;
        case OIS::KC_PGDOWN:
            Scroll(-scroll_line_size_ * max_visible_lines - 2);
            break;
        case OIS::KC_UP:
            Scroll(scroll_line_size_);
            break;
        case OIS::KC_DOWN:
            Scroll(-scroll_line_size_);
            break;
        case OIS::KC_BACK:
            {
                if (command_line_.empty() == false && cursor_offset_ < command_line_.length())
                {
                    command_line_ = command_line_.substr(0, command_line_.length() - cursor_offset_ - 1) + command_line_.substr(command_line_.length() - cursor_offset_, cursor_offset_);
                }
                break;
            }
        case OIS::KC_LEFT:
            MoveCursor(-1);
            break;

        case OIS::KC_RIGHT:
            MoveCursor(1);
            break;

        default:
            result = AddCharacter(text, command_line_, cursor_offset_);
            break;
        }

        if (result)
            update_ = true;

        return result;
    }

    void OgreOverlay::MoveCursor(int offset)
    {
        int offset_neg = -offset;
        if (offset_neg < 0)
        {
            Core::MutexLock lock(mutex_);
            if (cursor_offset_ >= -offset_neg) cursor_offset_ += offset_neg;
        } else
        {
            Core::MutexLock lock(mutex_);
            cursor_offset_ = std::min(cursor_offset_ + offset_neg, command_line_.length());
        }
    }

    void OgreOverlay::FormatPage(std::string &pageOut)
    {
        Core::MutexLock lock(mutex_);

        if (!console_overlay_)
            return;

        std::string page;
        size_t num_lines = 0;
        Core::StringList::const_iterator line = message_lines_.begin();

        assert (message_lines_.size() > text_position_);

        std::advance(line, text_position_);
        for ( ; line != message_lines_.end() ; ++line)
        {
            num_lines++;
            pageOut = *line + '\n' + pageOut;

            if (num_lines >= max_visible_lines)
                break;
        }
    }

    void OgreOverlay::Display(const std::string &page)
    {
        checked_static_cast<OgreRenderer::EC_OgreConsoleOverlay*>(console_overlay_.get())->Display(page);
    }

    bool OgreOverlay::AddCharacter(unsigned int character, std::string &lineOut, size_t offset)
    {
        assert (offset >= 0 && offset <= lineOut.size());
        if (character != 0)
        {
            // validate characters. Might no be strictly necessary.
            static const char legalchars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890+!\"#$§'<>|%&/()=?[]{}\\*-_.:,; ";
            int c;
            for (c = 0 ; c<sizeof(legalchars) - 1 ; ++c)
            {
                if (legalchars[c] == character)
                {
                    std::string s_char;
                    s_char += character;

                    lineOut.insert(lineOut.size() - offset, std::string(s_char));
                    return true;
                }
            }
        }
        return false;
    }
}
