// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ConsoleOgre_h
#define incl_ConsoleOgre_h

#include "ModuleInterface.h"
//#include "EC_OgreConsoleOverlay.h"

namespace Foundation
{
    class Framework;
}

namespace OIS
{
    enum KeyCode;
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

        virtual void Scroll(int rel);

        virtual void SetVisible(bool visible);
        virtual bool IsVisible() const;
        virtual bool IsActive() const;

        //! update overlay
        virtual void Update(Core::f64 frametime);

        //! process key down event
        bool HandleKeyDown(OIS::KeyCode code, unsigned int text);

    private:
        //! format a page according to overlay properties so it can be displayed on the console
        /*! Formats the current buffer and outputs it into the parameter

            \param pageOut Formatted page
        */
        void FormatPage(std::string &pageOut);

        //! Prints the specified page to the overlay
        /*! Use FormatPage() to automatically format current buffer contents
        */
        void Display(const std::string &page);

        //! entity component
        Foundation::ComponentPtr console_overlay_;
        
        //! parent module
        Foundation::ModuleInterface *module_;

        //! Log listener for renderer log
        LogListenerPtr log_listener_;

        //! Contains all lines printed to the console
        Core::StringList message_lines_;

        //! current offset for text, 0 is bottom.
        size_t text_position_;

        //! maximum number of lines the console will buffer
        const size_t max_lines_;

        //! max number of visible lines in the console
        size_t max_visible_lines;

        //! timer for displaying prompt
        Core::f64 prompt_timer_;

        //! if true, show prompt cursor
        bool show_cursor_;

        //! Update the contents of the overlay with some new input
        bool update_;

        //! mutex for the console
        Core::Mutex mutex_;
    };
}

#endif
