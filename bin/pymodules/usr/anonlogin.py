from circuits import Component
import rexviewer as r
import urllib2

from PythonQt.QtUiTools import QUiLoader
from PythonQt.QtCore import QFile, QSize
#from PythonQt.QtGui import QGroupBox, QPushButton
from PythonQt.QtUiTools import QUiLoader
from PythonQt.QtCore import QFile

ACCOUNTURI = "http://www.playsign.fi/engine/rex/anonuser"
DEMOLOGIN = "http://world.realxtend.org:9000"
UIFILELOC = "data/ui/demoworld/DemoLoginWidget.ui"

class AnonLogin(Component):
    def __init__(self):
        Component.__init__(self)

        loader = QUiLoader()
        uifile = QFile(UIFILELOC)
        self.widget = loader.load(uifile)
        self.widget.fishWorldButton.connect('clicked()', self.do_login)
        
        #self.group = QGroupBox()
        #self.pushbut = QPushButton(self.group)
        #self.pushbut.text = "Anon login"
        #self.pushbut.connect('clicked()', self.dologin)

        uism = r.getUiSceneManager()
        uism.SetDemoLoginWidget(self.widget)      

    def do_login(self):
        urlfile = urllib2.urlopen(ACCOUNTURI)
        s = urlfile.read()
        s = s.strip()
        r.logInfo("AnonLogin got account info: %s" % s)
        try:
            firstname, lastname = s.split() #just to check that it has the right format
        except ValueError:
            r.logInfo("Invalid data from anon login cred service: %s" % s)
        else:
            user = "%s %s" % (firstname, lastname)
            pwd = "anonpwd"
            r.startLoginOpensim(user, pwd, DEMOLOGIN)

    def on_exit(self): 
        #for live reloading
        self.widget.hide()
        self.widget = None
