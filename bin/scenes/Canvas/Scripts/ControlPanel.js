/*  For conditions of distribution and use, see copyright notice in LICENSE

    ControlPanel.js - Shows widget loaded from .ui file in GraphicsViewCanvas */

// !ref: ControlPanel.ui
// !ref: BigBoom.wav

var uiWidget = null;

if (me.graphicsViewCanvas)
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");

    uiWidget = asset.GetAsset("controlpanel.ui").Instantiate(false, 0);
    var proxy = me.graphicsViewCanvas.GraphicsScene().addWidget(uiWidget);
    uiWidget.show();

    findChild(uiWidget, "pushButton").pressed.connect(function()
    {
        audio.PlaySound(asset.GetAsset("BigBoom.wav"));
    });
}
else
{
    console.LogError("GraphicsViewCanvas component not available!");
}

function OnScriptDestroyed()
{
    if (framework.IsExiting())
        return; // Application exiting, widget pointers are garbage.
    if (uiWidget)
    {
        uiWidget.deleteLater();
        uiWidget = null;
    }
}
