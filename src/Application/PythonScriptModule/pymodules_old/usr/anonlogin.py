from circuits import Component
import rexviewer as r
import urllib2

from PythonQt.QtUiTools import QUiLoader
from PythonQt.QtCore import QFile

UIFILELOC = "data/ui/demoworld/DemoLoginWidget.ui"

class WorldInfo:
    passwd = "anonpwd"

class BeneaththewavesInfo(WorldInfo):
    accounturi = "http://www.playsign.fi/engine/rex/anonuser"
    loginuri = "http://world.realxtend.org:9000"
    butname = "fishWorldButton"

class CarshowInfo(WorldInfo):
    accounturi = "http://www.playsign.fi/engine/rex/anonuser"
    loginuri = "http://home.hulkko.net:9007"
    butname = "carWorldButton"

class AnonLogin(Component):
    WORLDS = [BeneaththewavesInfo, CarshowInfo]

    def __init__(self):
        Component.__init__(self)

        loader = QUiLoader()
        uism = r.getUiSceneManager()

        uifile = QFile(UIFILELOC)
        self.widget = loader.load(uifile)
        uism.SetDemoLoginWidget(self.widget)

        def make_worldlogin(w):
            def worldlogin():
                self.login(w)
            return worldlogin

        for w in self.WORLDS:
            but = getattr(self.widget, w.butname)
            but.connect('clicked()', make_worldlogin(w))       

    def login(self, worldinfo):
        r.logInfo("Anon login with %s - %s " % (worldinfo.loginuri, worldinfo.accounturi))
        urlfile = urllib2.urlopen(worldinfo.accounturi)
        s = urlfile.read()
        s = s.strip()
        r.logInfo("AnonLogin got account info: %s" % s)
        try:
            firstname, lastname = s.split() #just to check that it has the right format
        except ValueError:
            r.logInfo("Invalid data from anon login cred service: %s" % s)
        else:
            user = "%s %s" % (firstname, lastname)
            r.startLoginOpensim(user, worldinfo.passwd, worldinfo.loginuri)

    def on_exit(self): 
        #for live reloading
        self.widget.hide()
        self.widget = None
