// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLoginWindow_h
#define incl_RexLoginWindow_h

#pragma warning( push )
#pragma warning( disable : 4250 )
#undef max
#include <gtkmm/main.h>
#include <gtkmm/window.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/textview.h>
#include <gtkmm/scrolledwindow.h>
#include <libglademm.h>
#include <glade/glade.h>
#pragma warning( pop )

namespace RexLogic
{
    class RexLogicModule;
    
    class RexLoginWindow
    {
    public:
        RexLoginWindow(Foundation::Framework *framework, RexLogicModule *module);
        ~RexLoginWindow();
        
        /// Initializes the Login window.
        void InitLoginWindow();

        /// Callback funtion for the 'Connect' button. Connects to server.
        void OnClickConnect();
        
        /// Callback function for the 'Disconnect' button. Disconnects from the server.
        void OnClickLogout();
        
        /// Callback function for the 'Quit' button. Terminates the application.
        void OnClickQuit();
                
        // Handle to the login window controls.
        Glib::RefPtr<Gnome::Glade::Xml> loginControls;
        
        // The GTK window for login UI.
        Gtk::Window *loginWindow;
        
        // The GTK entries : server entry, server login.
        Gtk::Entry *entryServer;
        Gtk::Entry *entryUsername;
        
    private:
        RexLoginWindow(const RexLoginWindow &);
//        void operator=(const RexLoginWindow &);
        
        Foundation::Framework *framework_;
        
        RexLogicModule *rexLogic_;
    };
}

#endif
