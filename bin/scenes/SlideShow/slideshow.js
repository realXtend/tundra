engine.ImportExtension("qt.core");
engine.ImportExtension("qt.gui");

function nextSlide() {
    print("Next slide!");
    changeSlide(1);
}

function prevSlide() {
    print("Previous slide!");
    changeSlide(-1);
}


function changeSlide(dir) {
    print("Changing slide!");
    var dyn = me.GetComponentRaw("EC_DynamicComponent", "Slidelist");
    var slide_index = parseInt(dyn.GetAttribute("Current")) + dir;
    var max = dyn.GetAttribute("MaxIndex");

    if (slide_index < 0) {
	slide_index = max;
    } else if (slide_index > max) {
	slide_index = 0;
    }

    print('new slide index is ' + slide_index);
    dyn.SetAttribute("Current", slide_index + "");
}

function changeSlideTo(slide_index) {
    print("Changing slide!");
    var dyn = me.GetComponentRaw("EC_DynamicComponent", "Slidelist");
    dyn.SetAttribute("Current", slide_index + "");
}


function onSlideChanged(attribute, type) {
    print('I changes!!!!11!');
    var dyn = me.GetComponentRaw("EC_DynamicComponent", "Slidelist");
    var index = dyn.GetAttribute('Current');
    
    var canvassource = me.GetComponentRaw('EC_3DCanvasSource');
    var slide = dyn.GetAttribute(index + "");
    print('new url is ' + slide);
    canvassource.source = slide;
}


// start canvas

// By Ugly the Hack. 3D canvas component might not be rady when we get
// here so we wait for some time before we start it. Should prolably
// to connect some signal or something.
frame.DelayedExecute(1).Triggered.connect(this, function () {
	var canvas = me.GetComponentRaw('EC_3DCanvas');
	canvas.Start();
    });

// Maybe create the thumbnal view here
//me.Action("MousePress").Triggered.connect(nextSlide);

var dyn = me.GetComponentRaw("EC_DynamicComponent", "Slidelist");
dyn.AttributeChanged.connect(onSlideChanged);

var prev = scene.GetEntityByNameRaw('Button prev (' + me.name.name + ' ' + me.Id + ')');
prev.Action("MousePress").Triggered.connect(prevSlide);

var next = scene.GetEntityByNameRaw('Button next (' + me.name.name + ' ' + me.Id + ')');
next.Action("MousePress").Triggered.connect(nextSlide);