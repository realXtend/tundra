function ChangeEntityParent() {
    if (childEntity.parent == parentEntity)
        childEntity.parent = null;
    else
        childEntity.parent = parentEntity;
    frame.DelayedExecute(2.0).Triggered.connect(ChangeEntityParent);
}

if (!scene.EntityByName("Parent")) {

    var parentEntity = scene.CreateEntity(0, ["Placeable", "Mesh"]);
    parentEntity.name = "Parent";

    var trans = parentEntity.placeable.transform;
    trans.pos = new float3(0, 0, 10);
    parentEntity.placeable.transform = trans;

    var meshRef = parentEntity.mesh.meshRef;
    meshRef.ref = "cube.mesh";
    parentEntity.mesh.meshRef = meshRef;

    var childEntity = parentEntity.CreateChild(0, ["Placeable", "Mesh"]);
    childEntity.name = "Child";

    trans = childEntity.placeable.transform;
    trans.pos = new float3(0, 2, 0.5);
    childEntity.placeable.transform = trans;
    childEntity.mesh.meshRef = meshRef;

    frame.DelayedExecute(2.0).Triggered.connect(ChangeEntityParent);

}

