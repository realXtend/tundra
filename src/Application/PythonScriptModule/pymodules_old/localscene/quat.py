#try:
#    import PythonQt
#except:
class MockQuaternion:
        #def __init__(self, x, y, z, w):
    def __init__(self, w, x, y, z):
        self.w = w
        self.x = x
        self.y = y
        self.z = z
Quaternion = MockQuaternion
#else:
#    from PythonQt.QtGui import QQuaternion as Quaternion
