// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogicModule_ScriptDialogHandler_h
#define incl_RexLogicModule_ScriptDialogHandler_h

#include <QObject>
#include <QList>

namespace RexLogic
{
    class RexLogicModule;
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
        explicit ScriptDialogHandler(RexLogicModule *owner);
        ~ScriptDialogHandler();

        //! handle given script dialog request by creating 
        //! ScriptDialogWidget object and showing it
        virtual void Handle(ScriptDialogRequest &request);

    protected:
        void ShowDialog(ScriptDialogRequest& request);

        RexLogicModule *owner_;
        QList<ScriptDialogWidget*> dialogs_;

    private slots:
        // called when a ScriptDialogWidget is closed
        void OnDialogClosed(int channel, const QString &answer);
    };
}

#endif // incl_ScriptDialogHandler_h
