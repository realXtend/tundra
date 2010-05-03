#!/usr/bin/python

import rexviewer as r
from circuits import Component

from PythonQt.QtUiTools import QUiLoader
from PythonQt.QtCore import QFile

#import localscene.window
import window
import loader
import dotscenemanager

from window import LocalSceneWindow as LCwindow


class LocalScene(Component):
    def __init__(self):
        print "__init__"
        Component.__init__(self)
        #self.window = LocalSceneWindow(self)
        self.window = LCwindow(self)
        self.xsift = 127
        self.ysift = 127
        self.zsift = 25
        self.xscale = 1
        self.yscale = 1
        self.zscale = 1
        self.dotScene = None
        self.dsManager = None
        self.flipZY = True
        pass

    def loadScene(self, filename):
        print "loading: ", filename
        #file = QFile(filename)
        self.dotScene, self.dsManager = loader.load_dotscene(filename)

    def unloadScene(self):
        #loader.unload()
        pass
        
    def publishScene(self):
        print "publishing scene"
        
    def setxpos(self, x):
        self.xsift = x
        if(self.dsManager!=None):
            self.dsManager.setPosition(self.xsift, self.ysift, self.zsift)
        
    def setypos(self, y):
        self.ysift = y
        if(self.dsManager!=None):
            self.dsManager.setPosition(self.xsift, self.ysift, self.zsift)

    def setzpos(self, z):
        self.zsift = z
        if(self.dsManager!=None):
            self.dsManager.setPosition(self.xsift, self.ysift, self.zsift)

    def setxscale(self, x):
        self.xscale = x
        if(self.dsManager!=None):
            self.dsManager.setScale(self.xscale, self.yscale, self.zscale)

    def setyscale(self, y):
        self.yscale = y
        if(self.dsManager!=None):
            self.dsManager.setScale(self.xscale, self.yscale, self.zscale)

    def setzscale(self, z):
        self.zscale = z
        if(self.dsManager!=None):
            self.dsManager.setScale(self.xscale, self.yscale, self.zscale)        

    def checkBoxZYToggled(self, enabled):
        self.flipZY = enabled
        if(self.dsManager!=None):
            self.dsManager.setFlipZY(enabled, self.xsift, self.ysift, self.zsift, self.xscale, self.yscale, self.zscale)
        pass
        
    def on_exit(self):
        print "local scene exit"
        r.logInfo("Local Scene exiting...")
        self.window.on_exit()  

    def on_hide(self, shown):
        print "on hide"
        
    def update(self, time):
        #print "here", time
        pass

    def on_logout(self, id):
        r.logInfo("Local scene Logout.")