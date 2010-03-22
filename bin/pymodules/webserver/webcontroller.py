"""a non-blocking, non-threaded non-multiprocessing circuits web server"""

import rexviewer as r

try:
    import circuits
except ImportError: #not running within the viewer, but testing outside it
    import sys
    sys.path.append('..')

from circuits.web import Controller, Server, Static
import datetime
import os

PORT = 28008

#__file__ doesn't work in embedded context, but os.getcwd() helps
PATH = os.getcwd() + "/pymodules/webserver" + "/screenshot/"
#print "WEB PATH for images:", PATH

class WebServer(Server):
    """this is the component that autoload registers to the viewer"""
    def __init__(self):
        Server.__init__(self, PORT) #"localhost", PORT)
        self + WebController() + Static(docroot=PATH)

class WebController(Controller):                        
    def index(self):
        return "Hello World!"
        
    def render(self):
        timestr = datetime.datetime.today().isoformat()
        imgname = "image.png"
        r.takeScreenshot(PATH, imgname)
        
        return """
        <h1>Realxtend Naali viewer</h1>
        <h2>at %s</h2>
        <img src="%s"/>
        """ % (timestr, imgname)

#~ if __name__ == '__main__':
    #~ while 1:
        #~ print ".",
