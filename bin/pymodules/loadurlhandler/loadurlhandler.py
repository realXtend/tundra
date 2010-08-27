import rexviewer as r
import PythonQt

from circuits import Component

from PythonQt.QtUiTools import QUiLoader
from PythonQt.QtCore import QUrl, QFile, QSize
from PythonQt.QtGui import QWidget, QIcon
from PythonQt.QtWebKit import QWebView, QWebPage

instance = None

class LoadURLHandler(Component):
    def __init__(self):
        Component.__init__(self)
        self.proxywidget = None
        self.container = None
        
        self.webview = None
        self.lineedit_address = None
        self.progress_bar = None
        self.progress_label = None
        self.button_back = None
        self.button_forward = None
        self.button_go = None
        self.button_refresh = None
        self.button_stop = None
        
        global instance
        instance = self

    def load_url(self, qurl):
        if self.webview == None and self.container == None and self.proxywidget == None:
            self.init_ui()
        self.webview.load(qurl)
        self.set_address(qurl.toString())
        if not self.proxywidget.isVisible():
            self.proxywidget.show()
            
    def init_ui(self):
        loader = QUiLoader()
        
        # Container widget
        self.container = ui = loader.load(QFile("pymodules/loadurlhandler/webbrowser.ui"))
        
        # Webview
        self.webview = QWebView(self.container)
        self.webview.page().setLinkDelegationPolicy(QWebPage.DelegateAllLinks)
        
        self.webview.connect("loadStarted()", self.on_load_start)
        self.webview.connect("loadFinished(bool)", self.on_load_stop)
        self.webview.connect("loadProgress(int)", self.on_load_progress)
        self.webview.connect("linkClicked(QUrl)", self.link_clicked)
        
        layout = ui.findChild("QVBoxLayout", "verticalLayout_Container")
        layout.insertWidget(2, self.webview)
        
        # Buttons
        ui.findChild("QPushButton", "pushButton_ClearCookie").hide()
        self.button_back = ui.findChild("QPushButton", "pushButton_Back")
        self.button_forward = ui.findChild("QPushButton", "pushButton_Forward")
        self.button_refresh = ui.findChild("QPushButton", "pushButton_Refresh")
        self.button_stop = ui.findChild("QPushButton", "pushButton_Stop")
        self.button_go = ui.findChild("QPushButton", "pushButton_Go")        

        self.button_back.setIcon(QIcon("./data/ui/images/arrow_left_48.png"));
        self.button_back.setIconSize(QSize(20, 20));
        self.button_forward.setIcon(QIcon("./data/ui/images/arrow_right_48.png"));
        self.button_forward.setIconSize(QSize(20, 20));
        self.button_refresh.setIcon(QIcon("./data/ui/images/refresh_48.png"));
        self.button_refresh.setIconSize(QSize(20, 20));
        self.button_stop.setIcon(QIcon("./data/ui/images/cross_48.png"));
        self.button_stop.setIconSize(QSize(20, 20));
        self.button_stop.setEnabled(False);
        self.button_go.setIcon(QIcon("./data/ui/images/arrow_right_green_48.png"));
        self.button_go.setIconSize(QSize(20, 20));

        self.button_back.connect("clicked()", self.on_back)
        self.button_forward.connect("clicked()", self.on_forward)
        self.button_refresh.connect("clicked()", self.on_refresh)
        self.button_stop.connect("clicked()", self.on_stop)
        self.button_go.connect("clicked()", self.go_to_linedit_url)
        
        # Address bar
        self.lineedit_address = ui.findChild("QComboBox", "comboBox_Address")
        self.lineedit_address.lineEdit().connect("returnPressed()", self.go_to_linedit_url)
        
        # Progress bar and label
        self.progress_bar = ui.findChild("QProgressBar", "progressBar_Status")
        self.progress_bar.hide()
        
        self.progress_label = ui.findChild("QLabel", "label_Status")
        
        # Add to scene
        uism = r.getUiSceneManager()
        self.proxywidget = r.createUiProxyWidget(self.container)
        self.proxywidget.setWindowTitle("Naali Web Browser")
        self.proxywidget.connect("Visible(bool)", self.vibibility_changed)
        if not uism.AddWidgetToScene(self.proxywidget):
            r.logError('LoadURLHandler: Adding the ProxyWidget to the scene failed.')

    def vibibility_changed(self, visible):
        if not visible:
            self.webview.load(QUrl("about:blank"))

    def go_to_linedit_url(self):
        try:
            url = str(self.lineedit_address.currentText)
        except:
            self.lineedit_address.setEditText("invalid url") # should prolly support unicode but meh for now...
            return
        if url.startswith("http://") == False and url.startswith("https://") == False:
            url = "http://" + url
        self.load_url(QUrl(url))
        
    def on_back(self):
        self.webview.back()
        
    def on_forward(self):
        self.webview.forward()
        
    def on_refresh(self):
        self.webview.reload()
        
    def on_stop(self):
        self.webview.stop()
        
    def on_load_start(self):
        self.progress_bar.show()
        self.button_stop.setEnabled(True)
        self.set_address(self.webview.url.toString())
        
    def link_clicked(self, qurl):
        self.set_address(qurl.toString())
        self.webview.load(qurl)

    def on_load_stop(self, ok):
        self.progress_bar.hide()
        self.button_stop.setEnabled(False);
        if ok:
            self.progress_label.setText("Done")
        else:
            self.progress_label.setText("Errors while loading")
        self.set_address(self.webview.url.toString())

    def on_load_progress(self, progress):
        self.progress_bar.setValue(progress)
        self.progress_label.setText("Loading... " + str(progress) + " %")
        
    def set_address(self, url):
        if self.lineedit_address.lineEdit().text != url:
            self.lineedit_address.lineEdit().setText(url)
    
    def on_logout(self, id):
        if self.webview != None:
            self.webview.stop()
        if self.proxywidget != None:
            self.proxywidget.hide()
            
    def on_exit(self):
        if self.webview != None:
            self.webview.stop()
        if self.proxywidget is not None:
            self.proxywidget.hide()
            uism = r.getUiSceneManager()
            uism.RemoveWidgetFromScene(self.proxywidget)

def loadurl(urlstring):
    url = QUrl(urlstring)
    if instance is not None:
        instance.load_url(url)
    else:
        print "ERROR: loadurl handler wasn't there!"

