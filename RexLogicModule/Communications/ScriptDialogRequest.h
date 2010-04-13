// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogicModule_Communications_ScriptDialogRequest_h
#define incl_RexLogicModule_Communications_ScriptDialogRequest_h

#include "CoreTypes.h"

#include <QObject>

namespace RexLogic
{
    typedef std::vector<std::string> ButtonLabels;
    /**
     * Request to show dialog to user sent by server.
     * 
     */
    class ScriptDialogRequest
    {
    public:
        ScriptDialogRequest(
            const std::string &object_name,
            const std::string &objects_owner_name,
            const std::string &message,
            int channel,
            ButtonLabels &button_labels);

        std::string GetObjectName() const;
        std::string GetObjectsOwnerName() const;
        std::string GetMessageText() const;
        u32 GetChannel() const;
        ButtonLabels GetButtonLabels() const;

    protected:
        std::string object_name_;
        std::string objects_ownwer_name_;
        std::string message_;
        s32 channel_;
        ButtonLabels button_labels_;
    };
}

#endif // incl_RexLogicModule_Communications_ScriptDialogRequest_h
