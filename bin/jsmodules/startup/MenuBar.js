if (!framework.IsHeadless())
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");

    var menu = ui.MainWindow().menuBar();
    menu.clear();

    var fileMenu = menu.addMenu("&File");                     
    //fileMenu.addAction("New scene").triggered.connect(NewScene);
    // Reconnect menu items for client only
    if (!server.IsAboutToStart())
    {
        var disconnectAction = fileMenu.addAction("Disconnect");
        disconnectAction.triggered.connect(Disconnect);
        client.Connected.connect(Connected);
        client.Disconnected.connect(Disconnected);
        Disconnected();
    }
    fileMenu.addAction("Quit").triggered.connect(Quit);

    /*var viewMenu = menu.addMenu("&View");
    if (framework.GetModuleQObj("SceneStructure"))
    {
        viewMenu.addAction("Assets").triggered.connect(OpenAssetsWindow);
        viewMenu.addAction("Scene").triggered.connect(OpenSceneWindow);
    }

    if (framework.GetModuleQObj("Console"))
    {
        viewMenu.addAction("Console").triggered.connect(OpenConsoleWindow);  
    }

    //var eceditorAction = viewMenu.addAction("EC Editor");

    if (framework.GetModuleQObj("DebugStats"))
        viewMenu.addAction("Profiler").triggered.connect(OpenProfilerWindow);

    if (framework.GetModuleQObj("Environment"))
        viewMenu.addAction("Terrain Editor").triggered.connect(OpenTerrainEditor);

    if (framework.GetModuleQObj("PythonScript"))
        viewMenu.addAction("Python Console").triggered.connect(OpenPythonConsole);
*/
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

	/*
    function OpenSceneWindow()
    {
        framework.GetModuleQObj("SceneStructure").ShowSceneStructureWindow();
    }

    function OpenAssetsWindow()
    {
        framework.GetModuleQObj("SceneStructure").ShowAssetsWindow();
    }

    function OpenProfilerWindow()
    {
        console.ExecuteCommand("prof");
    }

    function OpenTerrainEditor()
    {
        console.ExecuteCommand("TerrainTextureEditor");
    }

    function OpenPythonConsole()
    {
        console.ExecuteCommand("pythonconsole");
    }

    function OpenConsoleWindow()
    {
        framework.GetModuleQObj("Console").ToggleConsole();
    }
	*/
}
