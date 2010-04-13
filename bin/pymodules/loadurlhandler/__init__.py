import rexviewer as r
import PythonQt

def loadurl(urlstring):
    print urlstring

    uism = r.getUiSceneManager()
    uiprops = r.createUiWidgetProperty(2) #2 is scene widget
    uiprops.widget_name_ = "LoadURL"
    #uiprops.my_size_ = QSize(width, height) #not needed anymore, uimodule reads it

    wv = PythonQt.QtWebKit.QWebView()

    proxywidget = r.createUiProxyWidget(wv, uiprops)
    if not uism.AddProxyWidget(proxywidget):
        print "Adding the ProxyWidget to the scene failed."
        return

    proxywidget.show()

    url = PythonQt.QtCore.QUrl(urlstring)
    wv.load(url)

    wv.show()
    print wv
    
    r.loadurlproxy = proxywidget
    r.loadurlvw = wv
