// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_SessionManager_h
#define incl_Communication_SessionManager_h

#include <QObject>

#include "ui_SessionManagerWidget.h"
#include "interface.h"

namespace UiHelpers
{
    class SessionManager : public QObject
    {

    Q_OBJECT
        
    public:
        SessionManager();
        virtual ~SessionManager();

        void SetupUi(Ui::SessionManagerWidget *session_manager_ui) { SAFE_DELETE(session_manager_ui_); session_manager_ui_ = session_manager_ui; }
        void Start(const QString &username, Communication::ConnectionInterface *im_connection);

    private:
        Ui::SessionManagerWidget *session_manager_ui_;
        Communication::ConnectionInterface *im_connection_;

        QMenuBar *menu_bar_;
    };
}

#endif // incl_Communication_SessionManager_h