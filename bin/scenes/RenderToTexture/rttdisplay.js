var rtt = null; //to keep the ref to the comp. what happens when it is removed? clean js execption?

function init() {
    //this could be just a straight setup script with no funcs,
    //but is apparently executed too early -- e.g. the camera is not there yet.
    //so have to wrap in a func and make a trick to run with a delay

    //could also have a camera in the scene xml with this component,
    //but here instead am using the pre-existing default cam
    //to make the demo simple, i.e. can just move the cam to see rtt working
    var cam = scene.GetEntityByNameRaw("FreeLookCamera");
    cam.GetOrCreateComponentRaw("EC_RttTarget");
    rtt = cam.rtttarget;
    rtt.size_x = 800;
    rtt.size_y = 600;
    rtt.PrepareRtt();
    rtt.SetAutoUpdated(true);

    var matname = cam.rtttarget.targettexture + "_mat"; //XXX add mat name getter to EC_RttTarget
    me.mesh.SetMaterial(0, matname);

    print(me + " set to display rtt image via mat " + matname + " from " + cam);
}

function update(frametime) {
    var vis = renderer.IsEntityVisible(me.Id);
    if(rtt != null) {
        rtt.SetAutoUpdated(vis);
    }
}

frame.DelayedExecute(0.1).Triggered.connect(this, init); //XXX dirty hack
frame.Updated.connect(update);
