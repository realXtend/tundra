#ifndef incl_Comm_CommunicationModule_h
#define incl_Comm_CommunicationModule_h

#include "Foundation.h"
#include "ModuleInterface.h"

#include "CommunicationModuleApi.h"
#include "CommunicationManager.h"
#include "ConsoleUI.h"
#include "QtGUI.h"

/**
 *  Communication module
 *
 *  Provides:
 *  - CommunicationManager object for connecting IM servers
 *  - ConsoleUI for text based user interface
 *  - QtGUI for Qt based user interface 
 *
 *  Instructions:
 * - TODO
 *
 */
namespace Communication
{
//	class TpQt4CommunicationManager;
//	typedef boost::shared_ptr<TpQt4CommunicationManager> TpQt4CommunicationManagerPtr;    
    
	class COMMS_MODULE_API CommunicationModule : public Foundation::ModuleInterfaceImpl
	{
	public:
		CommunicationModule(void);
		virtual ~CommunicationModule(void);

		void Load();
		void Unload();
		void Initialize();
		void PostInitialize();
		void Uninitialize();

		void Update(Core::f64 frametime);

		MODULE_LOGGING_FUNCTIONS

		//! returns name of this module. Needed for logging.
		static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }
		static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Communication;

	private:
		TpQt4Communication::CommunicationManager* communication_manager_;
		CommunicationUI::ConsoleUI* console_ui_;
		CommunicationUI::QtGUI* qt_ui_;
	};
}

#endif // incl_Comm_CommunicationModule_h
