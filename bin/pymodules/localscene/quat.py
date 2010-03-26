try:
    import PythonQt
except:
    class MockQuaternion:
        def __init__(self, x, y, z, w):
            self.x = x
            self.y = y
            self.z = z
            self.w = w
    Quaternion = MockQuaternion
else:
    from PythonQt.QtGui import QQuaternion as Quaternion
