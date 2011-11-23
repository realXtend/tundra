print("Loading Tooltip script.");

var speed = 1.0; 
var origBA;
var origFA;
var bc;
var fc;
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
                
            bc = comp.backgroundColor;
            fc = comp.fontColor;
            origBA = bc.a; 
            origFA = fc.a;
        }
    }
}

function Update(frametime) {
    if (comp == null)
      return;
    
    bc = comp.backgroundColor;
    fc = comp.fontColor;

    if(bMouseIn) {
        bc.a += frametime * speed;
        fc.a += frametime * speed;
    }           
    else {
        bc.a -= frametime * speed;
        fc.a -= frametime * speed;
    }
        
    if (bc.a >=1.0)
        bc.a = 1.0;
        
    if(bc.a <=0.0)
        bc.a = 0.0;
                
    if (fc.a >=1.0)
        fc.a = 1.0;
        
    if(fc.a <=0.0)
        fc.a = 0.0;
        
    comp.backgroundColor = bc;
    comp.fontColor = fc;
}

function MouseIn() {
    bMouseIn = true;
    print("Tooltip: Mouse IN");
}

function MouseOut() {
    bMouseIn = false;
    print("Tooltip: Mouse OUT");
}

function OnScriptDestroyed() {  
  if (comp == null)
      return;
    
    //Return original values
    bc.a = origBA;
    fc.a = origFA;
    comp.backgroundColor = bc; 
    comp.fontColor = fc;
}
