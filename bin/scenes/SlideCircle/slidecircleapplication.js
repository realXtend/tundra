/*
  SlideCircle

*/

engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

function HandleGotoNext() {
    current++;
    if (current >= end) {
	current = 0;
    }
    var screen = entities[current];
    camera.placeable.LookAt(screen.placeable.position);

}

var entities = scene.GetEntitiesWithComponentRaw("EC_WebView");
var current = 0;
var end = entities.length;

var inputmapper = me.GetOrCreateComponentRaw("EC_InputMapper", 2, false);
var camera = scene.GetEntityByNameRaw("FreeLookCamera");
print(camera);

inputmapper.RegisterMapping('n', "GotoNext", 1);

me.Action("GotoNext").Triggered.connect(HandleGotoNext);

print('..');