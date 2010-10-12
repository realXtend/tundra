print("Hello I want to control a door!");

var openbut = new QPushButton;
var lockbut = new QPushButton;
openbut.text = "o init";
lockbut.text = "l init";
openbut.show()
lockbut.show()

print("1");

print(touchable);

function onChanged() {
	print("door.js onChanged");
	var opened = component.GetAttribute('opened');
	var locked = component.GetAttribute('locked');
	
	print("Opened: " + opened);
	openbut.text = opened ? "Close" : "Open";
	lockbut.text = locked ? "Unlock" : "Lock";
	openbut.enabled = !(locked && !opened)
}

print("2");

// Todo: OnChanged() is deprecated
component.OnChanged.connect(onChanged);
//onChanged(); //once to init

print("3");

function open() {
	print("door.js open button clicked!");
	var opened = component.GetAttribute('opened');
	var locked = component.GetAttribute('locked');
	print("Opened: " + opened);
	if (opened || !locked) {
		opened = !opened;
		component.SetAttribute('opened', opened);
		component.OnChanged();
	}
	else {
		print("Can't open a locked door!");
	}
}

print("4");

function lock() {
	print("door.js lock button clicked!");
	var opened = component.GetAttribute('opened');
	var locked = component.GetAttribute('locked');
	print("Locked: " + locked);
	locked = !locked;
	component.SetAttribute('locked', locked);
	component.OnChanged();
}	

print("5");

openbut.clicked.connect(open);
lockbut.clicked.connect(lock);

function hover() {
	print("Hovering in JS");
}

touchable.MouseHover.connect(hover);

print("Happy end in door.js!");
