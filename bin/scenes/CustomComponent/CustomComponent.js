// Register component type if not registered yet
// We need to use a DynamicComponent as a blueprint, as ComponentDesc isn't available in script
if (!framework.Scene().IsComponentTypeRegistered("MyComponent"))
{
    print("Registering component type using DynamicComponent as a blueprint");
    var dc = me.CreateComponent("DynamicComponent");
    dc.CreateAttribute("string", "myString");
    dc.CreateAttribute("int", "myInteger");
    dc.CreateAttribute("bool", "myBool1");
    dc.CreateAttribute("bool", "myBool2");
    framework.Scene().RegisterComponentType("MyComponent", dc);
    // The dynamic component is unnecessary now and can be removed
    me.RemoveComponent("DynamicComponent");
}

print("Creating custom component");
var comp = me.GetOrCreateComponent("MyComponent");
print("Setting attribute values");
comp.myBool1 = true;
comp.myInteger = 1000;
comp.myString = "Testing, one two!";
