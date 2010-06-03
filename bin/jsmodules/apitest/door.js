print("Hello I want to control a door!");

var openbut = new QPushButton;
var lockbut = new QPushButton;
openbut.text = "o init";
lockbut.text = "l init";
openbut.show()
lockbut.show()

print("1");

function readdata(comp) {
	var datastr = comp.GetAttribute(); 
	print(datastr);
	var data = eval('(' + datastr + ')');	
	return data;
}

print("2");

function onChanged() {
	print("door.js onChanged");
	var data = readdata(component);
	
	print("Opened: " + data.opened);
	openbut.text = (data.opened) ? "Close" : "Open";
	lockbut.text = (data.locked) ? "Unlock" : "Lock";
	openbut.enabled = !(data.locked && !data.opened)
}

print("3");

component.OnChanged.connect(onChanged);
onChanged(); //once to init

print("4");

function sync(comp, data) {
	var newjson = JSON.stringify(data);
	component.SetAttribute(newjson);
}

print("5");

function open() {
	print("door.js open button clicked!");
	var data = readdata(component);
	print("Opened: " + data.opened);
	if (data.opened || !data.locked) {
		data.opened = !data.opened;
		sync(component, data);
	}
	else {
		print("Can't open a locked door!");
	}
}

print("6");

function lock() {
	print("door.js lock button clicked!");
	var data = readdata(component);
	print("Locked: " + data.locked);
	data.locked = !data.locked;
	sync(component, data);
}	

print("7");

openbut.clicked.connect(open);
lockbut.clicked.connect(lock);

print("Happy end in door.js!");
