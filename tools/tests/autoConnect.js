var randomnumber=Math.floor(Math.random()*100)

//console.ExecuteCommand("connect(chiru.cie.fi,2345,Pertti"+randomnumber.toString()+")" )
function AutoConnect()
{
    var isserver = server.IsRunning() || server.IsAboutToStart();
    if(isserver==false){
        //client.Login("localhost",2345,"Guest"+randomnumber.toString()," ","udp");
	client.Login("chiru.cie.fi",2345,"Guest"+randomnumber.toString()," ","tcp");
    }
}



frame.DelayedExecute(1).Triggered.connect(this,AutoConnect);

function Quit()
    {
        framework.Exit();
    }

//asset.OpenAssetCache("~/realxtend/bin/data/assets");

frame.DelayedExecute(90).Triggered.connect(this, Quit);

//var clientID=Math.floor(Math.random()*3)+1
//var avatar = "Avatar" + randomnumb1.toString();

function getAvatarID()	
{
	var clientID = client.GetConnectionID();
	var avatarx = "Avatar" + clientID.toString();
	return avatarx;
}
function MoveForward() 
 {
	
	var avatar = getAvatarID();
        var scene = framework.Scene();
	print(scene); 
	print(avatar);
	print(" forward");
	var omaScene = scene.GetDefaultSceneRaw();
	var myAvatar = omaScene.GetEntityByNameRaw(avatar);
        var moveAction = myAvatar.Exec(2, "Move", "forward");    
}
function MoveBack() 
 {
	var avatar = getAvatarID();
        var scene = framework.Scene();
	print(scene); 
	print(avatar);
	print(" back");
	var omaScene = scene.GetDefaultSceneRaw();
	var myAvatar = omaScene.GetEntityByNameRaw(avatar);
	var moveAction = myAvatar.Exec(2, "Move", "back");
}
function StopForward() 
 {
	var avatar = getAvatarID();
        var scene = framework.Scene();
	print(scene); 
	print(avatar);
	print(" stop forward");
	var omaScene = scene.GetDefaultSceneRaw();
	var myAvatar = omaScene.GetEntityByNameRaw(avatar);
        var moveAction = myAvatar.Exec(2, "Stop", "forward");      
}
function StopBack() 
 {
	var avatar = getAvatarID();
        var scene = framework.Scene();
	print(scene); 
	print(avatar);
	print(" stop back");
	var omaScene = scene.GetDefaultSceneRaw();
	var myAvatar = omaScene.GetEntityByNameRaw(avatar);
        var moveAction = myAvatar.Exec(2, "Stop", "back");      
}
function RotateRight() 
 {
	var avatar = getAvatarID();
        var scene = framework.Scene();
	print(scene); 
	print(avatar);
	print(" right");
	var omaScene = scene.GetDefaultSceneRaw();
	var myAvatar = omaScene.GetEntityByNameRaw(avatar);
	var moveAction = myAvatar.Exec(2, "Rotate", "right");
}
function RotateLeft() 
 {
	var avatar = getAvatarID();
        var scene = framework.Scene();
	print(scene); 
	print(avatar);
	print(" left");
	var omaScene = scene.GetDefaultSceneRaw();
	var myAvatar = omaScene.GetEntityByNameRaw(avatar);
	var moveAction = myAvatar.Exec(2, "Rotate", "left");
}
function RotateStopRight() 
 {
	var avatar = getAvatarID();
        var scene = framework.Scene();
	print(scene); 
	print(avatar);
	print(" stop right");
	var omaScene = scene.GetDefaultSceneRaw();
	var myAvatar = omaScene.GetEntityByNameRaw(avatar);
	var moveAction = myAvatar.Exec(2, "StopRotate", "right");
}
function RotateStopLeft() 
 {
	var avatar = getAvatarID();
        var scene = framework.Scene();
	print(scene); 
	print(avatar);
	print(" stop left");
	var omaScene = scene.GetDefaultSceneRaw();
	var myAvatar = omaScene.GetEntityByNameRaw(avatar);
	var moveAction = myAvatar.Exec(2, "StopRotate", "left");
}

/*
 The following slots are available:
 Exec(int type, QString action, QString p1, QString p2, QString p3)
 Exec(int type, QString action, QString p1, QString p2)
 Exec(int type, QString action, QString p1)
 Exec(int type, QString action)
 Exec(int type, QString action, QStringList params)
 Exec(int type, QString action, QVariantList params)
*/

var randomnumb=Math.floor(Math.random()*21)
var startTime = 10+randomnumb;
frame.DelayedExecute(startTime).Triggered.connect(this,MoveForward);
frame.DelayedExecute(startTime + 5).Triggered.connect(this,StopForward);
frame.DelayedExecute(startTime + 10).Triggered.connect(this,RotateRight);
frame.DelayedExecute(startTime + 10.7).Triggered.connect(this,RotateStopRight);
frame.DelayedExecute(startTime + 13).Triggered.connect(this,MoveForward);
frame.DelayedExecute(startTime + 16).Triggered.connect(this,StopForward);
frame.DelayedExecute(startTime + 16.1).Triggered.connect(this,RotateRight);
frame.DelayedExecute(startTime + 16.8).Triggered.connect(this,RotateStopRight);
frame.DelayedExecute(startTime + 17).Triggered.connect(this,MoveForward);
frame.DelayedExecute(startTime + 17.5).Triggered.connect(this,RotateRight);
frame.DelayedExecute(startTime + 18.2).Triggered.connect(this,RotateStopRight);
frame.DelayedExecute(startTime + 20).Triggered.connect(this,StopForward);
frame.DelayedExecute(startTime + 20.5).Triggered.connect(this,RotateLeft);
frame.DelayedExecute(startTime + 21.2).Triggered.connect(this,RotateStopLeft);
frame.DelayedExecute(startTime + 21.2).Triggered.connect(this,MoveForward);
frame.DelayedExecute(startTime + 25).Triggered.connect(this,StopForward);
frame.DelayedExecute(startTime + 25.5).Triggered.connect(this,RotateLeft);
frame.DelayedExecute(startTime + 25.9).Triggered.connect(this,RotateStopLeft);
frame.DelayedExecute(startTime + 26).Triggered.connect(this,MoveForward);
frame.DelayedExecute(startTime + 30).Triggered.connect(this,StopForward);
frame.DelayedExecute(startTime + 32).Triggered.connect(this,RotateLeft);
frame.DelayedExecute(startTime + 33).Triggered.connect(this,RotateStopLeft);
frame.DelayedExecute(startTime + 34).Triggered.connect(this,MoveForward);
frame.DelayedExecute(startTime + 40).Triggered.connect(this,StopForward);
frame.DelayedExecute(startTime + 41).Triggered.connect(this,RotateRight);
frame.DelayedExecute(startTime + 41.9).Triggered.connect(this,RotateStopRight);
frame.DelayedExecute(startTime + 42).Triggered.connect(this,MoveForward);
frame.DelayedExecute(startTime + 50).Triggered.connect(this,StopForward);
frame.DelayedExecute(startTime + 51).Triggered.connect(this,RotateRight);
frame.DelayedExecute(startTime + 51.9).Triggered.connect(this,RotateStopRight);
frame.DelayedExecute(startTime + 52).Triggered.connect(this,MoveForward);
frame.DelayedExecute(startTime + 53).Triggered.connect(this,RotateRight);
frame.DelayedExecute(startTime + 53.9).Triggered.connect(this,RotateStopRight);
frame.DelayedExecute(startTime + 66).Triggered.connect(this,RotateRight);
frame.DelayedExecute(startTime + 66.4).Triggered.connect(this,RotateStopRight);
frame.DelayedExecute(startTime + 73).Triggered.connect(this,StopForward);



