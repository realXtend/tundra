//var imgfilepath = "/tmp/tundrarender.jpg"
var qimg = null;

function saveimage(dt) {
    var cam = renderer.MainCameraComponent();
    if (cam) {
        //print("saving img to: " + imgfilepath + " -- from cam: " + cam);
        //cam.ToQImage(false).save(imgfilepath);
        print("rendering img to a qimage from cam: " + cam);
        qimg = cam.ToQImage(false);
    }
}

frame.Updated.connect(saveimage);
