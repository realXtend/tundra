// For conditions of distribution and use, see copyright notice in LICENSE

if (framework.IsHeadless())
{
    console.LogError("instructions.js: Cannot be run in headless mode!");
    return;
}

engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

var label = new QLabel();
label.indent = 10;

label.text = "This scene demonstrates the Ogre post-process effects available in Tundra by default.\n\n"
             + "See the associated PostProcessEffects.js for details.";
label.resize(660,100);
label.setStyleSheet("QLabel {background-color: transparent; font-size: 16px; }");

var proxy = new UiProxyWidget(label);
ui.AddProxyWidgetToScene(proxy);
proxy.x = 100;
proxy.y = 30;
proxy.windowFlags = 0;
proxy.visible = true;
