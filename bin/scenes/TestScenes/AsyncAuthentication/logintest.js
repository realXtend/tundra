if (server.IsRunning() || server.IsAboutToStart()) {
    server.UserAboutToConnect.connect(ServerHandleUserAboutToConnect);
    server.UserConnected.connect(ServerHandleUserConnected);
    server.UserDisconnected.connect(ServerHandleUserDisconnected);
    print("logintest signals connected");
}

function ServerHandleUserAboutToConnect(connectionID, user) {
    user.FinishLoginLater();
    frame.DelayedExecute(2.0).Triggered.connect(function(t) {
	    if (user.GetProperty("password") != "secret") {
		user.DenyConnection("wrong password");
		print("wrong password");
	    } else {
		print("correct password");
	    }
	    print("calling FinishLogin");
	    server.FinishLogin(user);
	});
    
}

function ServerHandleUserConnected(connectionID, user) {
    var username = null;
    if (user != null)
    	username = user.GetProperty("username");
    print("user " + username + " connected");
}

function ServerHandleUserDisconnected(connectionID, user) {
    var username = null;
    if (user != null)
    	username = user.GetProperty("username");
    print("user " + username + " disconnected");
	    
}
    
print("logintest end");