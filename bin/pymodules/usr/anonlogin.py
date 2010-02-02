from circuits import Component
import rexviewer as r
import urllib2

from PythonQt.QtGui import QGroupBox, QPushButton

ACCOUNTURI = "http://www.playsign.fi/engine/rex/anonuser"
DEMOLOGIN = "http://world.realxtend.org:9000"

class AnonLogin(Component):
    def __init__(self):
        #r.logInfo("AnonLogin initing..")
        Component.__init__(self)
        #uism = r.getUiSceneManager()
        self.group = QGroupBox()

        self.pushbut = QPushButton(self.group)
        self.pushbut.text = "Anon login"
        self.pushbut.connect('clicked()', self.dologin)

        self.group.show()
        #r.logInfo("AnonLogin showed?")

    def dologin(self):
        #r.logInfo("Login :)")
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
        self.group.hide()
        self.group = None
        self.pushbut = None
