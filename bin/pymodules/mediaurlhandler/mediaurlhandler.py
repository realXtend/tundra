import rexviewer as r
import PythonQt
import urllib2
import naali

from circuits import Component

import loadurlhandler

''' @todo Create only one instance per videoplayer per url and reuse that widget if 
          it's used multiple times in a scene.
'''

class MediaurlView:
    ''' Creates WebView or Phonon::VideoPlayer widget to present given media url.
    '''

    def __init__(self, urlstring, refreshrate):
        self.__url = PythonQt.QtCore.QUrl(urlstring)
        self.refreshrate = refreshrate
        self.type = self.__get_mime_type(urlstring)
        self.__media_player_service_used = False
        self.playback_widget = None
        self.entid_to_submeshes = {}
        self.entid_clicked_connected = []
        
    def create_playback_widget(self):
        if len(self.type) > 0 and naali.mediaplayerservice is not None:
            if naali.mediaplayerservice.IsMimeTypeSupported(self.type):
                self.playback_widget = naali.mediaplayerservice.GetPlayerWidget(str(self.__url))
                self.__media_player_service_used = True
                r.logInfo("Media content supported for video playback: " + str(self.__url))
                return
            #else:
                #print(" -- media content is not supported.")
        # error -- playback widget cannot be created using player service -> we create a QWebview widget
        self.playback_widget = PythonQt.QtWebKit.QWebView()
        
    def load_url(self):
        if self.playback_widget != None and self.__media_player_service_used == False:
            self.playback_widget.load(self.__url)

    def delete_playback_widget(self):
        if self.playback_widget is None:
            return
        if self.__media_player_service_used:
            # delete phonon bases player widget object
            naali.mediaplayerservice.DeletePlayerWidget(self.__url.toString())
        else:
            # delete WebView object
            self.playback_widget.stop() # stop any networking done by this webview
            self.playback_widget.deleteLater() # let qt delete when sees fit
        self.playback_widget = None # set internal widget as gone
        self.entid_to_submeshes = {}
            
    def url(self):
        return self.__url

    def mouse_clicked(self):
        if not self.__media_player_service_used:
            loadurlhandler.loadurl(self.__url)
        
    def mouse_hover_in(self):
        pass
        
    def mouse_hover_out(self):
        pass
        
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
        r.logInfo('MediaURLHandler exiting...')    
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
                    if mediaurlview.url().toString() == urlstring:
                        return # we already have this information...
                    del self.texture2mediaurlview[textureuuid]
                    mediaurlview.delete_playback_widget()

            #could check whether a webview for this url already existed
            mv = MediaurlView(urlstring, refreshrate)

            #for when get visuals_modified events later, 
            #e.g. for newly downloaded objects
            self.texture2mediaurlview[textureuuid] = mv
            
            # Check if texture is in scene and only if is create playback widget
            if mv.playback_widget == None:
                # Dont remove this print, as in mediaurl heavy worlds there may come many of these on login
                # so dev can at least see in console that viewer has not died as it might seem so otherwise - Jonne
                r.logInfo("MediaUrlHandler: Checking scene for texture " + str(textureuuid))
                if r.checkSceneForTexture(textureuuid):
                    mv.create_playback_widget()
                    
            # If widget was created (texture found in scene), then apply to submeshes
            if mv.playback_widget != None:
                affected_entitys = r.applyUICanvasToSubmeshesWithTexture(mv.playback_widget, textureuuid, mv.refreshrate)
                # Connect affected entities Toucheble signals to to MediaurlView
                if affected_entitys != None:
                    for entity in affected_entitys:
                        self.connect_touchable(mv, entity)
                mv.load_url()

    def on_entity_visuals_modified(self, entid):
        #print "MediaURLHandler got Visual Modified for:", entid 
        for texture_uuid, mediaurlview in self.texture2mediaurlview.iteritems():
            if mediaurlview is None:
                continue
            submeshes = r.getSubmeshesWithTexture(entid, texture_uuid)
            if submeshes:
                # This doesent work. Seems to be right in theory but some entitys textures are not updated
                # especially when there are multiple entitys with same mediurl texture
                """
                # Don't reapply if submeshes are the same for this entity
                try:
                    if mediaurlview.entid_to_submeshes[entid] == submeshes:
                        print "Already have submeshes for ", str(entid)
                        print " >> Present: ", mediaurlview.entid_to_submeshes[entid]
                        print " >> Found  : ", submeshes
                        return
                except KeyError, e:
                    print " Adding new submeshes to ent id to map for ", str(entid)
                    print " >> ", submeshes
                    mediaurlview.entid_to_submeshes[entid] = submeshes
                """
                
                # Check if we need to create the playback widget
                if mediaurlview.playback_widget == None:
                    mediaurlview.create_playback_widget()
                    r.applyUICanvasToSubmeshes(entid, submeshes, mediaurlview.playback_widget, mediaurlview.refreshrate)
                    mediaurlview.load_url()
                else:
                    r.applyUICanvasToSubmeshes(entid, submeshes, mediaurlview.playback_widget, mediaurlview.refreshrate)
                # Get entity for id and hook to Touchable signals
                entity = r.getEntity(entid)
                self.connect_touchable(mediaurlview, entity)
                
    def connect_touchable(self, mediaurlview, entity):
        if mediaurlview == None or entity == None:
            return
        entid = entity.id
        try:
            mediaurlview.entid_clicked_connected.index(entid) # already hooked up this entity
            return
        except:
            mediaurlview.entid_clicked_connected.append(entid)
        try:
            touchable = entity.touchable
            if touchable != None:
                touchable.connect("Clicked()", mediaurlview.mouse_clicked)
            else:
                r.logWarning("MediaUrlHandler - Could not find touchable from entity")
        except:
            r.logWarning("MediaUrlHandler - Failed to connect EC_Touchable signals to mediaurlview")

    def on_input(self, evid):
        #print(">>> hander:on_input")    
        if evid == r.Undo: #WorldInputModule doesn't send all keys, just naali events, so am just reusing ctrl-z for testing herew
            for tx, mediaurlview in self.texture2mediaurlview.iteritems():
                if mediaurlview is None:
                    continue
                #print tx, mediaurlview.url().toString()
                r.applyUICanvasToSubmeshesWithTexture(mediaurlview.playback_widget, tx, mediaurlview.refreshrate)
