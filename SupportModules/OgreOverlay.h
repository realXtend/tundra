// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ConsoleOgre_h
#define incl_ConsoleOgre_h

#include "ModuleInterface.h"

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

        //! Clear the console, clear all buffers
        void Clear()
        {
            Core::MutexLock lock(mutex_);
            message_lines_.clear();
            command_line_.clear();
            command_history_.clear();
            command_history_pos_ = command_history_.begin();
            text_position_ = 0;
            cursor_offset_ = 0;
            current_code_ = 0;
            current_key_ = 0;
        }

        //! print out message to console. thread safe
        virtual void Print(const std::string &text);


        virtual void Scroll(int rel);

        //! set console visible / invisible, not thread safe. Should be called from rendering thread.
        virtual void SetVisible(bool visible);
        //! Returns true if console is visible, false otherwise. Not thread safe, should be called from rendering thread.
        virtual bool IsVisible() const;
        //! Returns true if console is active i.e. it accepts input from keyboard, not thread safe. Should be called from rendering thread.
        virtual bool IsActive() const;

        //! update overlay. Should be called from rendering thread.
        virtual void Update(Core::f64 frametime);

        //! process key down event. Thread safe.
        virtual bool HandleKeyDown(int code, Core::uint text);

        //! process key up event. Thread safe.
        virtual bool HandleKeyUp(int code, Core::uint text);

        //! Returns the current command line, debug use only!
        const std::string &GetCommandLine() const { return command_line_; }

        //! Returns console line, 0 is bottom, debug use only!
        const std::string &GetLine(size_t index) const
        {
            Core::StringList::const_iterator line = message_lines_.begin();
            std::advance(line, index);
            return *line;
        }
        
        //! move cursor to left or right by offset amounr
        /*! Offset is constrained by current command line length

            \param offset negative for left movement, positive for right
        */
        void MoveCursor(int offset);

    private:
        bool HandleKey(int code, Core::uint text);

        //! format a page according to overlay properties so it can be displayed on the console
        /*! Formats the current buffer and outputs it into the parameter

            \param pageOut Formatted page
        */
        void FormatPage(std::string &pageOut);

        //! Prints the specified page to the overlay
        /*! Use FormatPage() to automatically format current buffer contents
        */
        void Display(const std::string &page);

        //! Adds a character to the parameter
        /*! 
            \param character character to embed to lineOut
            \param lineOut string the character should be appended to
            \param offset offset where the character should be appended to
            \return true if 'character' is an allowed char, false otherwise
        */
        bool AddCharacter(unsigned int character, std::string &lineOut, size_t offset);

        //! entity component
        Foundation::ComponentPtr console_overlay_;
        
        //! parent module
        Foundation::ModuleInterface *module_;

        //! Log listener for renderer log
        LogListenerPtr log_listener_;

        //! Contains all lines printed to the console
        Core::StringList message_lines_;

        //! current command line
        std::string command_line_;

        //! buffer for command history
        Core::StringList command_history_;

        //! current position in command history
        Core::StringList::const_iterator command_history_pos_;

        //! maximum number of commands the console will store
        const size_t max_command_history_;

        //! current offset for text, 0 is bottom.
        size_t text_position_;

        //! maximum number of lines the console will buffer
        const size_t max_lines_;

        //! size of one line when scrolling
        const int scroll_line_size_;

        //! max number of visible lines in the console
        size_t max_visible_lines;

        //! timer for displaying prompt
        Core::f64 prompt_timer_;

        //! if true, show prompt cursor
        bool show_cursor_;

        //! current offset for the cursor in command line, 0 for end of word
        size_t cursor_offset_;

        //! Update the contents of the overlay with some new input
        bool update_;

        //! mutex for the console
        Core::Mutex mutex_;

        //! Frequency for the cursor blink, in seconds
        Core::Real cursor_blink_freq_;

        //! default command manager
        CommandManagerPtr command_manager_;

        //! counter for key strokes
        Core::IntervalTimer counter_;

        //! OIS code for the current key held down, or 0 if no key is held down
        int current_code_;

        //! Ascii code for the current key held down, or 0 if no key is held down
        Core::uint current_key_;
    };
}

#endif
