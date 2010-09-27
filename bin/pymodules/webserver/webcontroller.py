"""a non-blocking, non-threaded non-multiprocessing circuits web server"""

import time #timestamping images
import datetime #showing human readable time on render page
import os

import rexviewer as r
import naali

try:
    import circuits
except ImportError: #not running within the viewer, but testing outside it
    import sys
    sys.path.append('..')

from circuits.web import Controller, Server, Static

#for camera rotating
import PythonQt.QtGui
from PythonQt.QtGui import QQuaternion as Quat
from PythonQt.QtGui import QVector3D as Vec

import mathutils as mu

PORT = 28008

#__file__ doesn't work in embedded context, but os.getcwd() helps
OWNPATH = os.getcwd() + "/pymodules/webserver/" 
SHOTPATH = OWNPATH + "screenshot/"
#print "WEB PATH for images:", PATH

class WebServer(Server):
    """this is the component that autoload registers to the viewer"""
    def __init__(self):
        Server.__init__(self, "0.0.0.0:%d" % PORT) #"localhost", PORT)
        self + WebController() + Static(docroot=SHOTPATH)

#the first version with relative controls to the cam
relhtml = """\
<html>
 <head>
  <title>Naali web ui</title>
 </head>
 <body>
  <h1>Naali</h1>

  <form action="camcontrol" method="GET">
  <p>rotate:<br/>
  <input type="submit" name="rotate" value="10"/> 
  <input type="submit" name="rotate" value="-10"/> 
  </p>

  <p>move:<br/
  <input type="submit" name="move" value="+1"/><br>
  <input type="submit" name="move" value="-1"/> 
  </p>
  </form>

  <img src="%s"/>

 </body>
</html>"""

#second version where webui gives absolute pos&ort for the cam, so each user has own on client side
abshtml = open(OWNPATH + "webui.html").read()

def save_screenshot():
    rend = naali.renderer
    rend.HideCurrentWorldView()
    rend.Render()
    imgname = "image-%s.png" % time.time()
    r.takeScreenshot(SHOTPATH, imgname)
    rend.ShowCurrentWorldView()
    baseurl = "/"
    #baseurl = "http://www.playsign.fi:28080/"
    return baseurl, imgname

class WebController(Controller):                        
    def index(self):
        return self.serve_file(OWNPATH + "naali.html")

    def hello(self):
        return "Hello World!"

    def camcontrol(self, rotate=None, move=None):
        cament = naali.getCamera()
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
            
        baseurl, imgname = save_screenshot()
        imgurl = baseurl + imgname
        
        #return "%s, %s" % (p.Position, p.Orientation)
        return relhtml % imgurl

    def render(self, camposx=None, camposy=None, camposz=None, camang=None):
#, camortx=None, camorty=None, camortz=None, camortw=None):
        cament = naali.getCamera()
        p = cament.placeable

        if camposx is not None:
            pos = Vec(*(float(v) for v in [camposx, camposy, camposz]))
            p.Position = pos

        if camang is not None:
            ort = p.Orientation
            start = Quat(0, 0, -0.707, -0.707)
            rot = Quat.fromAxisAndAngle(Vec(0, 1, 0), -float(camang))
            new = start * rot
            p.Orientation = new

        #if camortx is not None:
        #    ort = Quat(*(float(v) for v in [camortw, camortx, camorty, camortz]))
        #    p.Orientation = ort

        #return str(p.Position), str(p.Orientation) #self.render1()
        baseurl, imgname = save_screenshot()
        imgurl = baseurl + imgname

        pos = p.Position
        ort = p.Orientation
        #vec, ang = toAngleAxis(p.Orientation)
        #print vec, ang
        euler = mu.quat_to_euler(ort)
        ang = euler[0]
        if ang < 0:
            ang = 360 + ang

        return abshtml % (imgurl,
                          ang,
                          pos.x(), pos.y(), pos.z()
                          #ort.scalar(), ort.x(), ort.y(), ort.z(),
                          )

    def _renderimgurl(self, camposx=None, camposy=None, camposz=None, camortx=None, camorty=None, camortz=None, camortw=None):
        cament = naali.getCamera()
        p = cament.placeable
        orgpos = Vec(0, 0, 0)
        orgort = Quat(1, 0, 0, 0)

        if camposx is not None:
            pos = Vec(*(float(v) for v in [camposx, camposy, camposz]))
            p.Position = pos

        if camortx is not None:
            ort = Quat(*(float(v) for v in [camortw, camortx, camorty, camortz]))
            p.Orientation = ort

        baseurl, imgname = save_screenshot()

        p.Position = orgpos
        p.Orientation = orgort
        return baseurl, imgname

    def renderimgurl(self, camposx=None, camposy=None, camposz=None, camortx=None, camorty=None, camortz=None, camortw=None):
        baseurl, imgname = self._renderimgurl(camposx, camposy, camposz, camortx, camorty, camortz, camortw)
        return baseurl + imgname        

    def renderimg(self, camposx=None, camposy=None, camposz=None, camortx=None, camorty=None, camortz=None, camortw=None):
        _, imgname = self._renderimgurl(camposx, camposy, camposz, camortx, camorty, camortz, camortw)
        return self.serve_file(SHOTPATH + imgname)
        
    def render1(self, campos=None, camort=None):
        timestr = datetime.datetime.today().isoformat()
        baseurl, imgname = save_screenshot()
        imgurl = baseurl + imgname
        
        return """
        <h1>Realxtend Naali viewer</h1>
        <h2>at %s</h2>
        <img src="%s"/>
        """ % (timestr, imgurl)

#~ if __name__ == '__main__':
    #~ while 1:
        #~ print ".",
