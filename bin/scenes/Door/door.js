(function() {
  var animate, animctrl, curpos, data, hoverIn, hoverOut, lock, lockbut, mousePress, mouseScroll, newpos, onAttributeChanged, open, openbut;
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
  animctrl = me.animationcontroller;
  animctrl.EnableAnimation("open", false);
  curpos = 0;
  newpos = 0;
  onAttributeChanged = function() {
    var args, locked, opened;
    args = 1 <= arguments.length ? __slice.call(arguments, 0) : [];
    print("door.coffee onAttributeChanged", args);
    opened = data.GetAttribute('opened');
    locked = data.GetAttribute('locked');
    openbut.text = opened ? "Close" : "Open";
    lockbut.text = locked ? "Unlock" : "Lock";
    openbut.enabled = !(locked && !opened);
    return newpos = opened ? 0 : 1;
  };
  animate = function(dtime) {
    var diff, dir, speed;
    if (newpos !== curpos) {
      diff = Math.abs(newpos - curpos);
      dir = newpos < curpos ? -1 : 1;
      speed = 1;
      curpos += dir * (Math.min(diff, dtime * speed));
      animctrl.SetAnimWeight("open", 1 - curpos);
      return print(curpos + " - " + diff);
    }
  };
  onAttributeChanged();
  data.AttributeChanged.connect(onAttributeChanged);
  frame.Updated.connect(animate);
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
  mousePress = function() {
    return open();
  };
  mouseScroll = function() {
    var args;
    args = 1 <= arguments.length ? __slice.call(arguments, 0) : [];
    return print("mouse scroll:", args);
  };
  me.Action("MouseHoverIn").Triggered.connect(hoverIn);
  me.Action("MouseHoverOut").Triggered.connect(hoverOut);
  me.Action("MousePress").Triggered.connect(mousePress);
  me.Action("MouseScroll").Triggered.connect(mouseScroll);
}).call(this);
