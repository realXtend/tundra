/*
  SlideCircle
  - circling around slide thingies for your sliding pleasure

  TODO
  * Add a help text
  * Path finding so that we won't go through things
*/

engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");


// Some functions needed for QVector3D calculations. Hopefully we can
// get rid of these in future (or present)

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

function pad(number, length) {
   
    var string = '' + number;
    while (string.length < length) {
        string = '0' + string;
    }
   
    return string;
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

/* Goes in front of given entity*/
function viewScreen(screen) {
    var pos = getNormal(screen, infront);
    camera.placeable.position = pos;
    camera.placeable.LookAt(screen.placeable.position);
}

/* calculates to point to move and look */
function getPoints(from, to) {
    curveposition = 0;
    prev_r = 360;
    targets = [];
    targets.push(getNormal(from, close));
    targets.push(getNormal(from, close));
    targets.push(getNormal(from, far));
    targets.push(getNormal(to, far));
    targets.push(getNormal(to, close));
    targets.push(getNormal(to, infront));

    lookAtTargets = [];
    lookAtTargets.push(from.placeable.position);
    lookAtTargets.push(to.placeable.position);
    
}

function changeSlide(index) {
    var currentScreen = screens[index % screennumber];
    var newurl = slideinfo.GetAttribute("slide" + (index));
    var oldurl = currentScreen.EC_WebView.webviewUrl
    if (debug) {
	print("oldurl " + oldurl);
	print("newurl " + newurl);
    }
    if (oldurl != newurl) {
	if (debug) {
	    print("It changes");
	}
	currentScreen.EC_WebView.webviewUrl = newurl;
    }
    changes += 1;
}

function HandleGotoNext() {
    var slideinfo = me.GetComponentRaw("EC_DynamicComponent", "SlideCircleInfo");
    var currentIndex = slideinfo.GetAttribute("current");
    if (debug) {
	print("Current screen is");
	print(screens[currentIndex % screennumber]);
	print("Current index is " + currentIndex);
    }

    var newIndex = currentIndex + 1;
    if (newIndex > endIndex) {
	newIndex = 0;
    }
    if (debug) {
	print("New index is " + newIndex);
	print("Goto screen " + (newIndex % screennumber));
	print(screens[newIndex % screennumber]);
    }

    getPoints(screens[currentIndex % screennumber], screens[newIndex % screennumber]);
    slideinfo.SetAttribute("current", newIndex);

    changeSlide(newIndex);
}

function HandleGotoPrev() {
    var slideinfo = me.GetComponentRaw("EC_DynamicComponent", "SlideCircleInfo");
    var currentIndex = slideinfo.GetAttribute("current");
    if (debug) {
	print("Current screen is");
	print(screens[currentIndex % screennumber]);
	print("Current index is " + currentIndex);
    }
    var newIndex = currentIndex -1;
    if (newIndex < 0) {
	newIndex = endIndex;
    }
    if (debug) {
	print("New index is " + newIndex);
	print("Goto screen " + (newIndex % screennumber));
	print(screens[newIndex % screennumber]);
    }
    getPoints(screens[currentIndex % screennumber], screens[newIndex % screennumber]);
    slideinfo.SetAttribute("current", newIndex);

    changeSlide(newIndex);
}

function getBezier(t) {
    if (t <= 0) {
	return targets[0];
    }
    if (t >= 1) {
	targets.splice(0,4);
	return;
    }
    
    var p0 = targets[0];
    var p1 = targets[1];
    var p2 = targets[2];
    var p3 = targets[3];

    return vadd(vadd(vadd(smul(p0, Math.pow(1 - t, 3)), smul(p1, 3 * Math.pow(1 - t, 2) * t)), smul(p2, 3 * (1 - t) * Math.pow(t , 2))), smul(p3, Math.pow(t, 3)));
}


function animationUpdate(dt) {
    var settings = me.GetComponentRaw("EC_DynamicComponent", "SlideCircleSettings");

    var recording = settings.GetAttribute("recording");

    if (recording) {
	dt = 1 / 25;
	var filename = "frame" + pad(animationFrame, 6) + ".jpg";
	print(filename);
	renderer.TakeScreenshot("/home/therauli/temp/", filename);
	animationFrame += 1;
    }

    if (targets.length == 0) {
	if (recording) {
	    if (changes >= endIndex) {
		settings.SetAttribute("recording", false)
		max_ratio = 1.5;
		print("Video done");
		return;
	    }
	    HandleGotoNext();
	} else {
	    return;
	}
    }
    
    //backing from front and closing to front also, yes
    if (targets.length == 6 || targets.length == 1) {
	if (targets.length == 6) {
	    var target = targets[0];
	    curveposition = -dt / 3;
	} else {
	    var target = targets[0];
	}
	var pos = camera.placeable.position

	var dist = distance(pos, target);
	camera.placeable.LookAt(lookAtTargets[0]);
	// If we're there change targets for moving and looking.
	if (dist <= 0.1) {
	    targets.splice(0, 1);
	    lookAtTargets.splice(0, 1);
	    print("We're close");
	    return;
	}
	
	var direction = vsubb(target, pos);
	direction = normalize(direction);
	
	var newPos = vadd(pos, smul(direction, dt * 5));
	camera.placeable.position = newPos;
	return;
    }

    curveposition += dt / 3;
    
    camera.placeable.position = getBezier(curveposition);
    
    // Get current rotation
    var oldtransform = camera.placeable.transform;
    var currentRotation = oldtransform.rot;
    
    // get target rotation

    camera.placeable.LookAt(lookAtTargets[0]);
    var newtransform = camera.placeable.transform;
    var targetRotation = newtransform.rot;

    // put back to current position
    camera.placeable.transform = oldtransform;

    var a = Math.pow((currentRotation.x - targetRotation.x), 2);
    var b = Math.pow((currentRotation.y - targetRotation.y), 2);
    var c = Math.pow((currentRotation.z - targetRotation.z), 2);
    var r = Math.sqrt(a + b + c);

    // If we're close enough we won't turn
    if ((r <= tolerance) || (r >= 360 - tolerance)) {
	camera.placeable.LookAt(lookAtTargets[0]);
	print("look at stuff");
	return;
    }


    // Don't turn back
    if (r > prev_r + tolerance) {
	if (debug) {
	    print("won't turn back");
	}
	camera.placeable.LookAt(lookAtTargets[0]);
	prev_r = r;
	return;
    }
    prev_r = r;
    
    var drotx = targetRotation.x - currentRotation.x;
    var droty = targetRotation.y - currentRotation.y;
    var drotz = targetRotation.z - currentRotation.z;
    
    // Count magnitude
    var magnitude = Math.sqrt(Math.pow(drotx, 2) + Math.pow(droty, 2) + Math.pow(drotz, 2));

    var ratio = Math.max(Math.min(1 / curveposition * 3, max_ratio), 0);

    newtransform.rot.x = (currentRotation.x + drotx / magnitude * ratio) % 360;
    newtransform.rot.y = (currentRotation.y + droty / magnitude * ratio) % 360;
    newtransform.rot.z = (currentRotation.z + drotz / magnitude * ratio) % 360;
    
    camera.placeable.transform = newtransform;


}

function reset() {
    var entity = screens[currentIndex];
    viewScreen(entity);
    targets = [];
    lookAtTargets = [];
}

function updateSettings() {
    var settings = me.GetComponentRaw("EC_DynamicComponent", "SlideCircleSettings");
    infront = settings.GetAttribute('infront');
    close = settings.GetAttribute('close');
    far = settings.GetAttribute('far');
}


function StartRecord() {
    var settings = me.GetComponentRaw("EC_DynamicComponent", "SlideCircleSettings");
    settings.SetAttribute("recording", true)
    max_ratio = 2.5;
    HandleGotoNext()
}


// We look for anything that has a EC_WebView and circulate between them
var ents = scene.GetEntitiesWithComponentRaw("EC_DynamicComponent");
var screens = [];

for (i = 0; i < ents.length; i++) {
    var candidate = ents[i];
    if (candidate.EC_DynamicComponent.name == 'SlideScreenInfo') {
	screens.push(candidate);
    }
    
}

var debug = 1

if (debug) {
    print(screens);
    for (i = 0; i < screens.length; i++) {
	print(screens[i].EC_Name.name + ", id: " +screens[i].id);
    }
}

var slideinfo = me.GetComponentRaw("EC_DynamicComponent", "SlideCircleInfo");

//initialize screens
for (i = 0; i < screens.length; i++) {
    var url = slideinfo.GetAttribute("slide" + i);
    screens[i].EC_WebView.webviewUrl = url;
}

var currentIndex = slideinfo.GetAttribute("current");
var endIndex = slideinfo.GetAttribute("slidenumber") - 1; 
var screennumber = screens.length

var inputmapper = me.GetOrCreateComponentRaw("EC_InputMapper", 2, false);
var camera = scene.GetEntityByNameRaw("FreeLookCamera");

//Handy for debug
//var camera = scene.GetEntityByName("Monkey");

inputmapper.RegisterMapping('n', "GotoNext", 1);
inputmapper.RegisterMapping('p', "GotoPrev", 1);
inputmapper.RegisterMapping('r', "ResetShow", 1)
inputmapper.RegisterMapping('v', "StartRecord", 1);

// variables for viewpoint infront is the distance where you want to
// end up (and leave) close is the distance to which you back up still
// looking at the screen two points are counted for Bezier
// curves. These distances are read from the dynamic component

var settings = me.GetComponentRaw("EC_DynamicComponent", "SlideCircleSettings");
settings.AttributeChanged.connect(updateSettings);

var infront = settings.GetAttribute('infront');
var close = settings.GetAttribute('close');
var far = settings.GetAttribute('far');
var recording = settings.GetAttribute('record')

var tolerance = 3;
var max_ratio = 1.5;
var speed = 100

var prev_r = 360;
var curveposition;

var animationFrame = 0;
var changes = 0;

me.Action("GotoNext").Triggered.connect(HandleGotoNext);
me.Action("GotoPrev").Triggered.connect(HandleGotoPrev);
me.Action("ResetShow").Triggered.connect(reset);
me.Action("StartRecord").Triggered.connect(StartRecord);

var targets = [];

print('Slide circle started');


// Wait for FreeLookCamera to have a placeable

function initShow() {
    if (camera.placeable != undefined) {
	// go to first slide
	reset()
	frame.Updated.connect(animationUpdate);
	camera.ComponentAdded.disconnect(initShow);
	print('Show ready');
    }
}

camera.ComponentAdded.connect(initShow);
