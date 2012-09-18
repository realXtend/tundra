// Portal script which enables possibility to make new connection attempts by clicking objects.
// Place this script to your entity which has placeable and visible mesh.
// Jukka Vatjus-Anttila / CIE

print("Portal.js: script initialized in " + me);

var isServer = server.IsRunning();

if (!isServer)
{
    var parentEntity = this.me;
    if (!parentEntity)
        return;
    input.TopLevelInputContext().MouseLeftPressed.connect(mouseLeftPress);

    function mouseLeftPress(event)
    {
        // Get entity from mouseclick location.
        var raycastResult = scene.ogre.Raycast(event.x, event.y, 0xffffffff);
        if(raycastResult.entity != null)
        {
            // Check if clicked entity was parentEntity for this script
            if (raycastResult.entity == parentEntity)
            {
                // This should be changed according to the scene. If no avatar present then this is void.
                var avatar = scene.GetEntityByName("Avatar" + client.GetConnectionID());
                var avatarPos = new float3();
                var parentPos = new float3();

                avatarPos = avatar.placeable.transform.pos;
                parentPos = parentEntity.placeable.transform.pos;

                // Lets check if controlled avatar is inside arbitary range to initialize login procedure.
                var distance = avatarPos.Distance(parentPos);
                if (distance < 3)
                {

                    // This disconnect should be disabled if multiple simultaneous connections are wanted with multiconnection feature.a
                    console.ExecuteCommand("Disconnect()");
                    // Commence epic login
                    client.Login("localhost", 2347,"lal", "pass", "udp");
                }
            }
        }
    }
}
