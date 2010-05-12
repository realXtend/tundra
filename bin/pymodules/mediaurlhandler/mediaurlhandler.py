import rexviewer as r
import PythonQt

from circuits import Component

class MediaurlView:
    def __init__(self, urlstring, refreshrate):
        self.webview = PythonQt.QtWebKit.QWebView()
        url = PythonQt.QtCore.QUrl(urlstring)
        self.webview.load(url)
        self.refreshrate = refreshrate
        #self.webview.show()

class MediaURLHandler(Component):
    def __init__(self):
        Component.__init__(self)
        #which texture uuids with mediaurl are in which webview wrapper
        self.texture2webview = {}

        """
        the code below was to test webviews first as 2d widgets. 
        perhaps useful later when add feat to open from 3d canvas to 2d usage
        """
        #uism = r.getUiSceneManager()
        #uiprops = r.createUiWidgetProperty()
        #uiprops.widget_name_ = "MediaURL"
        ##uiprops.my_size_ = QSize(width, height)
        ##self.proxywidget = uism.AddWidgetToScene(ui, uiprops)
        #self.proxywidget = r.createUiProxyWidget(self.wv, uiprops)
        ##print widget, dir(widget)
        #uism.AddProxyWidget(self.proxywidget)
        #self.wv.show()
        
    def on_logout(self, id):
        r.logInfo("Uninitializing MediaURLHandler due to logout, deleting all open webviews")
        for textureid, mediaurlview in self.texture2webview.iteritems():
            mediaurlview.webview.delete()
        self.texture2webview.clear()

    def on_exit(self):
        r.logInfo("Uninitializing MediaURLHandler")
        for textureid, mediaurlview in self.texture2webview.iteritems():
            mediaurlview.webview.delete()
        self.texture2webview = None
        
    def on_genericmessage(self, name, data):
        #print "MediaURLHandler got Generic Message:", name, data
        if name == "RexMediaUrl":
            #print "MediaURLHandler got data:", data
            textureuuid, urlstring, refreshrate = data
            refreshrate = int(refreshrate)

            #could check whether a webview for this url already existed
            mv = MediaurlView(urlstring, refreshrate)
            
            #for objects we already had in the scene
            r.applyUICanvasToSubmeshesWithTexture(mv.webview, textureuuid, mv.refreshrate)
            
            #for when get visuals_modified events later, 
            #e.g. for newly downloaded objects
            self.texture2webview[textureuuid] = mv
                          
    def on_entity_visuals_modified(self, entid):
        #print "MediaURLHandler got Visual Modified for:", entid
        #XXX add checks to not re-apply blindly when is already up-to-date!
        for tx, wc in self.texture2webview.iteritems():
            submeshes = r.getSubmeshesWithTexture(entid, tx)
            if submeshes:
                #print "Modified entity uses a known mediaurl texture:", entid, tx, submeshes, wc
                r.applyUICanvasToSubmeshes(entid, submeshes, wc.webview, wc.refreshrate)
        
    def on_input(self, evid):
        if evid == r.Undo: #WorldInputModule doesn't send all keys, just naali events, so am just reusing ctrl-z for testing herew
            for tx, wc in self.texture2webview.iteritems():
                print tx, wc.webview.url.toString()
                r.applyUICanvasToSubmeshesWithTexture(wc.webview, tx, wc.refreshrate)
