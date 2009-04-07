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
#include "ConfigureDlg.h"

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

	    commUI_XML->connect_clicked("mi_connect", sigc::mem_fun(*this, &CommunicationUIModule::OnAccountMenuConnect));
		commUI_XML->connect_clicked("mi_disconnect", sigc::mem_fun(*this, &CommunicationUIModule::OnAccountMenuDisconnect));
		commUI_XML->connect_clicked("mi_setaccount", sigc::mem_fun(*this, &CommunicationUIModule::OnAccountMenuSetAccountAndPassword));
		commUI_XML->connect_clicked("btnTest", sigc::mem_fun(*this, &CommunicationUIModule::OnAccountMenuConnect));

		wndCommMain->show();
	}

	void CommunicationUIModule::OnAccountMenuSetAccountAndPassword()
	{
		LogInfo("Set account");
		std::map<std::string, Foundation::Comms::SettingsAttribute> attributes = commManager->GetAccountAttributes();
		int count = attributes.size();
		LogInfo(attributes.find("name")->first);

		
		//ConfigureDlg accDlg(count, attributes, "account settings", commManager, this);
		ConfigureDlg accDlg(count, attributes, "account settings", this);
		Gtk::Main::run(accDlg);
		//accDlg.
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

	void CommunicationUIModule::Callback(std::string aConfigName, std::map<std::string, Foundation::Comms::SettingsAttribute> attributes)
	{
		if(aConfigName=="account settings"){ commManager->SetAccountAttributes(attributes); }
	}

}

using namespace Communication;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
POCO_EXPORT_CLASS(CommunicationUIModule)
POCO_END_MANIFEST
