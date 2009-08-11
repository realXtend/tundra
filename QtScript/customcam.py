from PyQt4 import QtCore
from PyQt4.QtCore import QObject

class SceneEntity(QObject):
    """a dummy entity that's in a scene and which updates"""
    update = QtCore.pyqtSignal()
    
    def __init__(self):
        QObject.__init__(self)
        
        self.pos = 0.0
        
        self.update.connect(self.default_update)
        
    def default_update(self):
        pass
        
class Avatar(SceneEntity):
    def default_update(self):
        self.pos += 0.1
        
av = Avatar()

class Camera(SceneEntity):    
    def default_update(self):
        self.pos = av.pos - 1.0
        
cam = Camera()

def update():
    av.update.emit()
    cam.update.emit()

def checkcamdist(name, target):
    d = av.pos - cam.pos
    print "CHECK CAM DIST: cam distance from av is %f (posses: %f - %f)" % (d, av.pos, cam.pos)
    assert (target - 0.01) < d < (target + 0.01) #with an epsilon of 0.01 for float inaccuracy
    print "SUCCESS: %s cam is keeping cam within %f from av" % (name, target)    

def test_custom_cam():
    """let's say the default behaviour is that cam follows at -1.0"""
    for _ in range(5):
        update()
        checkcamdist("default", 1.0)
    
    """now let's change to a qtscripted behaviour that keeps at 0.5"""
    #cam.update.connect(customcam)
    update()
    checkcamdist("custom", 0.5)
    
if __name__ == '__main__':
    test_custom_cam()
    
    
    