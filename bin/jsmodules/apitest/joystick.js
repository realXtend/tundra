//reading joystick data, as sent by pyppet.py currently
//for docs on the api see http://www.realxtend.org/doxygen/device_api_page.html

//NOTE: might be cool if this worked also beforehand - that if the reading part is inited later, it'd start working
//now doing a trick to get around load order probs when testing with --run
var js0;
function init() {
    if (!js0) {
        js0 = devices.GetDevice("joystick0"); //the naming technique in pyppet.py
        if (js0) {
	    print("found joystick 0.");
	    js0.PositionEvent.connect(joymove)
	}
    }
    //else disconnect this init thing from update
}
frame.Updated.connect(init);

function joymove(axis, val) {
    print("joystick move: " + axis + ": " + val.x + ", " + val.y); //the sender abuses vec3, doesn't have z actually
}

