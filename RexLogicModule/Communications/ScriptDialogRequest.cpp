// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ScriptDialogRequest.h"

namespace RexLogic
{
    ScriptDialogRequest::ScriptDialogRequest(
        const std::string &object_name,
        const std::string &objects_ownwer_name,
        const std::string &message,
        int channel,
        ButtonLabels &button_labels):
        object_name_(object_name),
        objects_ownwer_name_(objects_ownwer_name),
        message_(message),
        channel_(channel),
        button_labels_(button_labels)
    {
    }

    std::string ScriptDialogRequest::GetObjectName() const
    {
        return object_name_;
    }

    std::string ScriptDialogRequest::GetObjectsOwnerName() const
    {
        return objects_ownwer_name_;
    }

    std::string ScriptDialogRequest::GetMessageText() const
    {
        return message_;
    }

    u32 ScriptDialogRequest::GetChannel() const
    {
        return channel_;
    }

    ButtonLabels ScriptDialogRequest::GetButtonLabels() const
    {
        return button_labels_;
    }

} // end of namespace: RexLogic
