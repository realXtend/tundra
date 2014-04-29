// For conditions of distribution and use, see copyright notice in LICENSE

var attrTypes   = framework.Scene().attributeTypes;
var scene       = framework.Scene().CreateScene("DynamiComponentTestScene", false, true);
var me          = scene.CreateEntity();
me.name         = "DynamicComponent.js Test Suite";
var dyn         = me.CreateComponent("DynamicComponent");
var attrId      = undefined;

// If the QObject property should be used to set the attribute value.
// Note that in both cases the value will be asserted against both
// EC_DynamicComponent.Attribute and the variable!
var usePropertySetter = false;

function run()
{
    log.info("Running DynamicComponent.js\n");

    log.info("**************************************")
    log.info("Using EC_DynamicComponent.SetAttribute")
    log.info("**************************************\n")
    usePropertySetter = false;

    for (var i = 0; i < attrTypes.length; i++)
    {
        var attr = dyn.CreateAttribute(attrTypes[i], attrTypes[i] + " test");
        if (attr !== undefined && attr !== null)
        {
            dumpAttribute(attr);
            setAttribute(attr);
        }
        else
            log.error("Failed to create attribute of type '" + attrTypes[i] + "'");
    }

    log.info("***************************************")
    log.info("Using EC_DynamicComponent.attributeId")
    log.info("***************************************\n")
    usePropertySetter = true;

    for (var i = 0; i < attrTypes.length; i++)
    {
        var attr = dyn.CreateAttribute(attrTypes[i], attrTypes[i] + " test");
        if (attr !== undefined && attr !== null)
        {
            dumpAttribute(attr);
            setAttribute(attr);
        }
        else
            log.error("Failed to create attribute of type '" + attrTypes[i] + "'");
    }

    log.info("*************************************************")
    log.info("Validating attribute id to QObject property naming")
    log.info("*************************************************\n")

    // Test invalid attribute IDs that cant be made as dynamic property
    var propertyNameTests = [
        { id : "My String ****", propName : "myString****", expected : undefined },
        { id : "1234 my string", propName : "1234MyString", expected : undefined },
        { id : "ÖÄÅ",            propName : "öÄÅ",          expected : undefined },
        { id : "My 123 Var 456", propName : "my123Var456",  expected : "test value" },
        { id : "my_var",         propName : "my_var",       expected : "test value" },
        { id : "_my var",        propName : "_myVar",       expected : "test value" }
    ];
    for (var i = 0; i < propertyNameTests.length; i++)
    {
        var test = propertyNameTests[i];
        var attr = dyn.CreateAttribute("string", test.id);
        dyn.SetAttribute(test.id, "test value");
        if (dyn[test.propName] === test.expected && test.expected === undefined)
            log.info('Validated that "' + test.id + '" WON\'T translate into dynamicComponent.' + test.propName + ' due to invalid characters');
        else if (dyn[test.propName] === test.expected && test.expected !== undefined)
            log.info('Validated that "' + test.id + '" WILL translate into dynamicComponent.' + test.propName);
    }

    log.info("");
    log.info("**************************************************************");
    log.info("Testing creating attribute with an invalid attribute type name");
    log.info("**************************************************************");
    log.info("");
    var attr = dyn.CreateAttribute("foo", "test");
    if (attr === null)
        log.info("EC_DynamicComponent::CreateAttribute('foo') returned null as expected.");
    else
        log.error("EC_DynamicComponent::CreateAttribute('foo') did not return null as expected.");
    log.info("");
    var attr = framework.Scene().CreateAttribute("foo", "test");
    if (attr === null)
        log.info("SceneAPI::CreateAttribute('foo') returned null as expected.");
    else
        log.error("SceneAPI::CreateAttribute('foo') did not return null as expected.");
}

function setAttribute(attr)
{
    attrId = attr.id;

    /// @note Real number test are done with single decimal precision.
    /// More decimals will trigger a assert failure due to floating point comparisons.

    if (attr.typeName === "string")
    {
        setAttributeValue("this is a test - usePropertySetter = " + usePropertySetter);
        setAttributeValue("");
        setAttributeValue("meh - usePropertySetter = " + usePropertySetter);
    }
    else if (attr.typeName === "int")
    {
        setAttributeValue(10);
        setAttributeValue(-10);
    }
    else if (attr.typeName === "real")
    {
        setAttributeValue(1);
        setAttributeValue(2.5);
    }
    else if (attr.typeName === "bool")
    {
        setAttributeValue(true);
        setAttributeValue(false);
    }
    else if (attr.typeName === "Color")
    {
        setAttributeValue(new Color(1.2, 3.4, 4.5, 1.0), ["r", "g", "b", "a"]);
        setAttributeValue({ r : 1, g : 2, b : 3, a : 4 }, ["r", "g", "b", "a"]); // float issues when using decimals!
    }
    else if (attr.typeName === "float2")
    {
        setAttributeValue(new float2(1.2, 3.4), ["x", "y"]);
    }
    else if (attr.typeName === "float3")
    {
        setAttributeValue(new float3(1.2, 3.4, 4.5), ["x", "y", "z"]);
    }
    else if (attr.typeName === "float4")
    {
        setAttributeValue(new float4(1.2, 3.4, 4.5, 6.7), ["x", "y", "z", "w"]);
    }
    else if (attr.typeName === "Quat")
    {
        setAttributeValue(new Quat(1.2, 3.4, 4.5, 6.7), ["x", "y", "z", "w"]);
    }
    else if (attr.typeName === "AssetReference")
    {
        /// @todo These test will fail when the asset type is changed.
        /// This is due to Attribute<AssetReference>::FromQVariant using
        /// Attribute::Set() and the assignment operator for AssetReference
        /// ignores type if the type is already set. This needs to be resolved somehow in core
        /// or documented better that the type can never be changed from anywhere once set!

        setAttributeValue("test1.mesh", "ref");
        setAttributeValue({ ref: "test2.mesh" }, "ref");
        setAttributeValue({ ref: "test3.mesh", type : "Binary" }, [ "ref", "type" ]);
        setAttributeValue(new AssetReference("test4.mesh", ""), [ "ref", "type" ]);
        setAttributeValue(new AssetReference("test5.mesh", "OgreMaterial"), [ "ref", "type" ]);
    }
    else if (attr.typeName === "AssetReferenceList")
    {
        setAttributeValue(["test1.mesh", "", "test2.mesh"]);
        setAttributeValue([{ ref : "test1.material", type : "OgreMaterial" }, "test2.mesh", 
                          new AssetReference("test3.material", "OgreMaterial"), "test4.mesh"]);

        var refs = new AssetReferenceList();
        refs[0] = { ref : "test1.material", type : "OgreMaterial" };
        refs[1] = "test2.mesh";
        refs[2] = new AssetReference("test3.material", "OgreMaterial");
        setAttributeValue(refs);

        setAttributeValue(new AssetReferenceList(["1", { ref: "2" } , new AssetReference("3")]));
    }
    else if (attr.typeName === "EntityReference")
    {
        setAttributeValue("TestEntity1", "ref");
        setAttributeValue(100, "ref");
        setAttributeValue({ ref: "TestEntity2" }, "ref");
        setAttributeValue({ ref: 150 }, "ref");
        setAttributeValue(new EntityReference("TestEntity3"), "ref");
        setAttributeValue(new EntityReference(200), "ref");

        var other = scene.CreateEntity(scene.NextFreeId());
        other.name = "Parent1";
        setAttributeValue(other);
        if (assertAttribute(current("ref"), other.id.toString()))
            log.info("    -- OK " + current());

        setAttributeValue(null, "ref");

        other.name = "Parent2";
        setAttributeValue(new EntityReference(other));
        if (assertAttribute(current("ref"), other.id.toString()))
            log.info("    -- OK " + current());

        setAttributeValue({ "ref": undefined }, "ref");

        scene.RemoveEntity(other.id);
    }

    print("");
}

function setAttributeValue(value, assertProperty)
{
    if (!usePropertySetter)
        dyn.SetAttribute(attrId, value);
    else
    {
        var dynPropName = dynamicPropertyName();
        dyn[dynPropName] = value;
    }

    var ok = false;

    if (value === null || value === undefined || 
        (typeof assertProperty === "string" && value.hasOwnProperty(assertProperty) && value[assertProperty] === null) ||
        (typeof assertProperty === "string" && value.hasOwnProperty(assertProperty) && value[assertProperty] === undefined))
    {
        if (typeof current(assertProperty) === "string")
            ok = assertAttribute(current(assertProperty), "", assertProperty, true);
    }
    else if (typeof value === "string" || typeof value === "number" || typeof value === "boolean")
    {
        // If the current value is a string and the set value is a number. Auto convert for assert.
        // This fixes the tests failing when setting EntityReference with a number.
        if (typeof value === "number" && typeof current(assertProperty) === "string")
            ok = assertAttribute(current(assertProperty), value.toString(), assertProperty, true);
        else
            ok = assertAttribute(current(assertProperty), value, assertProperty, true);
    }
    else if (Array.isArray(value) && Array.isArray(current()))
    {
        var arrayCurrent = current();
        for (var i = 0; i < value.length; i++)
        {
            // Lets handle comparing complex objects like AssetReference vs. plain strings in Arrays.
            // We might set a mixture of string, AssetReference and Object. But the returned ref list.
            // will only contain the ref strings.
            if (typeof value[i] === "object" && value[i].hasOwnProperty("ref"))
                ok = assertAttribute(value[i].ref, arrayCurrent[i]);
            else
                ok = assertAttribute(value[i], arrayCurrent[i]);
            if (!ok) break;
        }
    }
    else if (typeof value === "object" && typeof assertProperty === "string")
    {
        // If the current value is a string and the set value is a number. Auto convert for assert.
        // This fixes the tests failing when setting EntityReference with a number.
        if (typeof value[assertProperty] === "number" && typeof current(assertProperty) === "string")
            ok = assertAttribute(current(assertProperty), value[assertProperty].toString());
        else
            ok = assertAttribute(current(assertProperty), value[assertProperty]);
    }
    else if (typeof value === "object" && Array.isArray(assertProperty))
    {
        for (var i = 0; i < assertProperty.length; i++)
        {
            var subProperty = assertProperty[i];
            if (typeof subProperty === "string")
            {
                ok = assertAttribute(current(subProperty), value[subProperty], subProperty, true);
                if (!ok) break;
            }
        }
    }

    if (ok)
    {
        if (typeof current() === "string")
            log.info('    -- OK "' + current() + '"');
        else
            log.info("    -- OK " + current());
    }
}

function current(subProperty)
{
    var value = dyn.Attribute(attrId);
    if (subProperty !== undefined)
        return value[subProperty];
    return value;
}

function dumpAttribute(attr)
{
    var value = dyn.Attribute(attr.id);
    log.info("Attribute (" + attr.id + ")");
    log.info("  Type       : " + attr.typeName);
    log.info("  JS typeof  : " + typeof value + (Array.isArray(value) ? " (Array)" : ""));
    log.info("  Value      : " + value);

    if (attr.owner === undefined || attr.owner === null)
        log.error("  Owner component is invalid: " + attr.owner);
}

function assertAttribute(value, assertValue, assertProperty, testDynamicProp)
{
    // EC_DynamicComponent.Attribute(..)
    if (assertValue !== value)
    {
        return log.error("Test failed - Expected: " + assertValue + " (" + (typeof assertValue) + ")" + 
            " Current value: " + value + " (" + (typeof value) + ")\n" + current());
    }
    // dyn.attributePropertyName
    if (testDynamicProp === true)
    {
        var dynPropName = dynamicPropertyName();
        var dynPropValue = (assertProperty !== undefined && dyn[dynPropName] != null ? dyn[dynPropName][assertProperty] : dyn[dynPropName]);
        if (assertValue !== dynPropValue)
        {
            // Special check for subproperties that are a empty string, this equals to "" real world value!
            if (assertProperty !== undefined && assertProperty !== null && typeof assertValue === "string" && dynPropValue === undefined)
                ; // ok
            else
                return log.error("Dynamic QObject property test failed - Expected: " + assertValue + " (" + (typeof assertValue) + ")" + 
                    " Current value: " + dynPropValue + " (" + (typeof dynPropValue) + ")\n" + current());
        }
    }
    return true;
}

function dynamicPropertyName()
{
    var name = "";
    var parts = attrId.split(" ");
    for (var i = 0; i < parts.length; i++)
        name += (parts[i].substring(0,1).toUpperCase() + parts[i].substring(1));
    return name.substring(0,1).toLowerCase() + name.substring(1);
}

var log =
{
    info  : function(msg) { console.LogInfo(msg); return true; },
    error : function(msg) { console.LogError(msg); return false; }
};

// Support running the full sweep of tests also when a new server scene is created.
// This will enable a client to connect and the test component being sent to him.
framework.Scene().SceneCreated.connect(function(createdScene) {
    log.info("Detected scene creation. Running test suite again.");

    scene       = createdScene;
    me          = scene.CreateEntity(scene.NextFreeId());
    me.name     = "DynamicComponent.js Test Suite";
    dyn         = me.CreateComponent("DynamicComponent");
    attrId      = undefined;

    print("");
    run();
    print("");

    framework.Scene().RemoveScene("DynamiComponentTestScene")
});

print("");
run();
print("");
