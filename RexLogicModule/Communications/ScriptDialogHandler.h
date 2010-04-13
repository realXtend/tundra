// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ScriptDialogHandler_h
#define incl_ScriptDialogHandler_h

#include <QObject>
#include <QList>

namespace Foundation
{
    class Framework;
}

namespace RexLogic
{
    class ScriptDialogRequest;
    class ScriptDialogWidget;

    /**
     * Handles all ScriptDialog requests sent by server.  ScriptDialog request 
     * is sent when llDialog method is called from server side script.
     *
     */
    class ScriptDialogHandler : QObject
    {
        Q_OBJECT

    public:
        ScriptDialogHandler(Foundation::Framework* framework);
        ~ScriptDialogHandler();

        //! handle given script dialog request by creating 
        //! ScriptDialogWidget object and showing it
        virtual void Handle(ScriptDialogRequest &request);

    protected:
        void ShowDialog(ScriptDialogRequest& request);

        Foundation::Framework* framework_;
        QList<ScriptDialogWidget*> dialogs_;

    private slots:
        // called when a ScriptDialogWidget is closed
        void OnDialogClosed(int channel, QString answer);
    };
}

#endif // incl_ScriptDialogHandler_h
