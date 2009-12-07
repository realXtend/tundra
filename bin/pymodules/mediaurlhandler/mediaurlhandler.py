import rexviewer as r
import PythonQt

from circuits import Component

INTERNAL = 1

class MediaURLHandler(Component):
    def __init__(self):
        Component.__init__(self)

        mode = INTERNAL
        self.canvas = r.createCanvas(mode)

        self.wv = PythonQt.QtWebKit.QWebView()
        
        self.canvas.AddWidget(self.wv)
    
        qtmodule = r.getQtModule()
        qtmodule.AddCanvasToControlBar(self.canvas, "WebView")
        #self.canvas.connect('Hidden()', self.on_hide)

        url = PythonQt.QtCore.QUrl("http://an.org/")
        self.wv.load(url)
        
    #def on_hide(self):
        
        
