print("Loading EC_SoundListener example script.");

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
	var sound_listener = me.GetComponentRaw("EC_SoundListener");
	sound_listener.active = true;
	sound_listener.OnChanged();
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
	touchable_comp.MousePressed.connect(OnObjectClicked);
}