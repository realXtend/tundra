// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLoginWindow_h
#define incl_RexLoginWindow_h


#include <QObject>
#include <QWidget>

#include "NetworkEvents.h"
#include "Foundation.h"

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
        
       
        /// Updates the UI with
        ///@param The connection state enum.
        void UpdateConnectionStateToUI(OpenSimProtocol::Connection::State state);
                
        

    public slots:
	void Connect();
	void Disconnect();
	void Quit();
        
    private:
       
        
        Foundation::Framework *framework_;
        
        RexLogicModule *rexLogic_;
	QWidget *pLogin_widget_;
    };
}

#endif
