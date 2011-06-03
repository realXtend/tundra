
//Temporary Entity for Splash Particle
function Entity (position)
{
	this.EntityId = scene.NextFreeId();
	ParticleEntity = scene.CreateEntityRaw(this.EntityId, "", 2, false);
	ParticleEntity.GetOrCreateComponentRaw("EC_ParticleSystem");
	ParticleEntity.particlesystem.particleId = "file://splash.particle";
	ParticleEntity.GetOrCreateComponentRaw("EC_Placeable");
	var trans = ParticleEntity.placeable.transform;
	trans.pos.x = position.x;
	trans.pos.y = position.y;
	trans.pos.z = position.z;
	ParticleEntity.placeable.transform = trans;
	scene.EmitEntityCreated(ParticleEntity, 2);

	frame.DelayedExecute(2.0).Triggered.connect(this, this.DeleteEntity);
}

Entity.prototype.DeleteEntity = function()
{
	scene.DeleteEntityById(this.EntityId);
}


print("Loading WaterPlane Collision Script for enity");

var waterEntity = scene.GetEntityIdsWithComponent("EC_WaterPlane");
var Water = scene.GetEntityRaw(waterEntity[0]);
var inWater;

var CollisionSound = me.GetComponentRaw("EC_Sound", "Collision");
var RigidBody = me.GetComponentRaw("EC_RigidBody");

if(CollisionSound && RigidBody && Water)
{
	ConnectSignals();
}
else
{
	print("Missing needed Components, Check that you have RigidBody and Sound(Collision)");
	me.ComponentAdded.connect(CheckComponent);
}

//Checking if needed components are added after EC_Script to Entity
 function CheckComponent(component, type)
{
	if (component.typeName == "EC_Sound")
	{
		if (component.name == "Collision")
			CollisionSound = component;
	}

	else if (component.typeName == "RigidBody")
		RigidBody = true;

	if (CollisionSound && RigidBody && Water)
	{
		ConnectSignals();
		me.ComponentAdded.disconnect(CheckComponent);
	}
}

function ConnectSignals()
{
		print("found needed Components");
		var position = me.placeable.transform.pos;
		inWater = Water.waterplane.IsPointInsideWaterCube(position);
		me.rigidbody.PhysicsCollision.connect(Collision);
		me.placeable.AttributeChanged.connect(PositionChange);
}

function Collision (otherEntity, pos, normal, distance, impulse, newCollision)
{
	if (impulse > 3.5)
		PlayAudio("file://Crash.ogg");
}

function PositionChange (attribute, type)
{
	var position = me.placeable.transform.pos;
	position.z -= 2.0;

	if (inWater != Water.waterplane.IsPointInsideWaterCube(position))
	{
		inWater = !inWater;

		if (inWater)
		{
			//me.rigidbody.mass = 20.0;
			me.rigidbody.PhysicsCollision.disconnect(Collision);
			var splashEntity = new Entity(position);
			PlayAudio("file://Splash.ogg");
		}
		else
			me.rigidbody.PhysicsCollision.connect(Collision);
	}
}

function PlayAudio(audio_ref)
{
	audio.PlaySound(audio_ref);
}

//Play from EC_Sound
function Sound(soundref, volume)
{
	if(!volume)
		volume = 1.0;
	me.sound.SoundId = soundref;
	me.sound.soundGain = volume;
	me.Exec(1, "PlaySound");
}
