
print("Loading WaterPlane Volume Trigger script");

var rigidbody = me.GetComponentRaw("EC_RigidBody");
var volumetrigger = me.GetComponentRaw("EC_VolumeTrigger");
var entitiesInside = new Array();
if (server.IsRunning())
{
	if(rigidbody && volumetrigger)
	{	
		ConnectSignals();
	}	
	else
	{
		print("Missing needed Components, Check that you have RigidBody and VolumeTrigger");
		me.ComponentAdded.connect(CheckComponent);
	}
}

//Checking if needed components are added after EC_Script to Entity
 function CheckComponent(component, type)
{	
	if (component.TypeName == "EC_VolumeTrigger")
		volumetrigger = component;
	else if (component.TypeName == "EC_RigidBody")
		rigidbody = component;
		
	if (rigidbody && volumetrigger)
	{	
		ConnectSignals();
		me.ComponentAdded.disconnect(CheckComponent);
	}
}

function ConnectSignals()
{
	print("found needed Components");

	// Hook to physics update
	rigidbody.GetPhysicsWorld().Updated.connect(ServerUpdatePhysics);
	
	// Hook to volume trigger signals
	volumetrigger.EntityEnter.connect(EntityEnter);
	volumetrigger.EntityLeave.connect(EntityLeave);
}

function ServerUpdatePhysics(frametime)
{
	for (var i=0 ; i<volumetrigger.GetNumEntitiesInside() ; i++)
	{
		var entity = volumetrigger.GetEntityInside(i);
		
		var mag = volumetrigger.GetEntityInsidePercent(entity);
		var rb = entity.GetComponentRaw("EC_RigidBody");
		if (rb && mag > 0.1)
		{
			var impulseVec = new Vector3df();
			impulseVec.z = mag * 0.8;
			rb.ApplyImpulse(impulseVec);
		}
	}
}


function EntityEnter (entity)
{
	var rb = entity.GetComponentRaw("EC_RigidBody");
	rb.angularDamping = 0.25;
	rb.linearDamping = 0.15;
}

function EntityLeave (entity)
{
	var rb = entity.GetComponentRaw("EC_RigidBody");
	rb.angularDamping = 0;
	rb.linearDamping = 0;
}
