// For conditions of distribution and use, see copyright notice in LICENSE.md

// Implements renderer settings widget functionality.
// Also registers Ctrl+F shortcut for toggling fullscreen mode.
if (!framework.IsHeadless())
{
    var settingsWidget = null;

    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");

    var settingsMenu = findChild(ui.MainWindow().menuBar(), "SettingsMenu");
    if (!settingsMenu)
        settingsMenu = ui.MainWindow().menuBar().addMenu("&Settings");
    var rendererSettings = settingsMenu.addAction("Renderer");
    rendererSettings.triggered.connect(ShowRendererSettings);

    // Fullscreen shortcut key
    var inputContext = input.RegisterInputContextRaw("RendererSettings", 90);
    inputContext.KeyPressed.connect(HandleKeyPressed);

    // Shows renderer settings or hides it if it's already visible.
    function ShowRendererSettings()
    {
        if (settingsWidget && settingsWidget.visible)
        {
            settingsWidget.visible = false;
            return;
        }

        CreateRendererSettingsWindow();
        settingsWidget.visible = true;
    }

    function CreateRendererSettingsWindow()
    {
        settingsWidget = new QWidget(ui.MainWindow());
        settingsWidget.setAttribute(Qt.WA_DeleteOnClose);
        settingsWidget.setWindowFlags(Qt.Tool);
        settingsWidget.setLayout(new QVBoxLayout());
        var child = ui.LoadFromFile(application.installationDirectory + "data/ui/renderersettings.ui", false);
        child.setParent(settingsWidget);
        settingsWidget.layout().addWidget(child, 0, 0);

        settingsWidget.setWindowTitle("Renderer Settings");

        var viewDistance = findChild(settingsWidget, "spinbox_viewdistance")
        viewDistance.setValue(renderer.ViewDistance());
        viewDistance["valueChanged(double)"].connect(SetViewDistance);

        var fullscreen = findChild(settingsWidget, "fullscreen_toggle");
        fullscreen.setChecked(renderer.IsFullScreen());
        fullscreen.toggled.connect(SetFullScreenMode);

        var shadowQuality = findChild(settingsWidget, "combo_shadows");
        shadowQuality.setCurrentIndex(renderer.ShadowQuality());
        shadowQuality["currentIndexChanged(int)"].connect(SetShadowQuality);

        var textureQuality = findChild(settingsWidget, "combo_texture");
        textureQuality.setCurrentIndex(renderer.TextureQuality());
        textureQuality["currentIndexChanged(int)"].connect(SetTextureQuality);
    }

    function HandleKeyPressed(e)
    {
        if (e.HasCtrlModifier() && e.keyCode == Qt.Key_F)
        {
            renderer.SetFullScreen(!renderer.IsFullScreen());
            if (settingsWidget)
            {
                var fullscreen = findChild(settingsWidget, "fullscreen_toggle");
                if (fullscreen)
                    fullscreen.setChecked(renderer.IsFullScreen());
            }
        }
    }

    function SetViewDistance(value)
    {
        renderer.SetViewDistance(value);
    }

    function SetFullScreenMode(value)
    {
        renderer.SetFullScreen(value);
    }

    function SetShadowQuality(value)
    {
        if (value < 0 || value > 2)
            return;
        renderer.SetShadowQuality(value);
        findChild(settingsWidget, "label_restartmessage").setText("Setting will take effect after viewer restart.");
    }

    function SetTextureQuality(value)
    {
        if (value < 0 || value > 1)
            return;
        renderer.SetTextureQuality(value);
        findChild(settingsWidget, "label_restartmessage").setText("Setting will take effect after viewer restart.");
    }
}
