// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogicModule_Communications_ScriptDialogRequest_h
#define incl_RexLogicModule_Communications_ScriptDialogRequest_h

#include <Foundation.h>
#include <QObject>
#include "ModuleInterface.h"

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
		ScriptDialogRequest(std::string object_name, std::string objects_owner_name, std::string message,  s32 channel, ButtonLabels &button_labels);
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

} // end of namespace: incl_RexLogicModule_Communications_ScriptDialogRequest_h

#endif // incl_ScriptDialogRequest_h
