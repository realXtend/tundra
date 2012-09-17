// Portal script which enables possibility to make new connection attempts by clicking objects.
// Place this script to your entity which has placeable and visible mesh.
// Jukka Vatjus-Anttila / CIE

print("Portal.js: script initialized in " + me);

var isServer = server.IsRunning();

if (isServer)
    return;
else
{
    var parentEntity = this.me;
    input.TopLevelInputContext().MouseLeftPressed.connect(mouseLeftPress);

    function mouseLeftPress(event)
    {
        if (!parentEntity)
            return;
        // Get entity from mouseclick location.
        var raycastResult = scene.ogre.Raycast(event.x, event.y, 0xffffffff);
        if(raycastResult.entity != null)
        {
            if (raycastResult.entity == parentEntity)
            {
                // This should be changed according to the scene. If no avatar present then this is void.
                var avatar = scene.GetEntityByName("Avatar" + client.GetConnectionID());
                var avatarPos = new float3();
                var parentPos = new float3();

                avatarPos = avatar.placeable.transform.pos;
                parentPos = parentEntity.placeable.transform.pos;

                var distance = avatarPos.Distance(parentPos);
                if (distance < 3)
                {

                    // This disconnect should be disabled if multiple simultaneous connections are wanted.
                    //console.ExecuteCommand("Disconnect()");
                    client.Login("localhost", 2347,"lal", "pass", "udp");
                }
            }
        }
    }
}
