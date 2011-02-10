import json

import naali
import loadurlhandler

#apparently usage of dict in autoload changes the order from ini, hacking around load order prob now XXX
import circuits
class WebLogin(circuits.BaseComponent):
    inited = False
    #def __init__(self):
    #    circuits.BaseComponent.__init__(self)
    @circuits.handler("update")
    def update(self, frametime):
        if not self.inited:
            self.init()
            self.inited = True #XXX if need to keep this hack, at least remove this update handler

    def init(self):
        loadurlhandler.loadurl("http://localhost/GridFrontend")

        webview = loadurlhandler.loadurlhandler.instance.webview
        webview.show()
        page = webview.page()
        page.forwardUnsupportedContent = True

        page.connect("unsupportedContent(QNetworkReply*)", handlelaunch)

def handlelaunch(netreply): 
    netdata = str(netreply.readAll())
    print netdata

    try:
        info = json.loads(netdata)
    except ValueError: #wasn't json
        print "login/webui tried to handle something it shouldn't?", data[:160]
        return

    if not "authenticator" in info:
        print "login/webui got json which is not a world launch doc?", info
        return

    username = info['identifier']['name']
    loginurl = info['loginurl'] #has the cap (i.e. token)
        
    c = naali.client
    c.SetLoginProperty("loginurl", loginurl)

    #XXX TODO W.I.P unifinished implementation - got lost in the RFCs, not understanding loginurl etc.
    #so this is now just a hardcoded login actually :p .. and there is no server counterpart for the caps stuff yet
    c.Login("localhost", 2345, "Test_User", "tester")
