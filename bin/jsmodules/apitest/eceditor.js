print("Initializing ECEditor test script.");

eceditor.SelectionChanged.connect(SelectionChanged);

// Check if entity is already created.
var editEntity = scene.GetEntityRaw(5000);
if (!editEntity)
{
    editEntity = scene.CreateEntityRaw(5000);
    editEntity.GetOrCreateComponent("EC_Gizmo");
}

function SelectionChanged(compType, compName, attrType, attrName) 
{
    print("Component type:" + compType + " name:" + compName);
    print("Attribute type:" + attrType + " name:" + attrName);
    if (attrType == "transform")
        UpdateGizmo();
}

function UpdateGizmo()
{
    editEntity.gizmo.ClearEditableAttributes();
    var components = eceditor.GetSelectedComponents();
    for(var i = 0; i < components.length; ++i)
        if (components[i])
            editEntity.gizmo.AddEditableAttribute(components[i], "Transform");
    print("EC_Gizmo components updated.");
}