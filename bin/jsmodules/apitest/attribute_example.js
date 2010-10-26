print("Loading attribute example script.");

// Get or create new dynamic attribute and store it's pointer.
var attr = me.dynamiccomponent.CreateAttribute("real", "Testi");
print(attr.name);
print(attr.typename);
print(attr.value);