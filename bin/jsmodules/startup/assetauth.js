
asset.AssetStorageAdded.connect(function(storage) {
	if (storage.Type() != "HttpAssetStorage")
	    return;
	config_http_storage(storage);
    });

var storages = asset.GetAssetStorages();
for (var i = 0; i < storages.length; i++)
    if (storage.Type() == "HttpAssetStorage")
	config_http_storage(storages[i]);

function auth_handler(reply, authenticator) {
    var username = "kek";
    var password = "secret";
    if (reply.request().url().host() == "my_known_host" and reply.request().url().port(80) == 80 and reply.request.scheme() == 'https') {
	if (authenticator.realm() == "my_realm") {
	    authenticator.setUser(username);
	    authenticator.setPassword(password);
	    
}

function config_http_storage(storage) {
    var qnam = storage.GetNetworkAccessManager();
    qnam.authenticationRequired.connect(auth_handler);
}
