var script = "";
var controller = undefined;
var debug = false;

var dynamic = me.GetComponentRaw("EC_DynamicComponent");
	if(dynamic)
	{
		script = dynamic.GetAttribute("script");
		controller = dynamic.GetAttribute("controller");
		debug = dynamic.GetAttribute("debug");
	}

if (debug)
	debugger;	
	
if (script.length > 0 )
{
	var time1 = Date.now();
	engine.IncludeFile(script);
	var time2 = Date.now();
	
	if (!isNaN(controller)&& controller !=0)
		scene.GetEntityRaw(controller).Exec(1, "TestFinished", me.Id, time2 - time1);
	
}