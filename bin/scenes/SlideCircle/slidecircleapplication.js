/*
  SlideCircle

*/

engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

function distance(v1, v2) {
    var a = Math.pow((v1.x() - v2.x()), 2);
    var b = Math.pow((v1.y() - v2.y()), 2);
    var c = Math.pow((v1.z() - v2.z()), 2);
    return Math.sqrt(a + b + c);
}

function crossp(v1, v2) {
    var result = new QVector3D();

    result.setX(v1.y() * v2.z() - v1.z() * v2.y());
    result.setY(v1.z() * v2.x() - v1.x() * v2.z());
    result.setZ(v1.x() * v2.y() - v1.y() * v2.x());

    return result;
}

function vadd(v1, v2) {
    var result = new QVector3D();

    result.setX(v1.x() + v2.x());
    result.setY(v1.y() + v2.y());
    result.setZ(v1.z() + v2.z());

    return result;
}

function vsubb(v1, v2) {
    var result = new QVector3D();

    result.setX(v1.x() - v2.x());
    result.setY(v1.y() - v2.y());
    result.setZ(v1.z() - v2.z());

    return result;
}

function norm(v) {
    return Math.sqrt(Math.pow(v.x(), 2) + Math.pow(v.y(), 2) + Math.pow(v.z(), 2))
}

function normalize(v) {
    var result = new QVector3D();
    var l = norm(v);
    result.setX(v.x() / l);
    result.setY(v.y() / l);
    result.setZ(v.z() / l);

    return result;
}

function smul(v1, s) {
    var result = new QVector3D();
    
    result.setX(v1.x() * s);
    result.setY(v1.y() * s);
    result.setZ(v1.z() * s);

    return result;
}

function conjg(quat, v) {
    var qvec = quat.vector();

    var uv = crossp(qvec, v);
    var uuv = crossp(qvec, uv);
    uv = smul(uv, 2.0 * quat.scalar());
    uuv = smul(uuv, 2.0);

    return vadd(vadd(v, uv), uuv);
}

function getNormal(entity, distance) {
    /* returns the place in front of entity from distance */
    var placeable = entity.placeable;
    var q = placeable.orientation;

    var un = new QVector3D();

    un.setY(1);

    var v = conjg(q, un);
    var pos = vadd(placeable.position, smul(v, distance));

    return pos;
}

function viewScreen(screen) {

    var pos = getNormal(screen, 10);

    var p = camera.placeable.position;

    p.setX(pos.x());
    p.setY(pos.y());
    p.setZ(pos.z());

    camera.placeable.position = p;

    camera.placeable.LookAt(screen.placeable.position);
}

function getPoints(from, to) {
    targets = [];
    targets.push(getNormal(from, 15));
    targets.push(getNormal(to, 15));
    targets.push(getNormal(to, 10));
    
}

function HandleGotoNext() {
    newIndex = currentIndex + 1;
    if (newIndex >= endIndex) {
	newIndex = 0;
    }
    getPoints(entities[currentIndex], entities[newIndex]);
    currentIndex = newIndex;
}

function HandleGotoPrev() {
    newIndex = currentIndex - 1;
    if (newIndex < 0) {
	newIndex = endIndex - 1;
    }
    getPoints(entities[currentIndex], entities[newIndex]);
    currentIndex = newIndex;
}

function animationUpdate(dt) {

    if (targets.length == 0) {
	return;
    }
    var target = targets[0];
    var pos = camera.placeable.position;
    //print(pos);
    //print(target);
    var d = distance(pos, target);
 
    //print(d);

    if (d <= 0.1) {
	targets.splice(0, 1);
	return;
    }

    var velocity = vsubb(pos, target);
    velocity = normalize(velocity);

    var newPos = vadd(pos, smul(velocity, -dt * 5));

    pos.setX(newPos.x());
    pos.setY(newPos.y());
    pos.setZ(newPos.z());

    camera.placeable.position = pos;
    camera.placeable.LookAt(entities[currentIndex].placeable.position);

}

function reset() {
    viewScreen(entities[currentIndex]);
    targets = [];
}

function putCube(place) {
    cube.placeable.position = place;
}


var entities = scene.GetEntitiesWithComponentRaw("EC_WebView");
var currentIndex = 0;
var endIndex = entities.length;

var inputmapper = me.GetOrCreateComponentRaw("EC_InputMapper", 2, false);
var camera = scene.GetEntityByNameRaw("FreeLookCamera");
var cube = scene.GetEntityByName("Cubbe");

inputmapper.RegisterMapping('n', "GotoNext", 1);
inputmapper.RegisterMapping('p', "GotoPrev", 1);
inputmapper.RegisterMapping('r', "ResetShow", 1);

me.Action("GotoNext").Triggered.connect(HandleGotoNext);
me.Action("GotoPrev").Triggered.connect(HandleGotoPrev);
me.Action("ResetShow").Triggered.connect(reset);

var targets = [];
print(targets);

print('..');

frame.Updated.connect(animationUpdate);