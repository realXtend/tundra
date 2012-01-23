print("Loading Tooltip script.");

var speed = 1.0; 
var comp = me.hoveringtext;
var bMouseIn = false;

if(!comp)
    print("This Entity does not have HoveringText component");
else
    GetHoveringTextComponent();

frame.Updated.connect(Update);
scene.ComponentAdded.connect(CheckComponent);
me.Action("MouseHoverIn").Triggered.connect(MouseIn);
me.Action("MouseHoverOut").Triggered.connect(MouseOut);

//Checking if EC_Hoveringtext component has added after EC_Script to Entity
function CheckComponent(entity, component, type) {
    if (component.typeName == "EC_HoveringText")
        GetHoveringTextComponent();
}

function GetHoveringTextComponent() {
    if (comp == null) {
        comp = me.hoveringtext;
        if (comp != null) {
            var mode = comp.updateMode;
            mode.value = 2;
            comp.updateMode = mode;
        }
    }
}

function Update(frametime) {
    if (comp == null)
      return;
    
    comp.overlayAlpha = Math.max(0.0, Math.min(1.0, comp.overlayAlpha + frametime * speed * (bMouseIn ? 1 : -1)));
}

function MouseIn() {
    bMouseIn = true;
    print("Tooltip: Mouse IN");
}

function MouseOut() {
    bMouseIn = false;
    print("Tooltip: Mouse OUT");
}
