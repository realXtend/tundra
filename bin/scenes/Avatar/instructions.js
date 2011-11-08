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
    label.text = "This scene implements an \"avatar application\". In this world, each client who connects\nto the scene, \
gets an avatar presence to control. The avatars' visual appearance is\nimplemented via the Avatar and AnimationController \
components. The logic of adding\navatars is implemented in avatarapplication.js, and the movement is controlled with\nan \
InputMapper component.\n\nYou can embed AvatarApplication into your scenes by copying all the entities\nand assets from \
this scene to yours.\n\nWASD \t\t= Move avatar\nF \t\t= Toggle fly mode\nSpace \t\t= Jump/Fly up\nC \t\t= Fly down\nQ \t\t= Wave gesture\nE \t\t= \
Toggle sit\nMouse Scroll and +/- \t= Zoom in/out";

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
