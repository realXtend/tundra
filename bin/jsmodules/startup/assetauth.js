engine.ImportExtension("qt.core");
engine.ImportExtension("qt.network");

var show_debug = true;
function dprint(msg) {
    if (!show_debug)
	return;
    print("######### " + msg);
}

asset.AssetStorageAdded.connect(function(storage) {
	if (storage.Type() != "HttpAssetStorage")
	    return;
	config_http_storage(storage);
    });
dprint("hooked AssetStorageAdded");

var storages = asset.GetAssetStorages();
for (var i = 0; i < storages.length; i++)
    if (storage.Type() == "HttpAssetStorage")
	config_http_storage(storages[i]);
dprint("did existing storages");

function auth_handler(reply, authenticator) {
    dprint("auth handler called");
    var username = "bob";
    var password = "secretpassword";
    if (reply.request().url().host() == "test.example.com" &&
	reply.request().url().port(80) == 80 &&
	reply.request().url().scheme() == "http") {
	if (authenticator.realm() == "my-authtest") {
	    dprint("matching authenticationRequired seen, filling in credentials");
	    authenticator.setUser(username);
	    authenticator.setPassword(password);
	}
    }
}

function config_http_storage(storage) {
    var qnam = storage.GetNetworkAccessManager();
    qnam.authenticationRequired.connect(auth_handler);
    dprint("handling auth for storage "+ storage.Name());
}
