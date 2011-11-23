var rtt = null; //the current rtt target image used. what happens when it is removed? clean js execption?

var camerainfo = [ //source camera, and the button object used to switch to that
    ["FreeLookCamera", "button_freecam"],
    ["box_red_cam", "button_red"],
    ["box_blue_cam", "button_blue"],
    ["box_green_cam", "button_green"]
];

function init() {
    //UPDATE / TODO NOTE: check whether this is still the case in Tundra2:
    //this could be just a straight setup script with no funcs,
    //but is apparently executed too early -- e.g. the camera is not there yet.
    //so have to wrap in a func and make a trick to run with a delay

    /* create all RttTargets, i.e. the component for each cam that we want image from
       note: a simpler approach might be to use a single cam that moves,
       but am now testing if this works fine also to verify that RttTarget works for multiple.
       and this tech of having multiple cams and switching between them can be simpler.
       possibly nice that they all have own textures, so can use those also when that cam is not active.
    */
    var info, camname, butname;
    for (var i in camerainfo) {
        info = camerainfo[i];
        camname = info[0];
        butname = info[1];

        rtt = createRttTarget(camname); //leaves the last in the list as current
        bindButton(camname, butname);
    }

    setImageSource(scene.GetEntityByName(camname));    
}

function createRttTarget(camname) {
    //could also have a camera in the scene xml with this component,
    //but here instead am using the pre-existing default cam
    //to make the demo simple, i.e. can just move the cam to see rtt working

    //apparently EC_Camera does not persist, so we have to create them here XXX \todo
    //.. adding with the GUI editing was fun and worked otherwise
    var cam = scene.GetEntityByName(camname);
    var cam_ec = cam.GetOrCreateComponent("EC_Camera");

    cam.GetOrCreateComponent("EC_RttTarget");
    rtt = cam.rtttarget;
    rtt.textureName = camname + "_tex";
    rtt.size_x = 800;
    rtt.size_y = 600;
    rtt.PrepareRtt();
    rtt.SetAutoUpdated(true);

    return rtt;
}

function setImageSource(cam) {
    if (rtt != null) {
        rtt.SetAutoUpdated(false);
    }
    rtt = cam.rtttarget;
    rtt.SetAutoUpdated(true);

    var matname = rtt.textureName + "_mat"; //XXX add mat name getter to EC_RttTarget
    me.mesh.SetMaterial(0, matname);

    print(me + " set to display rtt image via mat " + matname); // + " from " + cam);
}

function bindButton(camname, butname) {
    var but = scene.GetEntityByName(butname);
    var cam = scene.GetEntityByName(camname);

    but.Action("MousePress").Triggered.connect(
      function() {
        setImageSource(cam);
      });
}

function displayEnterFreecam(ent) {
    if(rtt != null) {
        rtt.SetAutoUpdated(true);
        print("RenderToTexture update enabled");
    }
}

function displayLeaveFreecam(ent) {
    if(rtt != null) {
        rtt.SetAutoUpdated(false);
        print("RenderToTexture update disabled");
    }
}

//to track display entity visibility to disable rtt tex update
var freecam_ent = scene.GetEntityByName("FreeLookCamera");
var cam = freecam_ent.camera;
cam.StartViewTracking(me);
cam.EntityEnterView.connect(displayEnterFreecam);
cam.EntityLeaveView.connect(displayLeaveFreecam);

frame.DelayedExecute(0.1).Triggered.connect(this, init); //XXX dirty hack
