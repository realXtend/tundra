import rexviewer as r
import PythonQt

from circuits import Component

INTERNAL = 1

class MediaURLHandler(Component):
    def __init__(self):
        Component.__init__(self)

        mode = INTERNAL
        #self.canvas = r.createCanvas(mode)

        self.wv = PythonQt.QtWebKit.QWebView()
        
        #self.canvas.AddWidget(self.wv)
    
        uism = r.getUiSceneManager()
        uiprops = r.createUiWidgetProperty()
        uiprops.widget_name_ = "MediaURL"
        ##uiprops.my_size_ = QSize(width, height)
        ##self.proxywidget = uism.AddWidgetToCurrentScene(ui, uiprops)
        self.proxywidget = r.createUiProxyWidget(self.wv, uiprops)
        ##print widget, dir(widget)
        uism.AddProxyWidget(self.proxywidget)

    #def on_scene(self, evid, entid):
    #    pass
    #    print "Scene event in mediaurlhandler:", evid, entid

    def on_genericmessage(self, name, data):
        #print "MediaURLHandler got Generic Message:", name, data
        if name == "RexMediaUrl":
            print "MediaURLHandler got data:", data
            textureuuid, url, refreshrate = data
            
            self.loadurl(url)
            
            #r.applyUICanvasToSubmeshesWithTexture(self.canvas, textureuuid)
              

    def loadurl(self, urlstring):
        print "Showing URL", urlstring
        url = PythonQt.QtCore.QUrl(urlstring)
        self.wv.load(url)
