#httplib was ok and httplib2 especially had nice api, but they don't work thru proxies and stuff
#-- curl is the most robust thing
#import httplib
import curl #a high level wrapper over pycurl bindings
import json
import hashlib #only 'cause has a hardcoded pwd here now - for real this comes from connection or launcher

try:
    import naali
except ImportError:
    naali = None #so that can test standalone too, without Naali
else:
    import circuits
    class SimiangridAuthentication(circuits.BaseComponent):
        pass #put disconnecting to on_exit here to not leave old versions while reloading

url = "http://localhost/Grid/"

c = curl.Curl()
def simiangrid_auth(url, username, md5hex):
    params = {'RequestMethod': 'AuthorizeIdentity', 
             'Identifier': username, 
             'Type': 'md5hash', 
             'Credential': md5hex}

    rdata = c.post(url, params)

    print rdata
    r = json.loads(rdata)

    #http://code.google.com/p/openmetaverse/wiki/AuthorizeIdentity
    success = r.get('Success', False) 
    #NOTE: docs say reply should have Success:false upon failure.
    #however in my test run it doesn't just the Message of missing/invalid creds
    #this code works for that too. 

    return success

def on_connect(conn_id, userconn):
    print userconn.GetLoginData()
    username = userconn.GetProperty("username")
    username = username.replace('_', ' ') #XXX HACK: tundra login doesn't allow spaces, whereas simiangrid frontend demands them
    pwd = userconn.GetProperty("password")

    md5hex = hashlib.md5(pwd).hexdigest()
    success = simiangrid_auth(url, username, md5hex)
    print "Authentication success:", success, "for", conn_id, userconn

    if not success:
        userconn.DenyConnection()

if naali is not None:
    s = naali.server
    if s.IsAboutToStart():
        s.connect("UserAboutToConnect(int, UserConnection*)", on_connect)
        print "simiangrid/auth.py running on server - hooked to authorize connections"

else:
    on_connect(17, {'username': "Lady Tron",
                    'password': "They only want you when you're seventeen"})

    """
    { "Success":true, "UserID":"fe5f5ac3-7b28-4276-ae50-133db72040f0" }
    Authentication success: True
    """
