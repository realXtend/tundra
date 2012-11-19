//tests usage of the pre-modification signals in EC syncmanager,
//which can be used to filter changes to the scene to e.g. implement permissions

//first: prevent any remote modification of the scene.
//av app, chat etc. still work, as they send entity actions, not modify entities on the client
sceneapi.SceneAdded.connect(function(scenename) {
    var s = sceneapi.GetScene(scenename);
    print("Adding all-denying perm hook to scene called: " + scenename);

    s.AboutToModifyEntity.connect(function(change, user, entity) {
        print("Denying change by user: " + user + " on entity: " + entity);
        change.Deny();
    });
});
