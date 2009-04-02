
#ifndef incl_CommunicationUIModule_h
#define incl_CommunicationUIModule_h

#pragma once

#include "ModuleInterface.h"


//#pragma once

namespace Communication
{
	//
	class MODULE_API CommunicationUIModule : public Foundation::ModuleInterfaceImpl
	{
	public:
		CommunicationUIModule(void);
		virtual ~CommunicationUIModule(void);

		void Load();
		void Unload();
		void Initialize();
		void PostInitialize();
		void Uninitialize();

		void Update();

		MODULE_LOGGING_FUNCTIONS

			
		//! returns name of this module. Needed for logging.
		static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }
		static const Foundation::Module::Type type_static_ = Foundation::Module::MT_CommunicationUI;

	private:
		void initializeMainCommWindow();
		void OnAccountMenuConnect();
		void OnAccountMenuDisconnect();

		
		Glib::RefPtr<Gnome::Glade::Xml> commUI_XML;
		Gtk::Window *wndCommMain;
		Foundation::CommunicationManagerServiceInterface *commManager;
		//Foundation::CommunicationUIManagerPtr CommunicationUI_manager_;
	};
}

#endif