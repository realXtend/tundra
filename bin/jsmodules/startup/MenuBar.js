/**
    For conditions of distribution and use, see copyright notice in LICENSE

    Implements basic menu bar for accessing common Tundra functionality and UIs.
    NOTE: Clears any existing menus in the menu bar so make sure that this script
    is loaded first, or replace with another implementation*/

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

    fileMenu.addSeparator();
    fileMenu.addAction("Clear Asset Cache").triggered.connect(ClearAssetCache);
    fileMenu.addSeparator();

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
        viewMenu.addAction("Key Bindings").triggered.connect(OpenKeyBindingsWindow);
    }

    var ecEditor = framework.GetModuleByName("ECEditor");
    if (ecEditor)
        viewMenu.addAction("EC Editor").triggered.connect(OpenEcEditorWindow);

    // TODO: Avatar Editor menu action disabled for now, as it's not fully ready for end-users
    //    if (framework.GetModuleByName("Avatar"))
    //        viewMenu.addAction("Avatar Editor").triggered.connect(OpenAvatarEditorWindow);

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

        settingsMenu.addAction("Open config folder").triggered.connect(OpenConfigFolder);

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
            showGizmoAction.checked = ecEditor.gizmoEnabled;
            showGizmoAction.triggered.connect(ShowEditingGizmo);

            // Highlighting of selected entities
            var showHighlightAction = settingsMenu.addAction("Highlight selected entities");
            showHighlightAction.checkable = true;
            showHighlightAction.checked = ecEditor.highlightingEnabled;
            showHighlightAction.triggered.connect(HighlightSelectedEntities);
        }
    }

    // Help menu
    var helpMenu = menu.addMenu("&Help");
    var browserIcon = new QIcon(installDir + "data/ui/images/icon/browser.ico");
    helpMenu.addAction(browserIcon, "Wiki").triggered.connect(OpenWikiUrl);
    helpMenu.addAction(browserIcon, "Doxygen").triggered.connect(OpenDoxygenUrl);
    helpMenu.addAction(browserIcon, "Mailing list").triggered.connect(OpenMailingListUrl);

    function NewScene() {
        scene.RemoveAllEntities();
    }

    function Reconnect() {
        client.Reconnect();
    }

    function Disconnect() {
        client.Logout(client.getActiveScenename());
    }

    function Connected() {
        disconnectAction.enabled = true;
        screenshotAct.enabled = true;
    }

    function Disconnected() {
        var scene;
        var sceneNames;

        sceneNames = client.getSceneNames();
        print("\nScenes: " + sceneNames + "\n");
        if (sceneNames[0] == undefined)
        {
            disconnectAction.enabled = false;
            screenshotAct.enabled = false;
            return;
        }
        else
        {
            print("sceneNames[0] = " + sceneNames[0]);
            scene = framework.Scene().GetScene(sceneNames[0]);
        }
        cameraentity = scene.GetEntityByName("AvatarCamera");
        if (!cameraentity)
            cameraentity = scene.GetEntityByName("FreeLookCamera");
        var camera = cameraentity.camera;
        client.setActiveScenename(sceneNames[0]);
        camera.SetActive(camera);
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
        var cachePath = asset.GetAssetCache().CacheDirectory();
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
            asset.GetAssetCache().ClearAssetCache();

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

    function OpenKeyBindingsWindow() {
        framework.GetModuleByName("SceneStructure").ToggleKeyBindingsWindow();
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

    function OpenAvatarEditorWindow() {
        framework.GetModuleByName("Avatar").ToggleAvatarEditorWindow();
        if (client.IsConnected())
            framework.GetModuleByName("Avatar").EditAvatar("Avatar" + client.GetConnectionID())
    }

    function ShowEditingGizmo(show) {
        framework.GetModuleByName("ECEditor").gizmoEnabled = show;
    }

    function HighlightSelectedEntities(show) {
        framework.GetModuleByName("ECEditor").highlightingEnabled = show;
    }

    function OpenStereoscopyWindow() {
        framework.GetModuleByName("CAVEStereo").ShowStereoscopyWindow();
    }

    function OpenCaveWindow() {
        framework.GetModuleByName("CAVEStereo").ShowCaveWindow();
    }
    
    function OpenConfigFolder() {
        QDesktopServices.openUrl(new QUrl(config.GetConfigFolder()));
    }
}
