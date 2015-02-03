/** For conditions of distribution and use, see copyright notice in LICENSE

    WaypointBot.js - A simple bot that walks along predefined waypoints. */

var waypoints = [new float3(10, 0, 10), new float3(-10, 0, 10), new float3(-10, 0, -10), new float3(10, 0, -10)];
var currentWaypoint = 0;

// Init. waypoints
for(i in waypoints)
    waypoints[i] = waypoints[i].Add(me.placeable.WorldPosition());

if (server.IsRunning())
{
    scene.physics.Updated.connect(ServerPhysicsUpdate);
    if (!framework.IsHeadless())
        frame.Updated.connect(AnimationUpdate);
}
else
    frame.Updated.connect(AnimationUpdate);

function ServerPhysicsUpdate(frameTime)
{
    var targetPos = waypoints[currentWaypoint];
    TurnToSmooth(ComputeTargetHeading(targetPos), 0.05);

    // Move the bot
    me.rigidbody.linearVelocity = me.placeable.Orientation().Mul(new float3(0, 0, -5));

    if (HasReachedTarget(targetPos, 0.25))
        if (++currentWaypoint >= waypoints.length)
            currentWaypoint = 0; // start over
}

function AnimationUpdate(frameTime)
{
    if (!me.mesh.HasMesh())
        return; // mesh and/or skeleton not loaded yet

    if (!me.animationcontroller.IsAnimationActive("Walk"))
        me.animationcontroller.EnableAnimation("Walk", true, 0.25, false)

    // Scale the current playback speed of the walk animation according to our linear velocity
    me.animationcontroller.SetAnimationSpeed("Walk", 0.3 * me.rigidbody.linearVelocity.Length());
}

// Rotates smoothly (slerp) the bot around the Y-axis towards a target heading in degrees
function TurnToSmooth(targetHeading, weight)
{
    var targetRot = new Quat(new float3(0, 1, 0), targetHeading / (180 / Math.PI));
    me.placeable.SetOrientation(me.placeable.Orientation().Slerp(targetRot, weight));
}

// Calculates the heading (rotation arond Y-axis) required to look from the bot's current position to a target
function ComputeTargetHeading(targetPos)
{
    var targetDirection = targetPos.Sub(me.placeable.WorldPosition());
    targetDirection.y = 0; // for now at least, we're only interested in motion on the XZ plane
    return Math.atan2(-targetDirection.x, -targetDirection.z) * (180 / Math.PI);
}

// Checks if the bot has reached a target position, with a threshold distance given
function HasReachedTarget(targetPos, threshold)
{
    var currentPos = me.placeable.WorldPosition();
    targetPos.y = currentPos.y = 0; // for now at least, we're only interested in motion on the XZ plane
    return targetPos.Sub(currentPos).Length() < threshold;
}
