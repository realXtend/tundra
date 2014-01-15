
var attrTypes   = framework.Scene().attributeTypes;
var scene       = framework.Scene().CreateScene("DynamiComponentTestScene", false, true);
var me          = scene.CreateLocalEntity();
var dyn         = me.CreateComponent("DynamicComponent");
var attrName    = undefined;

function run()
{
    log.info("Running DynamicComponent.js\n");

    for (var i = 0; i < attrTypes.length; i++)
    {
        var attr = dyn.CreateAttribute(attrTypes[i], attrTypes[i] + " test");
        if (attr !== undefined && attr !== null)
        {
            dumpAttribute(attr);
            setAttribute(attr);
        }
    }
}

function setAttribute(attr)
{
    attrName = attr.name;

    if (attr.typename === "AssetReference")
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
    else if (attr.typename === "AssetReferenceList")
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
    else if (attr.typename === "EntityReference")
    {
        setAttributeValue("TestEntity1", "ref");
        setAttributeValue(100, "ref");
        setAttributeValue({ ref: "TestEntity2" }, "ref");
        setAttributeValue({ ref: 150 }, "ref");
        setAttributeValue(new EntityReference("TestEntity3"), "ref");
        setAttributeValue(new EntityReference(200), "ref");

        var other = scene.CreateLocalEntity();
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
    dyn.SetAttribute(attrName, value);

    var ok = false;

    if (value === null || value === undefined || 
        (typeof assertProperty === "string" && value.hasOwnProperty(assertProperty) && value[assertProperty] === null) ||
        (typeof assertProperty === "string" && value.hasOwnProperty(assertProperty) && value[assertProperty] === undefined))
    {
        if (typeof current(assertProperty) === "string")
            ok = assertAttribute(current(assertProperty), "");
    }
    else if (typeof value === "string" || typeof value === "number" || typeof value === "boolean")
    {
        // If the current value is a string and the set value is a number. Auto convert for assert.
        // This fixes the tests failing when setting EntityReference with a number.
        if (typeof value === "number" && typeof current(assertProperty) === "string")
            ok = assertAttribute(current(assertProperty), value.toString());
        else
            ok = assertAttribute(current(assertProperty), value);
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
                ok = assertAttribute(current(subProperty), value[subProperty]);
                if (!ok) break;
            }
        }
    }

    if (ok)
        log.info("    -- OK " + current());
}

function current(subProperty)
{
    var value = dyn.Attribute(attrName);
    if (subProperty !== undefined)
        return value[subProperty];
    return value;
}

function dumpAttribute(attr)
{
    var value = dyn.Attribute(attr.name);
    log.info("Attribute (" + attr.name + ")");
    log.info("  Type       : " + attr.typename);
    log.info("  JS typeof  : " + typeof value + (Array.isArray(value) ? " (Array)" : ""));
    log.info("  Value      : " + value);

    if (attr.owner === undefined || attr.owner === null)
        log.error("  Owner component is invalid: " + attr.owner);
}

function assertAttribute(value, assertValue)
{
    if (assertValue !== value)
        return log.error("Test failed - Expected: " + assertValue + " (" + (typeof assertValue) + ")" + 
            " Current value: " + value + " (" + (typeof value) + ")\n" + current());
    return true;
}

var log =
{
    info  : function(msg) { console.LogInfo(msg); return true; },
    error : function(msg) { console.LogError(msg); return false; }
};

print("");
run();
print("");