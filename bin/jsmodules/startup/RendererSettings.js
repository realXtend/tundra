/**
    For conditions of distribution and use, see copyright notice in LICENSE

    Implements renderer settings widget functionality.
    Also registers Ctrl+F shortcut for toggling fullscreen mode. */

// Saves widget position to config and destroys the widget.
function OnScriptDestroyed()
{
    if (!framework.IsHeadless())
    {
        if (settingsWidget)
        {
            SaveWindowPositionToSettings();
            settingsWidget.deleteLater();
            settingsWidget = null;
        }

        input.UnregisterInputContextRaw("RendererSettings");
    }
}

// Renderer settings widget usable only in headful mode.
if (!framework.IsHeadless())
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");

    var configFile = "tundra";
    var configUiSection = "ui";
    var configRenderingSection = "rendering";
    var configWinPos = "renderer settings windows pos"
    var configFpsTargetName = "fps target limit";
    var settingsWidget = null;

    // Add menu entry to Settings menu
    var settingsMenu = findChild(ui.MainWindow().menuBar(), "SettingsMenu");
    if (!settingsMenu)
    {
        settingsMenu = ui.MainWindow().menuBar().addMenu("&Settings");
        settingsMenu.objectName = "SettingsMenu";
    }
    var rendererSettings = settingsMenu.addAction("Renderer");
    rendererSettings.triggered.connect(ShowRendererSettings);

    // Register input context for listening fullscreen shortcut key
    // todo: Revert this when Mac OS X fullscreen issue is solved
    if (framework.application.platform != "mac")
    { 
        var inputContext = input.RegisterInputContextRaw("RendererSettings", 90);
        inputContext.KeyPressed.connect(HandleKeyPressed);
    }

    // Shows renderer settings or hides it if it's already visible.
    function ShowRendererSettings()
    {
        if (settingsWidget)
        {
            settingsWidget.visible = !settingsWidget.visible;
            return;
        }

        CreateRendererSettingsWindow();
        settingsWidget.visible = true;

        LoadWindowPositionFromSettings();
    }

    // Creates the widget, but does not show it.
    function CreateRendererSettingsWindow()
    {
        settingsWidget = new QWidget(ui.MainWindow());
        settingsWidget.setWindowFlags(Qt.Tool);
        settingsWidget.setLayout(new QVBoxLayout());
        var child = ui.LoadFromFile(application.installationDirectory + "data/ui/renderersettings.ui", false);
        child.setParent(settingsWidget);
        settingsWidget.layout().addWidget(child, 0, 0);

        settingsWidget.setWindowTitle("Renderer Settings");

        // TODO: ideally we would do the following, but for some reason destroyed is not emitted ever.
/*
        settingsWidget.setAttribute(Qt.WA_DeleteOnClose);
        // Connect to destroyed signal so that we can set the pointer to null when the window is closed and destroyed.
        settingsWidget.destroyed.connect(SetPointerToNull);
        function SetPointerToNull()
        {
            settingsWidget = null;
        }
*/
        var viewDistance = findChild(settingsWidget, "spinbox_viewdistance")
        viewDistance.setValue(renderer.viewDistance);
        viewDistance["valueChanged(double)"].connect(SetViewDistance);

        var textureBudget = findChild(settingsWidget, "spinbox_texturebudget")
        textureBudget.setValue(renderer.textureBudget);
        textureBudget["valueChanged(double)"].connect(SetTextureBudget);

        var fullscreen = findChild(settingsWidget, "fullscreen_toggle");
        fullscreen.setChecked(renderer.fullScreen);

        if (framework.application.platform == "mac")
            fullscreen.setEnabled(false);
        else
            fullscreen.toggled.connect(SetFullScreenMode);

        var shadowQuality = findChild(settingsWidget, "combo_shadows");
        shadowQuality.setCurrentIndex(renderer.shadowQuality);
        shadowQuality["currentIndexChanged(int)"].connect(SetShadowQuality);

        var textureQuality = findChild(settingsWidget, "combo_texture");
        textureQuality.setCurrentIndex(renderer.textureQuality);
        textureQuality["currentIndexChanged(int)"].connect(SetTextureQuality);

        var targetFps = findChild(settingsWidget, "targetFpsSpinBox")
        targetFps.setValue(application.targetFpsLimit);
        targetFps["valueChanged(double)"].connect(SetTargetFpsLimit);

        var targetFpsWhenInactive = findChild(settingsWidget, "targetFpsWhenInactiveBox");
        targetFpsWhenInactive.setValue(application.targetFpsLimitWhenInactive);
        targetFpsWhenInactive["valueChanged(double)"].connect(SetTargetFpsLimitWhenInactive);
    }

    // Loads window position from config.
    function LoadWindowPositionFromSettings()
    {
        if (settingsWidget && config.HasValue(configFile, configUiSection, configWinPos))
        {
            var pos = config.Get(configFile, configUiSection, configWinPos);
            ui.MainWindow().EnsurePositionWithinDesktop(settingsWidget, pos);
        }
    }

    // Saves window position to config.
    function SaveWindowPositionToSettings()
    {
        if (settingsWidget)
            config.Set(configFile, configUiSection, configWinPos, settingsWidget.pos);
    }

    // Looks for Ctrl+F and toggles fullscreen mode.
    function HandleKeyPressed(e)
    {
        if (e.HasCtrlModifier() && e.keyCode == Qt.Key_F)
        {
            renderer.fullScreen = !renderer.fullScreen;
            if (settingsWidget)
            {
                var fullscreenCheckBox = findChild(settingsWidget, "fullscreen_toggle");
                if (fullscreenCheckBox)
                    fullscreenCheckBox.setChecked(renderer.fullScreen);
            }
        }
    }

    // Sets new view distance.
    function SetViewDistance(value)
    {
        renderer.viewDistance = value;
    }
    
    // Sets new texture budget.
    function SetTextureBudget(value)
    {
        renderer.textureBudget = value;
    }

    // Sets fullscreen mode on/off.
    function SetFullScreenMode(value)
    {
        renderer.fullScreen = value;
    }

    // Sets shadow quality. The setting will take effect only after application restart.
    function SetShadowQuality(value)
    {
        if (value < 0 || value > 2)
            return;
        renderer.shadowQuality = value;
        findChild(settingsWidget, "messageLabel").setText("Setting will take effect after application restart.");
    }

    // Sets texture quality. The setting will take effect only after application restart.
    function SetTextureQuality(value)
    {
        if (value < 0 || value > 1)
            return;
        renderer.textureQuality = value;
        findChild(settingsWidget, "messageLabel").setText("Setting will take effect after application restart.");
    }

    // Sets the target FPS limit. 0 disables limitting.
    function SetTargetFpsLimit(value)
    {
        if (value < 0)
            return;
        application.targetFpsLimit = value;
        config.Set(configFile, configRenderingSection, configFpsTargetName, application.targetFpsLimit);
    }

    function SetTargetFpsLimitWhenInactive(value)
    {
        if (value < 0)
            return;

        application.targetFpsLimitWhenInactive = value;
    }
}
