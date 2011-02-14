print "Hello I want to control a door!"

engine.ImportExtension "qt.core"
engine.ImportExtension "qt.gui"

openbut = new QPushButton()
lockbut = new QPushButton()
openbut.text = "o init"
lockbut.text = "l init"
openbut.show()
lockbut.show()

data = me.dynamiccomponent

animctrl = me.animationcontroller
animctrl.EnableAnimation "open", false
animctrl.SetAnimationTimePosition "open", 1

curpos = 0 #keep so that can interpolate animated change to the var
animctrl.SetAnimWeight "open", curpos
newpos = 0

onAttributeChanged = (args...) ->
  print "door.coffee onAttributeChanged", args
  opened = data.GetAttribute 'opened'
  locked = data.GetAttribute 'locked'

  openbut.text = if opened then "Close" else "Open"
  lockbut.text = if locked then "Unlock" else "Lock"
  openbut.enabled = not (locked and not opened) #is closed, but not locked. not locked&closed

  newpos = if opened then 0 else 1

animate = (dtime) ->
  if newpos != curpos
    diff = Math.abs newpos - curpos
    dir = if (newpos < curpos) then -1 else 1
    speed = 1
    curpos += dir * (Math.min diff, dtime * speed)
    animctrl.SetAnimWeight "open", curpos
    print curpos + " - " + diff

onAttributeChanged() #once at startup to init
data.OnAttributeChanged.connect onAttributeChanged
frame.Updated.connect animate

open = ->
  print "door.coffee open button clicked!"
  opened = data.GetAttribute 'opened'
  locked = data.GetAttribute 'locked'
  if opened or not locked
    opened = not opened
    data.SetAttribute 'opened', opened
    #me.OnChanged -- what was that nowadays again?
  else
    print "Can't open a locked door!"

lock = ->
  print "door.coffee lock button clicked!"
  locked = data.GetAttribute 'locked'
  locked = not locked
  print locked
  data.SetAttribute 'locked', locked
  #component.OnChanged();

openbut.clicked.connect open
lockbut.clicked.connect lock

hoverIn = ->
  print "hovering in to over door"
  print me.dynamiccomponent.GetAttribute 'opened'

hoverOut = ->
  print "hovering out from over door"

mousePress = ->
  open()

me.Action("MouseHoverIn").Triggered.connect hoverIn
me.Action("MouseHoverOut").Triggered.connect hoverOut

me.Action("MousePress").Triggered.connect mousePress