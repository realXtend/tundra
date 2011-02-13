"""
A Naali UI widget made as a WebView, i.e. html+javascript.
In other words, a test of exposing the internal Naali Javascript API to a browser context.

Why? 
1. Many people know how to make web pages, more than do qt ui programming.
2. It is yet unknown whether QT UI widgets are safe for untrusted code -- web browsers are a proven GUI sandbox.

And it is kind of fun, no?
"""


from PythonQt.QtCore import QUrl
from PythonQt.QtWebKit import QWebView

import circuits
import rexviewer #proxywidget factory still needed from there
import naali

UIURL = "file:///home/antont/src/naali/bin/pymodules/apitest/webui.html"

def initcontext(f):
    """
    initializes the custom Naali javascript context for a webview frame,
    i.e. adds the objects that are exposed from Naali to the browser JS.
    """
    f.addToJavaScriptWindowObject("avatar", naali.getUserAvatar())

    worldstream = rexviewer.getServerConnection()
    f.addToJavaScriptWindowObject("connection", worldstream)

class WebuiDisplay(circuits.BaseComponent):
    def __init__(self):
        circuits.BaseComponent.__init__(self)

        self.webview = QWebView()
        
        #naali proxywidget boilerplate
        GUINAME = "WebUI Test"
        uism = naali.ui
        self.proxywidget = rexviewer.createUiProxyWidget(self.webview)
        self.proxywidget.setWindowTitle(GUINAME)
        if not uism.AddWidgetToScene(self.proxywidget):
            print "Adding the ProxyWidget to the bar failed."
        uism.AddWidgetToMenu(self.proxywidget, GUINAME, "Developer Tools")

        self.proxywidget.connect("Visible(bool)", self.handlevis)

    def handlevis(self, vis):
        if vis:
            self.show()

    def show(self):
        print "Loading WebUI from", UIURL
        self.webview.load(QUrl(UIURL))
        f = self.webview.page().mainFrame()
        initcontext(f)
    
    @circuits.handler("on_exit")
    def on_exit(self):
        rexviewer.logInfo("WebuiDisplay starting exit...")
        
        uism = naali.ui
        uism.RemoveWidgetFromScene(self.proxywidget)
        uism.RemoveWidgetFromMenu(self.proxywidget)
        rexviewer.logInfo("WebuiDisplay proxywidget removed from scene.")
