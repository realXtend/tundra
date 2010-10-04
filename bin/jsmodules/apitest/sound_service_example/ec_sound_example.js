print("Loading EC_Sound example script.");

//This script demonstrates how can we use EC_Sound with EC_Toucable components together. Script listens EC_Touchable's
//MousePressed signal and when that signal is emmitted script will find a EC_Sound component and triggers it to play wanted sound file.

//How to use:
//1. Create a new entity to the world scene.
//2. Add EC_Touchable, EC_Sound and EC_Script components into the entity.
//3. Set EC_Script name to ".\jsmodules\apitest\sound_service_example\ec_sound_example.js"

var touchable_comp = 0;
if (me.HasComponent("EC_Touchable"))
{
	touchable_comp = me.GetComponentRaw("EC_Touchable");
	AddConnections();
}
else
	scene.ComponentAdded.connect(OnComponentAdded);

function OnObjectClicked()
{
	var sound = me.GetComponentRaw("EC_Sound");
	sound.triggerSound = true;
    # Todo: OnChanged() is deprecated
	sound.OnChanged();
}

function OnComponentAdded(entity, component)
{
	if (me.Id != entity.Id && component.TypeName != "EC_Touchable")
		return;
		
	touchable_comp = component;
	AddConnections();
}

function AddConnections()
{
	print("Clicked.");
	touchable_comp.MousePressed.connect(OnObjectClicked);
}