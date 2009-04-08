// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "OgreOverlay.h"
#include "EC_OgreConsoleOverlay.h"
#include "OgreRenderingModule.h"

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

            DisplayCurrentBuffer();
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

        DisplayCurrentBuffer();
    }

    // virtual
    void OgreOverlay::Scroll(int rel)
    {   
        {
            Core::MutexLock lock(mutex_);
            int lines = rel / 20;
            if (static_cast<int>(text_position_) + lines < 0)
                lines = -(lines - (lines - static_cast<int>(text_position_)));

            if (text_position_ + lines > message_lines_.size() - max_visible_lines - 1)
                lines -= (lines - ((message_lines_.size() - max_visible_lines - 1) - text_position_));

            text_position_ += lines;
        }

        DisplayCurrentBuffer();
    }

    void OgreOverlay::SetVisible(bool visible)
    {
        if (console_overlay_)
        {
            checked_static_cast<OgreRenderer::EC_OgreConsoleOverlay*>
                (console_overlay_.get())->SetVisible(visible);
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

    void OgreOverlay::Update(Core::f64 frametime)
    {
        //if (IsVisible())
        {
            checked_static_cast<OgreRenderer::EC_OgreConsoleOverlay*>
                (console_overlay_.get())->Update(frametime);
        }
    }

    void OgreOverlay::DisplayCurrentBuffer()
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
            page = *line + '\n' + page;

            if (num_lines >= max_visible_lines)
                break;
        }

        checked_static_cast<OgreRenderer::EC_OgreConsoleOverlay*>(console_overlay_.get())->Display(page);
    }
}
