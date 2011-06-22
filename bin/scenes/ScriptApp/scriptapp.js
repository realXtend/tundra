print("Script application loaded");

function TestClass(entity, comp)
{
    print("TestClass Constructor called, entityid " + entity.id);
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
}

function TestClientClass(entity, comp)
{
    print("TestClientClass constructor called");
}

function TestServerClass(entity, comp)
{
    print("TestServerClass constructor called");
}




