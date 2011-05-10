var iseditionless = framework.IsEditionless();
var actionQuit;

if (!framework.IsHeadless() && !iseditionless)
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");
	var actions_parent = ui.MainWindow();

	if (!uiservice)
		return;

	//Create menus structure
	var fileMenu = uiservice.AddExternalMenu("File", 99);
	var fileMenu = uiservice.AddExternalMenu("View", 60);
	var fileMenu = uiservice.AddExternalMenu("ToolBars", 59);
	var fileMenu = uiservice.AddExternalMenu("Window", 58);
	var fileMenu = uiservice.AddExternalMenu("Scripts", 57);
	var helpMenu = uiservice.AddExternalMenu("Help", 40);
	
	
	
    if (framework.GetModuleQObj("UpdateModule") && fileMenu)
	{
		var action = new QAction(new QIcon("./data/ui/images/icon/update.ico"), "Check Updates", actions_parent);
		action.triggered.connect(CheckForUpdates);
		uiservice.AddExternalMenuAction(action, "Check Updates", "File");
	}
       
    // Reconnect menu items for client only
    if (!server.IsAboutToStart())
    {
        var disconnectAction = new QAction(new QIcon("./data/ui/images/icon/disconnect.ico"), "Disconnect", actions_parent);
        disconnectAction.triggered.connect(Disconnect);
        client.Connected.connect(Connected);
        client.Disconnected.connect(Disconnected);
		uiservice.AddExternalMenuAction(disconnectAction, "Disconnect", "File");
        Disconnected();
    }
	actionQuit = new QAction(new QIcon("./data/ui/images/icon/system-shutdown.ico"), "Quit", actions_parent);
	actionQuit.triggered.connect(Quit);
	//actionQuit.setText("Quit");
	uiservice.AddExternalMenuAction(actionQuit, "Quit", "File", 1);
	
	if (helpMenu)
	{
		var actionOpenWikiUrl = new QAction(new QIcon("./data/ui/images/icon/browser.ico"), "Wiki", actions_parent);
		actionOpenWikiUrl.triggered.connect(OpenWikiUrl);
		uiservice.AddExternalMenuAction(actionOpenWikiUrl, "Wiki", "Help");
		
		var actionOpenDoxygenUrl = new QAction(new QIcon("./data/ui/images/icon/browser.ico"), "Doxygen", actions_parent);
		actionOpenDoxygenUrl.triggered.connect(OpenDoxygenUrl);
		uiservice.AddExternalMenuAction(actionOpenDoxygenUrl, "Doxygen", "Help");
		
		var actionOpenMailingListUrl = new QAction(new QIcon("./data/ui/images/icon/browser.ico"), "Mailing list", actions_parent);
		actionOpenMailingListUrl.triggered.connect(OpenMailingListUrl);
		uiservice.AddExternalMenuAction(actionOpenMailingListUrl, "Mailing list", "Help");
	}

    function NewScene()
    {
        scene.RemoveAllEntities();
    }

    function Reconnect()
    {
        client.Reconnect();
    }

    function Disconnect()
    {
        client.Logout();
    }

    function Connected()
    {
        disconnectAction.setEnabled(true);
    }

    function Disconnected()
    {
        disconnectAction.setEnabled(false);
    }

    function Quit()
    {
        framework.Exit();
    }

    function CheckForUpdates()
    {
        if (framework.GetModuleQObj("UpdateModule"))
            framework.GetModuleQObj("UpdateModule").RunUpdater("/checknow");
    }
       
    function OpenMailingListUrl()
    {
        QDesktopServices.openUrl(new QUrl("http://groups.google.com/group/realxtend/"));
    }
    
    function OpenWikiUrl()
    {
        QDesktopServices.openUrl(new QUrl("http://wiki.realxtend.org/"));
    }
    
    function OpenDoxygenUrl()
    {
        QDesktopServices.openUrl(new QUrl("http://www.realxtend.org/doxygen/"));
    }
}
