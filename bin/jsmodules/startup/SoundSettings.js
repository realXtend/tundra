/**
    For conditions of distribution and use, see copyright notice in LICENSE

    Implements sound settings widget functionality. */

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
    }
}

// Renderer settings widget usable only in headful mode.
if (!framework.IsHeadless())
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");

    var configFile = "tundra";
    var configUiSection = "ui";
//    var configRenderingSection = "rendering";
    var configWinPos = "sound settings windows pos"
    var settingsWidget = null;

    // Add menu entry to Settings menu
    var settingsMenu = findChild(ui.MainWindow().menuBar(), "SettingsMenu");
    if (!settingsMenu)
    {
        settingsMenu = ui.MainWindow().menuBar().addMenu("&Settings");
        settingsMenu.objectName = "SettingsMenu";
    }
    var soundSettings = settingsMenu.addAction("Sound");
    soundSettings.triggered.connect(ShowSoundSettings);

    // Shows renderer settings or hides it if it's already visible.
    function ShowSoundSettings()
    {
        if (settingsWidget)
        {
            settingsWidget.visible = !settingsWidget.visible;
            return;
        }

        CreateSoundSettingsWindow();
        settingsWidget.visible = true;

        LoadWindowPositionFromSettings();
    }

    // Creates the widget, but does not show it.
    function CreateSoundSettingsWindow()
    {
        settingsWidget = new QWidget(ui.MainWindow());
        settingsWidget.setWindowFlags(Qt.Tool);
        settingsWidget.setLayout(new QVBoxLayout());
        var child = ui.LoadFromFile(application.installationDirectory + "data/ui/soundsettings.ui", false);
        child.setParent(settingsWidget);
        settingsWidget.layout().addWidget(child, 0, 0);

        settingsWidget.setWindowTitle("Sound Settings");

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

        var master = findChild(settingsWidget, "slider_master")
        master.setValue(audio.GetMasterGain()*100);
        master.valueChanged.connect(SetMasterVolume);

        var triggered = findChild(settingsWidget, "slider_triggered");
        triggered.setValue(audio.GetSoundMasterGain(0)*100);
        triggered.valueChanged.connect(SetTriggeredSoundVolume);

        var ambient = findChild(settingsWidget, "slider_ambient");
        ambient.setValue(audio.GetSoundMasterGain(1)*100);
        ambient.valueChanged.connect(SetAmbientSoundVolume);

        var voice = findChild(settingsWidget, "slider_voice");
        voice.setValue(audio.GetSoundMasterGain(2)*100);
        voice.valueChanged.connect(SetVoiceSoundVolume);
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

    // Sets master volume
    function SetMasterVolume(value)
    {
        audio.SetMasterGain(value/100);
    }

    // Sets volume for triggered sounds.
    function SetTriggeredSoundVolume(value)
    {
        audio.SetSoundMasterGain(0, value/100);
    }

    // Sets volume for ambient sounds.
    function SetAmbientSoundVolume(value)
    {
        audio.SetSoundMasterGain(1, value/100);
    }

    // Sets volume for voice sounds.
    function SetVoiceSoundVolume(value)
    {
        audio.SetSoundMasterGain(2, value/100);
    }
}
