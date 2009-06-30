// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include <OISKeyboard.h>

#include "OgreOverlay.h"
#include "EC_OgreConsoleOverlay.h"
#include "OgreRenderingModule.h"
#include "InputEvents.h"
#include "ConsoleModule.h"
#include "ConsoleManager.h"
#include "CommandManager.h"
#include "SceneManager.h"

namespace Console
{

//////////////////////////////////////////

    //   Log listeners

    class LogListener : public Foundation::LogListenerInterface
    {
        LogListener();
        
    public:
        LogListener(OgreOverlay *console) : Foundation::LogListenerInterface(), console_(console) {}
        virtual ~LogListener() {}

        virtual void LogMessage(const std::string &message)
        {
            if (console_)
                console_->Print(message);
        }

        OgreOverlay *console_;
    };

    class PocoLogChannel : public Poco::Channel
    {
    private:
        //! default constructor not applicable
        PocoLogChannel();

    public:
        PocoLogChannel(Console::ConsoleServiceInterface *console) : Poco::Channel(), console_(console) { assert(console_ && "Console not set for PocoLogChannel log listener!"); }
        virtual ~PocoLogChannel() {}
        
        void log(const Poco::Message & msg) { console_->Print(msg.getText()); }
    private:
        Console::ConsoleServiceInterface *console_;
    };

/////////////////////////////////////////////

    OgreOverlay::OgreOverlay(Foundation::ModuleInterface *module) : 
        Console::ConsoleServiceInterface()
            , module_(module)
            , log_listener_(LogListenerPtr(new LogListener(this)))
            , poco_log_channel_(PocoLogChannelPtr(new PocoLogChannel(this)))
            , max_lines_(256)
            , max_visible_lines(1)
            , text_position_(0)
            , prompt_timer_(0)
            , update_(false)
            , scroll_line_size_(20)
            , cursor_offset_(0)
            , max_command_history_(50)
            , command_history_pos_(command_history_.begin())
            , current_key_(0)
            , current_code_(0)
    {
        Foundation::Framework *framework = module_->GetFramework();

        framework->AddLogChannel(poco_log_channel_.get());

        const Foundation::ConfigurationManager &config = framework->GetDefaultConfig();
        cursor_blink_freq_ = config.DeclareSetting("DebugConsole", "cursor_blink_frequency", 0.5f);
                
        Core::f64 slow = config.DeclareSetting("DebugConsole", "key_repeat_slow", 0.5f);
        Core::f64 fast = config.DeclareSetting("DebugConsole", "key_repeat_fast", 0.045f);
        Core::f64 change = config.DeclareSetting("DebugConsole", "key_repeat_change", 0.5f);
        counter_.Reset(slow, fast, change);
    }

    OgreOverlay::~OgreOverlay()
    {
        Foundation::Framework *framework = module_->GetFramework();

        framework->RemoveLogChannel(poco_log_channel_.get());

        console_overlay_.reset();

        if (framework->HasScene("Console"))
        {
            framework->RemoveScene("Console");
        }

        boost::shared_ptr<Foundation::RenderServiceInterface> renderer = 
            framework->GetService<Foundation::RenderServiceInterface>(Foundation::Service::ST_Renderer).lock();
        if (renderer)
        {
            renderer->UnsubscribeLogListener(log_listener_);
        }
    }

    void OgreOverlay::Create()
    {
        Foundation::Framework *framework = module_->GetFramework();

        boost::shared_ptr<Foundation::RenderServiceInterface> renderer = 
            framework->GetService<Foundation::RenderServiceInterface>(Foundation::Service::ST_Renderer).lock();
        if (renderer && framework->GetComponentManager()->CanCreate("EC_OgreConsoleOverlay"))
        {
            renderer->SubscribeLogListener(log_listener_);
        
            if (framework->HasScene("Console"))
                throw Core::Exception("Scene for console already exists."); // could be assert also

            Scene::ScenePtr scene = framework->CreateScene("Console");
            Scene::EntityPtr entity = scene->CreateEntity(scene->GetNextFreeId());

            console_overlay_ = framework->GetComponentManager()->CreateComponent("EC_OgreConsoleOverlay");
            entity->AddEntityComponent(console_overlay_);

            max_visible_lines = checked_static_cast<OgreRenderer::EC_OgreConsoleOverlay*>
                (console_overlay_.get())->GetMaxVisibleLines();

        }

        update_ = true;

        ConsolePtr manager = checked_static_cast<ConsoleModule*>(module_)->GetConsole();
        command_manager_ = checked_static_cast<ConsoleManager*>(manager.get())->GetCommandManager();
    }
    
    // virtual
    void OgreOverlay::Print(const std::string &text)
    {
        if (text.empty())
            return;

        {
            Core::MutexLock lock(mutex_);

            // split text by line breaks
            std::string line;
            line.reserve(text.size());
            for (size_t n=0 ; n<text.size() ; ++n)
            {
                if (text[n] != '\n')
                    line += text[n];

                if (text[n] == '\n' || n == text.size() - 1)
                {
                    message_lines_.push_front(line);
                    if (message_lines_.size() >= max_lines_)
                        message_lines_.pop_back();

                    line.clear();
                }
                
           }

           update_ = true;
        }
    }

    // virtual
    void OgreOverlay::Scroll(int rel)
    {   
        {
            Core::MutexLock lock(mutex_);
            int lines = rel / scroll_line_size_;
            if (static_cast<int>(text_position_) + lines < 0)
                lines = -(lines - (lines - static_cast<int>(text_position_)));
            else if (text_position_ + lines > static_cast<int>(message_lines_.size()) - static_cast<int>(max_visible_lines) - 1)
                lines -= (lines - ((message_lines_.size() - max_visible_lines) - text_position_));
            else if (static_cast<int>(message_lines_.size()) - static_cast<int>(max_visible_lines) <= 0)
                lines = 0;
            
            text_position_ += lines;

            update_ = true;
        }
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
        if (console_overlay_)
        {
            checked_static_cast<OgreRenderer::EC_OgreConsoleOverlay*>
                    (console_overlay_.get())->Update(frametime);
        }

        bool update = false;
        bool repeat_key = false;
        int code = 0;
        Core::uint text = 0;
        {
            Core::MutexLock lock(mutex_);
            update = update_;

            if (current_key_ || current_code_)
            {
                if (frametime == 0)
                    frametime = 0.0001;
                if (counter_.Tick(frametime))
                {
                    repeat_key = true;
                    code = current_code_;
                    text = current_key_;
                }
            }
        }

        if (repeat_key)
        {
            HandleKey(code, text);
            update = true;
        }

        // blink prompt cursor
        static bool show_cursor = false;
        prompt_timer_ += frametime;
        if (prompt_timer_ > cursor_blink_freq_)
        {
            show_cursor = !show_cursor;
            prompt_timer_ = 0.f;
            update = true;
        }

        if (update)
        {
            std::string page;
            FormatPage(page);

            std::string prompt;
            size_t cursor_offset;
            {
                Core::MutexLock lock(mutex_);

                prompt = command_line_;
                cursor_offset = cursor_offset_;
                update_ = false;
            }

            // add cursor
            if (show_cursor)
                prompt.insert(prompt.size() - cursor_offset, "_");
            else if (cursor_offset > 0)
                prompt.insert(prompt.size() - cursor_offset, " ");

            //add the prompt
            page += ">" + prompt;

            Display(page);
        }
    }

    bool OgreOverlay::HandleKeyDown(int code, Core::uint text)
    {
        bool result = HandleKey(code, text);

        if (result)
        {
            Core::MutexLock lock(mutex_);

            update_ = true;
            current_code_ = code;
            current_key_ = text;
            counter_.Reset();
        }

        return result;
    }

    bool OgreOverlay::HandleKey(int code, Core::uint text)
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
            {
                Core::MutexLock lock(mutex_);

                if (command_history_pos_ != command_history_.begin())
                {
                    --command_history_pos_;
                    command_line_ = *command_history_pos_;
                    cursor_offset_ = 0;
                }
            }
            break;
        case OIS::KC_DOWN:
            {
                Core::MutexLock lock(mutex_);

                Core::StringList::const_iterator new_pos = command_history_pos_;
                if (new_pos != command_history_.end())
                    ++new_pos;

                if (new_pos != command_history_.end())
                {
                    command_history_pos_ = new_pos;
    
                    command_line_ = *command_history_pos_;
                    cursor_offset_ = 0;
                }
            }
            break;
        case OIS::KC_BACK:
            {
                Core::MutexLock lock(mutex_);
                if (command_line_.empty() == false && cursor_offset_ < command_line_.length())
                {
                    command_line_ = command_line_.substr(0, command_line_.length() - cursor_offset_ - 1)  +  command_line_.substr(command_line_.length() - cursor_offset_, cursor_offset_);
                }
                break;
            }
        case OIS::KC_DELETE:
            {
                Core::MutexLock lock(mutex_);
                if (command_line_.empty() == false && cursor_offset_ != 0)
                {
                    command_line_ = command_line_.substr(0, command_line_.length() - cursor_offset_)  +  command_line_.substr(command_line_.length() - cursor_offset_ + 1, cursor_offset_ - 1);
                    cursor_offset_--;
                }
                break;
            }
        case OIS::KC_LEFT:
            MoveCursor(-1);
            break;

        case OIS::KC_RIGHT:
            MoveCursor(1);
            break;

        case OIS::KC_RETURN:
            {
                std::string command_line;
                {
                    Core::MutexLock lock(mutex_);
                    command_line = command_line_;
                    command_line_.clear();
                    text_position_ = 0;
                    cursor_offset_ = 0;

                    // store command line to history
                    if (command_history_.size() >= max_command_history_)
                        command_history_.pop_front();

                    command_history_.push_back(command_line);

                    command_history_pos_ = command_history_.end();
                }
                if (command_manager_)
                    command_manager_->QueueCommand(command_line);

                Print(command_line);
            }
            break;

        default:
            {
                Core::MutexLock lock(mutex_);
                result = AddCharacter(text, command_line_, cursor_offset_);
            }
            break;
        }

        return result;
    }

    bool OgreOverlay::HandleKeyUp(int code, Core::uint text)
    {
        Core::MutexLock lock(mutex_);
        bool result = (current_code_ != 0);

        current_key_ = 0;
        current_code_ = 0;

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

        //! \todo We could split long lines for nice word-wrap on the console, but problem is figuring out how long a line is.
        //!       Currently in the font used, not all characters are fixed-width (space...).

        std::string page;
        size_t num_lines = 0;
        Core::StringList::const_iterator line = message_lines_.begin();

        assert (message_lines_.size() >= text_position_);

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
        if (console_overlay_)
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
