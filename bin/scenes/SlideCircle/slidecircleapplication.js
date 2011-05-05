/*
  SlideCircle

*/

engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

function vprint(v) {
    print(v.x + ", " + v.y + ", " + v.z);
}

function crossp(v1, v2) {
    var result = new Vector3df();

    result.x = v1.y * v2.z - v1.z * v2.y;
    result.y = v1.z * v2.x - v1.x * v2.z;
    result.z = v1.x * v2.y - v1.y * v2.x;

    return result;
}

function vadd(v1, v2) {
    var result = new Vector3df();

    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    result.z = v1.z + v2.z;

    return result;
}

function smul(v1, s) {
    var result = new Vector3df();
    
    result.x = v1.x * s;
    result.y = v1.y * s;
    result.z = v1.z * s;

    return result;
}

function conjg(quat, v) {
    var qvec = new Vector3df();
    qvec.x = quat.x();
    qvec.y = quat.y();
    qvec.z = quat.z();

    var uv = crossp(qvec, v);
    var uuv = crossp(qvec, uv);
    uv = smul(uv, 2.0 * quat.scalar());
    uuv = smul(uuv, 2.0);

    return vadd(vadd(v, uv), uuv);
}

function viewScreen(screen) {
    var placeable = screen.placeable;
    var q = placeable.orientation;

    var temp_position = new Vector3df();
    temp_position.x = placeable.position.x();
    temp_position.y = placeable.position.y();
    temp_position.z = placeable.position.z();

    var un = new Vector3df();

    un.y = 1;

    var v = conjg(q, un);
    var worldpos = vadd(temp_position, smul(v, 10));

    var p = camera.placeable.position;

    p.setX(worldpos.x);
    p.setY(worldpos.y);
    p.setZ(worldpos.z);

    camera.placeable.position = p;

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