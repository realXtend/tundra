// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ScriptDialogHandler_h
#define incl_ScriptDialogHandler_h

#include <Foundation.h>
#include <QObject>
#include <QList>
#include "ModuleInterface.h"
#include "ScriptDialogRequest.h"
#include "ScriptDialogWidget.h"

namespace RexLogic
{
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
		Foundation::Framework* framework_;
		QList<ScriptDialogWidget*> dialogs_;

		void ShowDialog(ScriptDialogRequest& request);

	private slots:

		// called when a ScriptDialogWidget is closed
		void OnDialogClosed(s32 channel, QString answer);
	};

} // end of namespace: RexLogic

#endif // incl_ScriptDialogHandler_h
