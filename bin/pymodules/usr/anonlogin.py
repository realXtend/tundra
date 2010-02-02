from circuits import Component
import rexviewer as r
import urllib2

from PythonQt.QtGui import QGroupBox, QPushButton
from PythonQt.QtUiTools import QUiLoader
from PythonQt.QtCore import QFile

ACCOUNTURI = "http://www.playsign.fi/engine/rex/anonuser"
DEMOLOGIN = "http://world.realxtend.org:9000"

UIFILELOCATION = "data/ui/demoworld/DemoLoginWidget.ui"

class AnonLogin(Component):
    def __init__(self):
        Component.__init__(self)

        self.loader = QUiLoader()
        self.uifile = QFile(UIFILELOCATION)
        self.demologinwidget = self.loader.load(self.uifile)
        self.demologinwidget.fishWorldButton.connect('clicked()', self.do_fistworld_login)
        
        uism = r.getUiSceneManager()
        uism.SetDemoLoginWidget(self.demologinwidget)      

    def do_fistworld_login(self):
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
        self.demologinwidget = None
        self.uifile = None
        self.loader = None
