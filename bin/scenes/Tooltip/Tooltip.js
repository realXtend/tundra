print("Loading Tooltip script.");

var speed = 1.0; 
var origBA;
var origFA;
var bc;
var fc;
var comp = me.GetComponentRaw("EC_HoveringText");
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
 function CheckComponent(entity, component, type)
{
	if (component.TypeName == "EC_HoveringText")
		GetHoveringTextComponent();
}

function GetHoveringTextComponent()
{
	comp = me.GetComponentRaw("EC_HoveringText");
	var mode = me.hoveringtext.GetUpdateMode();
	mode.value = 2;
	me.hoveringtext.SetUpdateMode(mode);
		
	bc = comp.backgroundColorAttr;
	fc = comp.fontColorAttr;
	origBA = bc.a; 
	origFA = fc.a;
}

function Update(frametime)
{
	bc = comp.backgroundColorAttr;
	fc = comp.fontColorAttr;

	if(bMouseIn) 
	{
		bc.a += frametime * speed;
		fc.a += frametime * speed;
	}		
	else 
	{
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
	
	comp.backgroundColorAttr = bc;
	comp.fontColorAttr = fc;
}

 function MouseIn()
{
	bMouseIn = true;
}

 function MouseOut()
{
	bMouseIn = false;
}


function OnScriptDestroyed()
{	
	//Return original values
	bc.a = origBA;
	fc.a = origFA;
	comp.backgroundColorAttr = bc; 
	comp.fontColorAttr = fc;
}
