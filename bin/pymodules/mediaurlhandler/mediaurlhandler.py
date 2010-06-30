import rexviewer as r
import PythonQt
import urllib2

from circuits import Component

''' @todo Create only one instance per videoplayer per url and reuse that widget if 
          it's used multiple times in a scene.
'''

class MediaurlView:
    ''' Creates WebView or Phonon::VideoPlayer widget to present given media url.
    '''

    def __init__(self, urlstring, refreshrate):
        self.__url = PythonQt.QtCore.QUrl(urlstring)
        self.refreshrate = refreshrate
        type = self.__get_mime_type(urlstring)
        if len(type) > 0 and r.isMimeTypeSupportedForVideoWidget(type):
            self.playback_widget = r.createVideoWidget(str(urlstring))
        else:
            self.playback_widget = PythonQt.QtWebKit.QWebView()
            self.playback_widget.load(self.__url)

    def delete_playback_widget(self):
        if self.playback_widget is None:
            return
        if type(self.playback_widget).__name__ == 'Phonon::VideoPlayer':
            # Phonon::VideoPlayer object
            r.deleteVideoWidget(self.__url.toString())
            self.playback_widget = None
        else:
            # VewView object
            self.playback_widget.delete()
            
    def url(self):
        return self.__url        
        
    def __get_mime_type(self, url_string):   
        try:
            file = urllib2.urlopen(url_string)
            type = str(file.info()["Content-Type"])
            file.close()
            return type
        except:
            return ""

class MediaURLHandler(Component):
    def __init__(self):
        Component.__init__(self)
        #which texture uuids with mediaurl are in which webview wrapper
        self.texture2mediaurlview = {}

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
        #r.logInfo("Uninitializing MediaURLHandler due to logout, deleting all open playback widgets")
        for textureid, mediaurlview in self.texture2mediaurlview.iteritems():
            if mediaurlview is None:
                continue
            self.texture2mediaurlview[textureid] = None
            mediaurlview.delete_playback_widget()
        self.texture2mediaurlview.clear()

    def on_exit(self):
        #r.logInfo("Uninitializing MediaURLHandler")
        for textureid, mediaurlview in self.texture2mediaurlview.iteritems():
            if mediaurlview is None:
                continue
            mediaurlview.delete_playback_widget()        
        self.texture2mediaurlview.clear()                
        self.texture2mediaurlview = None
        
    def on_genericmessage(self, name, data):
        #print "MediaURLHandler got Generic Message:", name, data
        if name == "RexMediaUrl":
            #print "MediaURLHandler got data:", data
            textureuuid, urlstring, refreshrate = data
            refreshrate = int(refreshrate)
            
            if self.texture2mediaurlview.has_key(textureuuid):
                mediaurlview = self.texture2mediaurlview[textureuuid]
                if mediaurlview is not None:
                    del self.texture2mediaurlview[textureuuid]
                    mediaurlview.delete_playback_widget()                        

            #could check whether a webview for this url already existed
            mv = MediaurlView(urlstring, refreshrate)

            #for when get visuals_modified events later, 
            #e.g. for newly downloaded objects
            self.texture2mediaurlview[textureuuid] = mv
            
            #for objects we already had in the scene
            r.applyUICanvasToSubmeshesWithTexture(mv.playback_widget, textureuuid, mv.refreshrate)
                          
    def on_entity_visuals_modified(self, entid):
        #print "MediaURLHandler got Visual Modified for:", entid 
        #XXX add checks to not re-apply blindly when is already up-to-date!
        for tx, mediaurlview in self.texture2mediaurlview.iteritems():
            if mediaurlview is None:
                continue
            submeshes = r.getSubmeshesWithTexture(entid, tx)
            if submeshes:
                #print "Modified entity uses a known mediaurl texture:", entid, tx, submeshes, mediaurlview
                r.applyUICanvasToSubmeshes(entid, submeshes, mediaurlview.playback_widget, mediaurlview.refreshrate)
        
    def on_input(self, evid):
        #print(">>> hander:on_input")    
        if evid == r.Undo: #WorldInputModule doesn't send all keys, just naali events, so am just reusing ctrl-z for testing herew
            for tx, mediaurlview in self.texture2mediaurlview.iteritems():
                if mediaurlview is None:
                    continue
                #print tx, mediaurlview.url().toString()
                r.applyUICanvasToSubmeshesWithTexture(mediaurlview.playback_widget, tx, mediaurlview.refreshrate)
