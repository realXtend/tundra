var showUi = true;
if (server.IsRunning() && framework.IsHeadless())
    showUi = false;

if (showUi)
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");

    var label = new QLabel();
    label.objectName = "InfoLabel";
    label.setStyleSheet("QLabel#InfoLabel { padding: 10px; background-color: rgba(230,230,230,175); border: 1px solid black; font-size: 16px; }");
    label.text = "This scene shows how to do a selective glow postprocess\n pass on the scene by using Ogre materials. The scene contains\nan OgreCompositor component \
that tells the Ogre renderer to perform\nan additional 2D rendering pass from the main camera. To control that\nthe fish should emit a glow effect, the fish object \
has a material which\ncontains an extra technique that is applied when the glow pass is being rendered.";

    var proxy = ui.AddWidgetToScene(label);

    // Check if the browser ui is present (or anything else on top left corner)
    proxy.x = 20;
    if (ui.GraphicsView().GetVisibleItemAtCoords(20,40) == null)
        proxy.y = 40;
    else
        proxy.y = 100;
    proxy.windowFlags = 0;
    proxy.visible = true;
    proxy.focusPolicy = Qt.NoFocus;
}
