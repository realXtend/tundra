// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLoginWindow_h
#define incl_RexLoginWindow_h

#include <QObject>

#include "NetworkEvents.h"
#include "Foundation.h"

class QWidget;
class QPushButton;

namespace RexLogic
{
    class RexLogicModule;
    class RexLoginWindow : public QObject
    {
        Q_OBJECT

    public:
        RexLoginWindow(Foundation::Framework* framework, RexLogicModule *module);
        virtual ~RexLoginWindow();
        
        /// Initializes the Login window.
        void InitLoginWindow();
        	
        // Shows an error dialog.
	//void ShowErrorDialog(std::string message);        
       
        /// Updates the UI with a new connection state string.
        /// @param The connection state enum.
        void UpdateConnectionStateToUI(OpenSimProtocol::Connection::State state);
        
        /// Shows the external login dialog and hides the embedded logout buttons.
        void ShowLoginWindow();

        /// Hides the external login dialog and shows the embedded logout buttons.
        void HideLoginWindow();

    public slots:
        void Connect();
        void Disconnect();
        void DisconnectAndShowLoginWindow();
        void Quit();
	void processCBLogin(QString result);
        
    private:
        /// Hides the login window and shows the logout/quit menu that is embedded to main window.
        void CreateLogoutMenu();

        Foundation::Framework *framework_;
        ///\todo SharedPtr'ize.
        RexLogicModule *rex_logic_;
        QWidget *login_widget_;
        QPushButton *logout_button_;
        QPushButton *quit_button_;
    };
}

#endif
