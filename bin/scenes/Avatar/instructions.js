var showUi = true;
if (server.IsRunning() && framework.IsHeadless())
    showUi = false;

if (showUi)
{
    engine.ImportExtension("qt.core");
    engine.ImportExtension("qt.gui");

    var label = new QLabel();
    label.indent = 10;
    label.text = "This scene implements an \"avatar application\". In this world, each client who connects\nto the scene, gets an avatar presence to control. The avatars' visual appearance is\nimplemented via the Avatar and AnimationController components. The logic of adding\navatars is implemented in avatarapplication.js, and the movement is controlled with\nan InputMapper component.\n\nYou can embed AvatarApplication into your scenes by copying all the entities\nand assets from this scene to yours.\n\nWASD = Move avatar\nF = Toggle fly mode\nSpace = Jump/Fly up\nC = Fly down\nQ = Wave gesture\nE = Toggle sit\nMouse Scroll and +/- = Zoom in/out";
    label.resize(660,350);
    label.setStyleSheet("QLabel {background-color: transparent; font-size: 16px; }");

    var proxy = new UiProxyWidget(label);
    ui.AddProxyWidgetToScene(proxy);
    proxy.x = 50
    proxy.y = 50;
    proxy.windowFlags = 0;
    proxy.visible = true;
}
