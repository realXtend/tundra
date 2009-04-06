#pragma warning( push )
#pragma warning( disable : 4250 )
#undef max
#include <gtkmm/main.h>
#include <gtkmm/window.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/actiongroup.h>

#include <libglademm.h>
#include <glade/glade.h>
#pragma warning( pop )

#include <Poco/ClassLibrary.h>
//#include <sstream>

#include "StableHeaders.h"
#include "Foundation.h"

#include "CommunicationUIModule.h"
//#include "PythonScriptModule.h"



namespace Communication
{

	CommunicationUIModule::CommunicationUIModule(void):ModuleInterfaceImpl("CommunicationUIModule")
	{
	}

	CommunicationUIModule::~CommunicationUIModule(void)
	{
	}

	void CommunicationUIModule::Load()
	{
	}
	
	void CommunicationUIModule::Unload()
	{
	}

	void CommunicationUIModule::Initialize() 
	{
	}

	void CommunicationUIModule::PostInitialize()
	{
		initializeMainCommWindow();
		commManager = framework_->GetService<Foundation::Comms::CommunicationManagerServiceInterface>(Foundation::Service::ST_CommunicationManager);
	}

	void CommunicationUIModule::Uninitialize()
	{
	}

	void CommunicationUIModule::Update()
	{
	}

	void CommunicationUIModule::initializeMainCommWindow()
	{
		commUI_XML = Gnome::Glade::Xml::create("data/communicationUI.glade");
		if (!commUI_XML)
			return;
		wndCommMain = 0;
		                        
		commUI_XML->get_widget("wndCMain", wndCommMain);
		if(!wndCommMain)
			return;

		//actionGroup = Gtk::ActionGroup::create();
		//actionGroup->add(Gtk::Action::create("Connect",
  //          Gtk::Stock::NEW, "_Connect", "Connect to server"),
		//	sigc::mem_fun(*this, &CommunicationUIModule::OnAccountMenuConnect));



	    commUI_XML->connect_clicked("mi_connect", sigc::mem_fun(*this, &CommunicationUIModule::OnAccountMenuConnect));
		commUI_XML->connect_clicked("mi_disconnect", sigc::mem_fun(*this, &CommunicationUIModule::OnAccountMenuDisconnect));
		commUI_XML->connect_clicked("mi_setaccount", sigc::mem_fun(*this, &CommunicationUIModule::OnAccountMenuSetAccountAndPassword));
		commUI_XML->connect_clicked("btnTest", sigc::mem_fun(*this, &CommunicationUIModule::OnAccountMenuConnect));

		wndCommMain->show();
	}

	void CommunicationUIModule::OnAccountMenuSetAccountAndPassword()
	{
		LogInfo("Set account");
		dlgAccount = 0;
		commUI_XML->get_widget("dlgSetAccount", dlgAccount);
		if(!dlgAccount)
			return;
	}

	void CommunicationUIModule::OnAccountMenuConnect()
	{
		LogInfo("something clicked");
		commManager->Connect();		

		//PythonScript::PythonScriptModule *pyModule_ = dynamic_cast<PythonScript::PythonScriptModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_PythonScript));
		//pyModule_->DoConnect();
	}
	
	void CommunicationUIModule::OnAccountMenuDisconnect()
	{
		LogInfo("something clicked");
		commManager->Disconnect();	
	}

}

using namespace Communication;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
POCO_EXPORT_CLASS(CommunicationUIModule)
POCO_END_MANIFEST
