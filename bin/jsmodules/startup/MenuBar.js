/**
    For conditions of distribution and use, see copyright notice in LICENSE

    Implements basic menu bar for accessing common Tundra functionality and UIs. */

// Applicable only in headful mode.
if (!framework.IsHeadless())
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");

    // Use absolute paths for rel image files so they always work!
    var installDir = QDir.fromNativeSeparators(application.installationDirectory);
    
    // Get the menubar
    var menu = ui.MainWindow().menuBar();
    menu.clear();

    // File menu
    var fileMenu = menu.addMenu("&File");
    if (framework.GetModuleByName("UpdateModule"))
        fileMenu.addAction(new QIcon(installDir + "data/ui/images/icon/update.ico"), "Check Updates").triggered.connect(CheckForUpdates);

    var screenshotAct = fileMenu.addAction("Take Screenshot");
    screenshotAct.triggered.connect(TakeScreenshot);
    screenshotAct.enabled = false;

    //fileMenu.addAction("New scene").triggered.connect(NewScene);
    // Reconnect menu items for client only
    if (!server.IsAboutToStart())
    {
        var disconnectAction = fileMenu.addAction(new QIcon(installDir + "data/ui/images/icon/disconnect.ico"), "Disconnect");
        disconnectAction.triggered.connect(Disconnect);
        client.Connected.connect(Connected);
        client.Disconnected.connect(Disconnected);
        disconnectAction.enabled = false;
    }
    fileMenu.addAction(new QIcon(installDir + "data/ui/images/icon/system-shutdown.ico"), "Quit").triggered.connect(Quit);

    // View menu
    var viewMenu = menu.addMenu("&View");

    if (framework.GetModuleByName("SceneStructure"))
    {
        viewMenu.addAction("Assets").triggered.connect(OpenAssetsWindow);
        viewMenu.addAction("Scene").triggered.connect(OpenSceneWindow);
    }

    var ecEditor = framework.GetModuleByName("ECEditor");
    if (ecEditor)
        viewMenu.addAction("EC Editor").triggered.connect(OpenEcEditorWindow);

    if (framework.GetModuleByName("DebugStats"))
        viewMenu.addAction("Profiler").triggered.connect(OpenProfilerWindow);

    if (framework.GetModuleByName("PythonScript"))
        viewMenu.addAction("Python Console").triggered.connect(OpenPythonConsole);

    // Settings menu
    if (framework.GetModuleByName("MumbleVoip") || framework.GetModuleByName("CAVEStereo") || ecEditor)
    {
        var settingsMenu = menu.addMenu("&Settings");
        // Set unique object name so that other scripts can query this menu.
        settingsMenu.objectName = "SettingsMenu";
        if (framework.GetModuleByName("MumbleVoip"))
            settingsMenu.addAction("Voice settings").triggered.connect(OpenVoiceSettings);
        if (framework.GetModuleByName("CAVEStereo"))
        {
            settingsMenu.addAction("Cave").triggered.connect(OpenCaveWindow);
            settingsMenu.addAction("Stereoscopy").triggered.connect(OpenStereoscopyWindow);
        }

        if (ecEditor)
        {
            // Gizmo
            var showGizmoAction = settingsMenu.addAction("Show editing gizmo");
            showGizmoAction.checkable = true;
            showGizmoAction.checked = ecEditor.gizmo;
            showGizmoAction.triggered.connect(ShowEditingGizmo);

            // Highlighting of selected entities
            var showHighlightAction = settingsMenu.addAction("Highlight selected entities");
            showHighlightAction.checkable = true;
            showHighlightAction.checked = ecEditor.highlighting;
            showHighlightAction.triggered.connect(HighlightSelectedEntities);
        }
    }

    // Help menu
    var helpMenu = menu.addMenu("&Help");
    helpMenu.addAction(new QIcon(installDir + "data/ui/images/icon/browser.ico"), "Wiki").triggered.connect(OpenWikiUrl);
    helpMenu.addAction(new QIcon(installDir + "data/ui/images/icon/browser.ico"), "Doxygen").triggered.connect(OpenDoxygenUrl);
    helpMenu.addAction(new QIcon(installDir + "data/ui/images/icon/browser.ico"), "Mailing list").triggered.connect(OpenMailingListUrl);

    function NewScene() {
        scene.RemoveAllEntities();
    }

    function Reconnect() {
        client.Reconnect();
    }

    function Disconnect() {
        client.Logout();
    }

    function Connected() {
        disconnectAction.enabled = true;
        screenshotAct.enabled = true;
    }

    function Disconnected() {
        disconnectAction.enabled = false;
        screenshotAct.enabled = false;
    }

    function Quit() {
        framework.Exit();
    }
    
    function TakeScreenshot() {
        var mainCamera = renderer.MainCameraComponent();
        var imgPath = mainCamera.SaveScreenshot();
        QDesktopServices.openUrl(new QUrl(imgPath));
    }

    function CheckForUpdates() {
        if (framework.GetModuleByName("UpdateModule"))
            framework.GetModuleByName("UpdateModule").RunUpdater("/checknow");
    }

    function OpenMailingListUrl() {
        QDesktopServices.openUrl(new QUrl("http://groups.google.com/group/realxtend/"));
    }
    
    function OpenWikiUrl() {
        QDesktopServices.openUrl(new QUrl("http://wiki.realxtend.org/"));
    }

    function OpenDoxygenUrl() {
        QDesktopServices.openUrl(new QUrl("http://www.realxtend.org/doxygen/"));
    }

    function OpenSceneWindow() {
        framework.GetModuleByName("SceneStructure").ToggleSceneStructureWindow();
    }

    function OpenAssetsWindow() {
        framework.GetModuleByName("SceneStructure").ToggleAssetsWindow();
    }

    function OpenProfilerWindow() {
        framework.GetModuleByName("DebugStats").ShowProfilingWindow();
    }

    function OpenTerrainEditor() {
        framework.GetModuleByName("Environment").ShowTerrainWeightEditor();
    }

    function OpenPostProcessWindow() {
        framework.GetModuleByName("Environment").ShowPostProcessWindow();
    }

    function OpenPythonConsole() {
        framework.GetModuleByName("PythonScript").ShowConsole();
    }

    function OpenVoiceSettings() {
        framework.GetModuleByName("MumbleVoip").ToggleSettingsWidget();
    }

    function OpenConsoleWindow() {
        framework.GetModuleByName("Console").ToggleConsole();
    }

    function OpenEcEditorWindow() {
        framework.GetModuleByName("ECEditor").ShowEditorWindow();
    }

    function ShowEditingGizmo(show) {
        framework.GetModuleByName("ECEditor").gizmo = show;
    }

    function HighlightSelectedEntities(show) {
        framework.GetModuleByName("ECEditor").highlighting = show;
    }

    function OpenStereoscopyWindow() {
        framework.GetModuleByName("CAVEStereo").ShowStereoscopyWindow();
    }

    function OpenCaveWindow() {
        framework.GetModuleByName("CAVEStereo").ShowCaveWindow();
    }
}
