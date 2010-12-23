(function() {
  var data, hoverIn, hoverOut, lock, lockbut, onAttributeChanged, open, openbut;
  var __slice = Array.prototype.slice;
  print("Hello I want to control a door!");
  engine.ImportExtension("qt.core");
  engine.ImportExtension("qt.gui");
  openbut = new QPushButton();
  lockbut = new QPushButton();
  openbut.text = "o init";
  lockbut.text = "l init";
  openbut.show();
  lockbut.show();
  data = me.dynamiccomponent;
  onAttributeChanged = function() {
    var args, locked, opened;
    args = 1 <= arguments.length ? __slice.call(arguments, 0) : [];
    print("door.coffee onAttributeChanged", args);
    opened = data.GetAttribute('opened');
    locked = data.GetAttribute('locked');
    openbut.text = opened ? "Close" : "Open";
    lockbut.text = locked ? "Unlock" : "Lock";
    return openbut.enabled = !(locked && !opened);
  };
  onAttributeChanged();
  data.OnAttributeChanged.connect(onAttributeChanged);
  open = function() {
    var locked, opened;
    print("door.coffee open button clicked!");
    opened = data.GetAttribute('opened');
    locked = data.GetAttribute('locked');
    if (opened || !locked) {
      opened = !opened;
      return data.SetAttribute('opened', opened);
    } else {
      return print("Can't open a locked door!");
    }
  };
  lock = function() {
    var locked;
    print("door.coffee lock button clicked!");
    locked = data.GetAttribute('locked');
    locked = !locked;
    print(locked);
    return data.SetAttribute('locked', locked);
  };
  openbut.clicked.connect(open);
  lockbut.clicked.connect(lock);
  hoverIn = function() {
    print("hovering in to over door");
    return print(me.dynamiccomponent.GetAttribute('opened'));
  };
  hoverOut = function() {
    return print("hovering out from over door");
  };
  me.Action("MouseHoverIn").Triggered.connect(hoverIn);
  me.Action("MouseHoverOut").Triggered.connect(hoverOut);
}).call(this);
