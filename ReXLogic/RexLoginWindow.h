// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLoginWindow_h
#define incl_RexLoginWindow_h

#pragma warning( push )
#pragma warning( disable : 4250 )
#include <gtkmm/main.h>
#include <gtkmm/window.h>
#include <gtkmm/entry.h>
#include <gtkmm/messagedialog.h>
#include <libglademm.h>
#include <glade/glade.h>
#pragma warning( pop )

#include "NetworkEvents.h"

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
        
        /// Shows an error dialog.
        void ShowErrorDialog(std::string message);
        
        /// Callback funtion for the 'Connect' button. Connects to server.
        void OnClickConnect();
        
        /// Callback function for the 'Disconnect' button. Disconnects from the server.
        void OnClickLogout();
        
        /// Callback function for the 'Quit' button. Terminates the application.
        void OnClickQuit();

        /// Updates the UI with
        ///@param The connection state enum.
        void UpdateConnectionStateToUI(OpenSimProtocol::Connection::State state);
                
        /// Handle to the login window controls.
        Glib::RefPtr<Gnome::Glade::Xml> loginControls;
        
        /// The GTK window for login UI.
        Gtk::Window *loginWindow;
        
        /// The GTK entry box for server address.
        Gtk::Entry *entryServer;
        
        /// The GTK entry box for username .
        Gtk::Entry *entryUsername;
        
    private:
        RexLoginWindow(const RexLoginWindow &);
        void operator=(const RexLoginWindow &);
        
        Foundation::Framework *framework_;
        
        RexLogicModule *rexLogic_;
    };
}

#endif
