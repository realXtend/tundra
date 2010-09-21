print("Loading scenemanager test script.");

function OnComponentAdded(entity, component, type)
{
    if(component.TypeName !== "EC_Mesh")
        return;        

    print("EC_Mesh component has been added to scene renaming it to MeshEntity");
    component.Name = "MeshEntity";
    component.ComponentChanged(1)
    print("Component has been added to entity. Id:" + entity.Id);
}

function OnEntityCreated(entity, change)
{
    var component = entity.GetOrCreateComponentRaw("EC_Light");
    component.ComponentChanged(1);
}

scene.ComponentAdded.connect(OnComponentAdded);
scene.EntityCreated.connect(OnEntityCreated);

me.GetOrCreateComponentRaw("EC_Light");
me.ComponentChanged(1);