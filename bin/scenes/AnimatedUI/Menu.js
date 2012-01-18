// !ref: Menu.ui
// !ref: image.png

engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

var menu = asset.GetAsset("Menu.ui").Instantiate(true, 0);
menu.move(50, 50);
menu.show();

findChild(menu, "noanim").pressed.connect(NoAnimPressed);
function NoAnimPressed()
{
   scene.GetEntityByName("Animation").script.scriptRef = new AssetReference("");
}

findChild(menu, "anim1").pressed.connect(Anim1Pressed);
function Anim1Pressed()
{
   scene.GetEntityByName("Animation").script.scriptRef = new AssetReference("anim1.js");
}

findChild(menu, "anim2").pressed.connect(Anim2Pressed);
function Anim2Pressed()
{
   scene.GetEntityByName("Animation").script.scriptRef = new AssetReference("anim2.js");
}

findChild(menu, "anim3").pressed.connect(Anim3Pressed);
function Anim3Pressed()
{
   scene.GetEntityByName("Animation").script.scriptRef = new AssetReference("anim3.js");
}

findChild(menu, "anim4").pressed.connect(Anim4Pressed);
function Anim4Pressed()
{
   scene.GetEntityByName("Animation").script.scriptRef = new AssetReference("anim4.js");
}

function OnScriptDestroyed()
{
  menu.deleteLater();
  
//  renderer.DeleteUiPlane("MyPlane");
}

frame.Updated.connect(OnFrameUpdate);
function OnFrameUpdate()
{
   menu.raise();
   var proxy = menu.graphicsProxyWidget();
//   proxy.setZValue(1000);
}

var uiplane = renderer.CreateUiPlane("MyPlane");
uiplane.SetTexture(asset.GetAsset("image.png"));
uiplane.Show();
uiplane.SetAlpha(1);
uiplane.SetZOrder(250);
uiplane.SetX(0, true, 0);
uiplane.SetY(0, true, 0);
uiplane.SetWidth(288, true);
uiplane.SetHeight(288, true);
