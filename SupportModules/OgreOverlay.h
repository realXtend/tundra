// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ConsoleOgre_h
#define incl_ConsoleOgre_h

#include "ModuleInterface.h"
//#include "EC_OgreConsoleOverlay.h"

namespace Foundation
{
    class Framework;
}

namespace Console
{
    class ConsoleModule;
    class LogListener;

    typedef boost::shared_ptr<LogListener> LogListenerPtr;

    //! Ogre debug console
    class OgreOverlay : public Console::ConsoleServiceInterface, public boost::noncopyable
    {
        OgreOverlay();
    public:
        //! constructor that takes parent module
        OgreOverlay(Foundation::ModuleInterface *module);

        //! destructor
        virtual ~OgreOverlay();

        //! create the actual overlay. not thread safe
        void Create();

        //! print out message to console. thread safe
        virtual void Print(const std::string &text);

    private:
        void DisplayCurrentBuffer();

        //! entity component
        Foundation::ComponentPtr console_overlay_;
        
        //! parent module
        Foundation::ModuleInterface *module_;

        //! Log listener for renderer log
        LogListenerPtr log_listener_;

        //! Contains all lines printed to the console
        Core::StringList message_lines_;

        //! maximum number of lines the console will buffer
        const size_t max_lines_;

        size_t max_visible_lines;

        //! mutex for the console
        Core::Mutex mutex_;
    };
}

#endif
