
var hydrax = null;

frame.DelayedExecute(1).Triggered.connect(WaitForHydrax)

function WaitForHydrax()
{
    hydrax = scene.Components("Hydrax")[0];
    if (hydrax)
        frame.Updated.connect(Update);
    else
        frame.DelayedExecute(1).Triggered.connect(WaitForHydrax)
}

function Update()
{
    profiler.BeginBlock("FloatingBox_Update");
    
    var pos = me.placeable.WorldPosition();
    pos.y = hydrax.HeightAt(pos);
    me.placeable.SetPosition(pos);

    profiler.EndBlock();
}
