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

function HandleGotoPrev() {
    current--;
    if (current < 0) {
	current = end - 1;
    }
    var screen = entities[current];
    camera.placeable.LookAt(screen.placeable.position);
}

var entities = scene.GetEntitiesWithComponentRaw("EC_WebView");
var current = 0;
var end = entities.length;

var inputmapper = me.GetOrCreateComponentRaw("EC_InputMapper", 2, false);
var camera = scene.GetEntityByNameRaw("FreeLookCamera");

inputmapper.RegisterMapping('n', "GotoNext", 1);
inputmapper.RegisterMapping('p', "GotoPrev", 1);

me.Action("GotoNext").Triggered.connect(HandleGotoNext);
me.Action("GotoPrev").Triggered.connect(HandleGotoPrev);

print('..');