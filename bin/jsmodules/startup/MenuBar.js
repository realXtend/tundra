/**
    For conditions of distribution and use, see copyright notice in LICENSE

    MenuBar.js - Implements basic menu bar for accessing common Tundra functionality and UIs.

    NOTE: Clears any existing menus in the menu bar so make sure that this script
    is loaded first, or replace with another implementation*/

// Widget for showing instructions if user starts Tundra without any scene.
var sceneInstructions = null;

function SizeOf(obj)
{
    var size = 0, key;
    for(key in obj)
        if (obj.hasOwnProperty(key))
            ++size;
    return size;
};

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
    fileMenu.addAction("New Scene...").triggered.connect(NewScene);
    fileMenu.addAction("Open Scene...").triggered.connect(OpenScene);
    fileMenu.addSeparator();

    var screenshotAct = fileMenu.addAction("Take Screenshot");
    screenshotAct.triggered.connect(TakeScreenshot);
    screenshotAct.enabled = false;

    fileMenu.addSeparator();
    fileMenu.addAction("Clear Asset Cache").triggered.connect(ClearAssetCache);
    fileMenu.addSeparator();

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

    // Tools menu
    var toolsMenu = menu.addMenu("&Tools");
    // Set unique object name so that other scripts can query this menu.
    toolsMenu.objectName = "ViewMenu";

    if (framework.ModuleByName("SceneStructure"))
    {
        toolsMenu.addAction("Assets").triggered.connect(OpenAssetsWindow);
        toolsMenu.addAction("Scene").triggered.connect(OpenSceneWindow);
        toolsMenu.addAction("Key Bindings").triggered.connect(OpenKeyBindingsWindow);
    }

    var ecEditor = framework.ModuleByName("ECEditor");
    if (ecEditor)
        toolsMenu.addAction("EC Editor").triggered.connect(OpenEcEditorWindow);

    // TODO: Avatar Editor menu action disabled for now, as it's not fully ready for end-users
//    if (framework.ModuleByName("Avatar"))
//        toolsMenu.addAction("Avatar Editor").triggered.connect(OpenAvatarEditorWindow);

    if (framework.ModuleByName("DebugStats"))
        toolsMenu.addAction("Profiler").triggered.connect(OpenProfilerWindow);

    if (console)
        toolsMenu.addAction("Show Console").triggered.connect(OpenConsoleWindow);
    // Settings menu
    if (framework.ModuleByName("MumbleVoip") || ecEditor)
    {
        var settingsMenu = menu.addMenu("&Settings");
        // Set unique object name so that other scripts can query this menu.
        settingsMenu.objectName = "SettingsMenu";

        settingsMenu.addAction("Open Config Folder").triggered.connect(OpenConfigFolder);

        if (framework.ModuleByName("MumbleVoip"))
            settingsMenu.addAction("Voice Settings").triggered.connect(OpenVoiceSettings);

        if (ecEditor)
        {
            // Gizmo
            var showGizmoAction = settingsMenu.addAction("Show Editing Gizmo");
            showGizmoAction.checkable = true;
            showGizmoAction.checked = ecEditor.gizmoEnabled;
            showGizmoAction.triggered.connect(ShowEditingGizmo);

            // Highlighting of selected entities
            var showHighlightAction = settingsMenu.addAction("Highlight Selected Entities");
            showHighlightAction.checkable = true;
            showHighlightAction.checked = ecEditor.highlightingEnabled;
            showHighlightAction.triggered.connect(HighlightSelectedEntities);
        }
        // TODO Lanuage change is broken, do not show the menu for now.
        //ui.InitLanguageMenu();
    }

    // Help menu
    var helpMenu = menu.addMenu("&Help");
    var browserIcon = new QIcon(installDir + "data/ui/images/icon/browser.ico");
    helpMenu.addAction(browserIcon, "realXtend Home Page").triggered.connect(OpenHomePageUrl);
    helpMenu.addAction(browserIcon, "Developer Mailing List").triggered.connect(OpenDevMailingListUrl);
    helpMenu.addAction(browserIcon, "User Mailing List").triggered.connect(OpenMailingListUrl);
    helpMenu.addAction(browserIcon, "GitHub").triggered.connect(OpenGitHubUrl);
    // TODO Doxygen documentation not hosted anywhere currently!
//    helpMenu.addAction(browserIcon, "Developer Documentation").triggered.connect(OpenDoxygenUrl);

    function NewScene()
    {
        if (framework.Scene().MainCameraScene() != null)
        {
            var result = QMessageBox.warning(ui.MainWindow(), "New Scene", "Making a new scene will discard any changes you made to the current scene. Do you want to continue?", QMessageBox.Yes, QMessageBox.No);
            if (result == QMessageBox.No)
                return;
        }

        var fileName = QFileDialog.getSaveFileName(ui.MainWindow(), "Save New Scene As", application.currentWorkingDirectory, "Tundra TXML file (*.txml)");
        if (fileName == "")
            return;

        var sceneNumber = Math.floor(Math.random() * 10000000 + 1);
        var newScene = framework.Scene().CreateScene("Scene" + sceneNumber, true, true);
        if (newScene == null)
            return;

        var storage = asset.DeserializeAssetStorageFromString(fileName, false); 
        if (!storage)
        {
            console.LogError("Failed to create asset storage for file " + fileName);
            return;
        }

        asset.SetDefaultAssetStorage(storage);

        // Give some sort of feedback (f.ex. make a skybox)
        var environment = newScene.CreateEntity(newScene.NextFreeId(), ["Name", "Sky"]);
        environment.name = "Environment";
        environment.sky.enabled = true;
    }

    function OpenScene() {
        var fileName = QFileDialog.getOpenFileName(ui.MainWindow(), "Open Scene", application.currentWorkingDirectory, "Tundra TXML file (*.txml)");
        if (fileName == "")
            return;

        if (framework.Scene().MainCameraScene() != null)
        {
            var result = QMessageBox.warning(ui.MainWindow(), "Open Scene", "Opening a new scene will discard any changes you made to the current scene. Do you want to continue?", QMessageBox.Yes, QMessageBox.No);
            if (result == QMessageBox.No)
                return;
        }

        var sceneNumber = Math.floor(Math.random() * 10000000 + 1);
        var openedScene = framework.Scene().CreateScene("Scene" + sceneNumber, true, true);
        if (openedScene == null)
            return;

        openedScene.LoadSceneXML(fileName, true, false, 0);

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

    function ClearAssetCache() {
        // Show some additional info: count and size of removed files
        var cachePath = asset.Cache().CacheDirectory();
        if (cachePath != "" && cachePath != null)
        {
            var count = 0;
            var size = 0;

            var cacheDir = new QDir(cachePath);
            var cacheFiles = cacheDir.entryInfoList();  // Using flags here would be nice but does not seem to work
            for(var i=0; i<cacheFiles.length; ++i)
            {
                try
                {
                    if (cacheFiles[i].isFile() && cacheFiles[i].exists())
                    {
                        count++;
                        size += cacheFiles[i].size();
                    }
                }
                catch(e)
                {
                    console.LogError("Failed to iterate files in cache: " + e);
                    count = null;
                    size = null;
                    break;
                }
            }

            // Clear the cache. If we happen to throw here we wont show false log/ui information.
            asset.Cache().ClearAssetCache();

            // Log to console
            var msg = "";
            if (count != null && size != null)
            {
                // Round to two decimals
                var roundedSize = Math.round(((size/1024)/1024) * Math.pow(10,2)) / Math.pow(10,2);
                msg = "Cleared " + count + " files with total size of " + roundedSize + " mb from asset cache."
            }
            else
                msg = "Errors occurred while calculating cache file information. Cache cleared.";
            console.LogInfo(msg);

            // Show information box for user. Note: this will block so not the most convenient.
            if (ui.MainWindow() != null)
                QMessageBox.information(ui.MainWindow(), "Cache Cleared", msg);
        }
    }

    function OpenDevMailingListUrl()
    {
        QDesktopServices.openUrl(new QUrl("http://groups.google.com/group/realxtend-dev/"));
    }

    function OpenMailingListUrl()
    {
        QDesktopServices.openUrl(new QUrl("http://groups.google.com/group/realxtend/"));
    }
    
    function OpenHomePageUrl() {
        QDesktopServices.openUrl(new QUrl("http://realxtend.org/"));
    }

    function OpenGitHubUrl() {
        QDesktopServices.openUrl(new QUrl("https://github.com/realxtend/naali/"));
    }

    function OpenDoxygenUrl()
    {
        // TODO host doxygen-generated documentation somewhere!
        QDesktopServices.openUrl(new QUrl("http://www.realxtend.org/doxygen/"));
    }

    function OpenSceneWindow() {
        framework.ModuleByName("SceneStructure").ToggleSceneStructureWindow();
    }

    function OpenAssetsWindow() {
        framework.ModuleByName("SceneStructure").ToggleAssetsWindow();
    }

    function OpenKeyBindingsWindow() {
        framework.ModuleByName("SceneStructure").ToggleKeyBindingsWindow();
    }

    function OpenProfilerWindow() {
        framework.ModuleByName("DebugStats").ShowProfilerWindow();
    }

    function OpenVoiceSettings() {
        framework.ModuleByName("MumbleVoip").ToggleSettingsWidget();
    }

    function OpenConsoleWindow() {
        console.ToggleConsole();
    }

    function OpenEcEditorWindow() {
        framework.ModuleByName("ECEditor").ShowEditorWindow();
    }

    function OpenAvatarEditorWindow() {
        framework.ModuleByName("Avatar").ToggleAvatarEditorWindow();
        if (client.IsConnected())
           framework.ModuleByName("Avatar").EditAvatar("Avatar" + client.connectionId)
   }

    function ShowEditingGizmo(show) {
        framework.ModuleByName("ECEditor").gizmoEnabled = show;
    }

    function HighlightSelectedEntities(show) {
        framework.ModuleByName("ECEditor").highlightingEnabled = show;
    }

    function OpenConfigFolder() {
        QDesktopServices.openUrl(new QUrl(config.ConfigFolder()));
    }
}
