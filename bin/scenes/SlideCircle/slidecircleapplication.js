/*
  SlideCircle

*/

engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

function viewScreen(screen) {
    camera.placeable.LookAt(screen.placeable.position);
}


function HandleGotoNext() {
    currentIndex++;
    if (currentIndex >= endIndex) {
	currentIndex = 0;
    }
    var screen = entities[currentIndex];
    viewScreen(screen);
}

function HandleGotoPrev() {
    currentIndex--;
    if (currentIndex < 0) {
	currentIndex = endIndex - 1;
    }
    var screen = entities[currentIndex];
    viewScreen(screen);
}

var entities = scene.GetEntitiesWithComponentRaw("EC_WebView");
var currentIndex = 0;
var endIndex = entities.length;

var inputmapper = me.GetOrCreateComponentRaw("EC_InputMapper", 2, false);
var camera = scene.GetEntityByNameRaw("FreeLookCamera");

inputmapper.RegisterMapping('n', "GotoNext", 1);
inputmapper.RegisterMapping('p', "GotoPrev", 1);

me.Action("GotoNext").Triggered.connect(HandleGotoNext);
me.Action("GotoPrev").Triggered.connect(HandleGotoPrev);

print('..');