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
    var slide_index = dyn.GetAttribute("Current") + dir;

    if (dyn.GetAttribute(slide_index + "") == undefined) {
	slide_index = 0;
    } else if (dyn.GetAttribute(slide_index + "") == undefined) {
	//FIXME
	slide_index = 0;
    }

    dyn.SetAttribute("Current", slide_index);
}

function changeSlideTo(slide_index) {
    print("Changing slide!");
    var dyn = me.GetComponentRaw("EC_DynamicComponent", "Slidelist");
    dyn.SetAttribute("Current", slide_index);
}


function onSlideChanged(attribute, type) {
    print('I changes!!!!11!');
    var dyn = me.GetComponentRaw("EC_DynamicComponent", "Slidelist");
    var index = dyn.GetAttribute('Current');

    var canvassource = me.GetComponentRaw('EC_3DCanvasSource');
    canvassource.source = dyn.GetAttribute(index);
}

// start canvas
var canvas = me.GetComponentRaw('EC_3DCanvas');
frame.DelayedExecute(1).Triggered.connect(this, function () {
    canvas.Start();
});

me.Action("MousePress").Triggered.connect(nextSlide);

var dyn = me.GetComponentRaw("EC_DynamicComponent", "Slidelist");
dyn.OnAttributeChanged.connect(onSlideChanged);
