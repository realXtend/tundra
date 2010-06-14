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

#for camera rotating
import PythonQt.QtGui
from PythonQt.QtGui import QQuaternion as Quat
from PythonQt.QtGui import QVector3D as Vec

PORT = 28008

#__file__ doesn't work in embedded context, but os.getcwd() helps
PATH = os.getcwd() + "/pymodules/webserver" + "/screenshot/"
#print "WEB PATH for images:", PATH

class WebServer(Server):
    """this is the component that autoload registers to the viewer"""
    def __init__(self):
        Server.__init__(self, "0.0.0.0:%d" % PORT) #"localhost", PORT)
        self + WebController() + Static(docroot=PATH)

    def on_exit(self):
        self.stop()

html = """\
<html>
 <head>
  <title>Naali web ui</title>
 </head>
 <body>
  <h1>Naali</h1>
  <form action="camcontrol" method="POST">

  <p>rotate:<br/>
  <input type="submit" name="rotate" value="10"/> 
  <input type="submit" name="rotate" value="-10"/> 
  </p>

  <p>move:<br/
  <input type="submit" name="move" value="+1"/><br>
  <input type="submit" name="move" value="-1"/> 
  </p>

  </form>
 </body>
</html>"""


class WebController(Controller):                        
    def index(self):
        return html #"Hello World!"

    def camcontrol(self, rotate=None, move=None):
        camid = r.getCameraId()
        #print "CAM:", camid
        cament = r.getEntity(camid)
        p = cament.placeable
        #print p.Position, p.Orientation

        if rotate is not None:
            ort = p.Orientation
            rot = Quat.fromAxisAndAngle(Vec(0, 1, 0), float(rotate))
            ort *= rot
            p.Orientation = ort

        if move is not None:
            pos = p.Position
            pos += Vec(float(move), 0, 0)
            p.Position = pos

        #return "%s, %s" % (p.Position, p.Orientation)
        return html
        
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
