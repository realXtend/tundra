from PythonQt.QtGui import QPushButton

class PyPushButton(QPushButton):
    def __init__(self, *args):
        QPushButton.__init__(self, args)
        
    def lineValueChanged(self):
        if not self.enabled:
            self.setEnabled(True)