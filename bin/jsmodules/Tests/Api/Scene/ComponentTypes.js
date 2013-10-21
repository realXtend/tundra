/** For conditions of distribution and use, see copyright notice in LICENSE

    ComponentTypes.js - Test script. */

function SizeOf(obj)
{
    var size = 0, key;
    for(key in obj)
        if (obj.hasOwnProperty(key))
            ++size;
    return size;
};

var sceneApi = framework.Scene();
var compTypeNames = sceneApi.ComponentTypes(); // Will contain type names without the "EC_"-prefix.
var compEcTypeNames = []; // Type names with the "EC_"-prefix.
var compTypeIds = {}; // Associative array (TypeName-TypeID).

console.LogInfo("**************************");

console.LogInfo("Available component types (" + compTypeNames.length + "):")
for(i in compTypeNames)
{
    var typeName = compTypeNames[i];
    if (typeName.search("EC_") != -1)
    {
        compEcTypeNames.push(typeName);
        typeName = typeName.slice(3);
    }
    else
    {
        compEcTypeNames.push("EC_" + typeName);
    }

    compTypeNames[i] = typeName;

    var typeId = sceneApi.ComponentTypeIdForTypeName(typeName);
    compTypeIds[typeName] = typeId;
    console.LogInfo(" *" + typeName + " (ID " + typeId + ")");
}

console.LogInfo("**************************");

console.LogInfo("Creating all components by type name as unparented with a null scene...")
for(i in compTypeNames)
{
    var typeName = compTypeNames[i];
    console.LogInfo("Creating " + typeName + " (" + compTypeIds[typeName] + ")");
    sceneApi.CreateComponentByName(null, typeName);
}

console.LogInfo("**************************");

console.LogInfo("Creating all components by type ID as unparented with a null scene...")
for(i in compTypeIds)
{
    var typeId = compTypeIds[i];
    console.LogInfo("Creating " + typeId + " (" + i + ")");
    sceneApi.CreateComponentById(null, typeId);
}

console.LogInfo("**************************");

var viewEnabled = false;
var authority = false;
console.LogInfo("Creating scene with viewEnabled false and authority false.");
// The scene might already exists if reloading script live.
var scene = sceneApi.SceneByName("ComponentTypes");
if (!scene)
    scene = sceneApi.CreateScene("ComponentTypes", viewEnabled, authority);

console.LogInfo("Creating all components by type name as unparented with a scene...")
for(i in compTypeNames)
{
    var typeName = compTypeNames[i];
    console.LogInfo("Creating " + typeName + " (" + compTypeIds[typeName] + ")");
    sceneApi.CreateComponentByName(scene, typeName);
}

console.LogInfo("**************************");

console.LogInfo("Creating all components by type ID as unparented with a scene...")
for(i in compTypeIds)
{
    var typeId = compTypeIds[i];
    console.LogInfo("Creating " + typeId + " (" + i + ")");
    sceneApi.CreateComponentById(scene, typeId);
}

console.LogInfo("**************************");

var entity = scene.CreateEntity();

console.LogInfo("Creating all components by type name to " + entity.ToString() + "...")
for(i in compTypeNames)
{
    var typeName = compTypeNames[i];
    console.LogInfo("Creating " + typeName + " (" + compTypeIds[typeName] + ")");
    entity.CreateComponent(typeName);
}

// Components can create other components.
/*
var numAddedComps = SizeOf(entity.components);
console.LogInfo(numAddedComps + " components created and added successfully to the entity.");
if (numAddedComps != compTypeNames)
    console.LogError("Failed to create and add some of the components.");
*/

console.LogInfo("**************************");

console.LogInfo("Removing all components by type name from " + entity.ToString() + "...")
for(i in compTypeNames)
{
    var typeName = compTypeNames[i];
    console.LogInfo("Removing " + typeName + " (" + compTypeIds[typeName] + ")");
    entity.RemoveComponent(typeName);
}

console.LogInfo("**************************");

console.LogInfo("Creating all components by type ID to " + entity.ToString() + "...")
for(i in compTypeIds)
{
    var typeId = compTypeIds[i];
    console.LogInfo("Creating " + typeId + " (" + i + ")");
    entity.CreateComponent(typeId);
}

console.LogInfo("**************************");

/*
console.LogInfo("Removing all components by type ID from a entity...")
for(i in compTypeIds)
{
    var typeId = compTypeIds[i];
    console.LogInfo("Removing " + typeId + " (" + i + ")");
    entity.RemoveComponent(typeId);
}

console.LogInfo("**************************");
*/
