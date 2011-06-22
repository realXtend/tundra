print("Script application loaded");

function OnScriptDestroyed()
{
    print("Script destructor called");
}

function TestClass(entity, comp)
{
    print("TestClass constructor called, entityid " + entity.id);
    this.counter = 0;
    this.entityId = entity.id;
    frame.DelayedExecute(1.0).Triggered.connect(this, this.Execute);
}

TestClass.prototype.Execute = function()
{
    this.counter++;
    frame.DelayedExecute(1.0).Triggered.connect(this, this.Execute);
}

TestClass.prototype.OnScriptObjectDestroyed = function()
{
    print("TestClass destructor called");
}

function TestClientClass(entity, comp)
{
    print("TestClientClass constructor called");
}

TestClientClass.prototype.OnScriptObjectDestroyed = function()
{
    print("TestClientClass destructor called");
}

function TestServerClass(entity, comp)
{
    print("TestServerClass constructor called");
}

TestServerClass.prototype.OnScriptObjectDestroyed = function()
{
    print("TestServerClass destructor called");
}




