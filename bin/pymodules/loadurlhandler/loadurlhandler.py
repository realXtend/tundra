import rexviewer as r
import PythonQt

from circuits import Component
from PythonQt.QtGui import QWidget, QVBoxLayout
from PythonQt.QtWebKit import QWebView

instance = None

class LoadURLHandler(Component):
    def __init__(self):
        Component.__init__(self)
        self.proxywidget = None
        self.container = None
        self.webview = None
        
        global instance
        instance = self

    def loadurl(self, url):       
        if self.webview == None and self.container == None and self.proxywidget == None:
            self.init_ui()

        self.webview.load(url)
        print "Loaded url:", str(url)
        if not self.proxywidget.isVisible():
            print "Showing webview proxy in scene"
            self.proxywidget.show()
            
    def init_ui(self):
        print "Initialising ui"
        self.container = QWidget()
        self.webview = QWebView(self.container)
        self.container.setLayout(QVBoxLayout())
        self.container.layout().addWidget(self.webview)
        
        print "Adding webview to scene"
        uism = r.getUiSceneManager()
        uiprops = r.createUiWidgetProperty(2) # SceneWidget
        uiprops.widget_name_ = 'LoadURL'
        self.proxywidget = r.createUiProxyWidget(self.container, uiprops)
        if not uism.AddProxyWidget(self.proxywidget):
            r.logError('LoadURLHandler: Adding the ProxyWidget to the scene failed.')
            return

    def on_exit(self):
        r.logInfo('LoadURLHandler exiting..')
        if self.proxywidget is not None:
            self.proxywidget.hide()
            uism = r.getUiSceneManager()
            uism.RemoveProxyWidgetFromScene(self.proxywidget)

def loadurl(urlstring):
    url = PythonQt.QtCore.QUrl(urlstring)
    if instance is not None:
        instance.loadurl(url)
    else:
        print "ERROR: loadurl handler wasn't there!"

