/**
    For conditions of distribution and use, see copyright notice in LICENSE

    SceneInstructions.js - Instructs user how to open a scene if Tundra was started without startup scene. */

var sceneInstructions = null;

function OnScriptDestroyed()
{
    if (framework.IsExiting())
        return;
    HideSceneInstructions();
}

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

    // If we started without scene, show scene loading instructions for the user.
    // Delay the check for couple of seconds so that we can also check for existence of a potential login UI.
    frame.DelayedExecute(3.0).Triggered.connect(function()
    {
        var startupScenes = framework.CommandLineParameters("--file");
        var mainCameraScene = framework.Scene().MainCameraScene();
        var sceneLoadFailed = (startupScenes.length >= 1 && mainCameraScene && SizeOf(mainCameraScene.entities) <= 1);
        if (!input.ItemAtCoords(ui.GraphicsScene().sceneRect.toRect().center()) || sceneLoadFailed)
        {
            ShowSceneInstructions(sceneLoadFailed, startupScenes);
            framework.Scene().SceneAdded.connect(HideSceneInstructions);
        }
    });

    function ShowSceneInstructions(sceneLoadFailed, startupScenes)
    {
        if (!sceneInstructions)
        {
            var label = new QLabel();
            label.indent = 10;
            if (sceneLoadFailed)
            {
                label.text = "Tundra has started succesfully, but the startup scene '" + startupScenes[0] + "' failed to load.\n" +
                    "Please check the filename.";
            }
            else
            {
                label.text = "Tundra has started succesfully, but you have no active scene currently.\n" +
                    "Startup scenes can be specified by using the --file command line parameter. Run Tundra --help for instructions. \n" +
                    "Or alternatively, use File -> Open Scene to load an existing scene or New Scene to create an empty scene.";
            }

            label.resize(800, 200);
            label.setStyleSheet("QLabel {color: white; background-color: transparent; font-size: 16px; }");

            sceneInstructions = new UiProxyWidget(label);
            ui.AddProxyWidgetToScene(sceneInstructions);
            sceneInstructions.x = 50
            sceneInstructions.y = 50;
            sceneInstructions.windowFlags = 0;
            sceneInstructions.focusPolicy = Qt.NoFocus;
        }
        sceneInstructions.visible = true;
    }

    function HideSceneInstructions()
    {
        if (sceneInstructions)
        {
            sceneInstructions.deleteLater();
            sceneInstructions = null;
        }
    }
}
