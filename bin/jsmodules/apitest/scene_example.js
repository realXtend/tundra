print("Loading scenemanager test script.");

function OnComponentAdded(entity, component, type)
{
    if(component.TypeName !== "EC_Mesh")
        return;        

    print("EC_Mesh component has been added to scene renaming it to MeshEntity");
    component.Name = "MeshEntity";
    component.p_meshResourceId = "Mesh ref here";
    component.ComponentChanged(1);
    print("Component has been added to entity. Id:" + entity.Id);
}

function OnEntityCreated(entity, change)
{
    print("Entity created");
}

function OnInputMapAction()
{
    me.Exec("PlaySound");
}

scene.ComponentAdded.connect(OnComponentAdded);
scene.EntityCreated.connect(OnEntityCreated);
me.Action("Move").Triggered.connect(OnInputMapAction);