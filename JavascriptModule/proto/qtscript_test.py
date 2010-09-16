"""a pyqt test app that embeds javascript using QtScript,
defines qt properties and slots in py, and exposes them to js"""

import sys
from PyQt4 import QtGui, QtCore
from PyQt4.QtScript import QScriptEngine, QScriptValue
from PyQt4.QtCore import QObject, pyqtSignature, pyqtProperty, QVariant, QTimer


ps1 = ">>> "
ps2 = "... "

def printhello():
    print "Hello."

app = QtGui.QApplication(sys.argv)
window = QtGui.QWidget()
# Create our main window using a plain QWidget.
window.setWindowTitle("Signals")
# Set our window's title as "Signals".
button = QtGui.QPushButton("Press", window)
# Create, with "Press" as its caption,
# a child button in the window.
# By specifying a parent object,
# this new widget is automatically added to the same.
button.resize(200, 40)
# Resize our button to (200, 40) -> (X, Y)

"""old style signals, works in pyqt 4.4"""
#button.connect(button, QtCore.SIGNAL("clicked()"), printhello) #QtCore.SLOT("quit()"))

#new style signals in 4.5, must upgdate to get these
button.clicked.connect(printhello)

# Connect the button's click signal to the QApplication's quit() slot.
window.show()
# Show our window.

class Python(QObject):
    def __init__(self):
        QObject.__init__(self)
        self.setObjectName("python")
        # Does not work as expected :(
        self.setProperty("app", QVariant(self))
        self.t = QTimer(self)
        self.t.setObjectName("timer")

    @pyqtSignature("QString")
    def hello(self, name):
        print "Hello,", name

    def get_test(self):
        return 123

    test = pyqtProperty("int", get_test)

engine = QScriptEngine()
engine.evaluate("function dir(obj) { for(o in obj) print(o); }")

py = Python()
spy = engine.newQObject(py)
engine.globalObject().setProperty("python", spy)

#app.exec_()

print "Ctrl+D to quit"
prompt = ps1
code = ""
while True:
    app.processEvents()
    line = raw_input(prompt)
    if not line.strip():
        continue
    code = code + line + "\n"
    if engine.canEvaluate(code):
        result = engine.evaluate(code)
        if engine.hasUncaughtException():
            bt = engine.uncaughtExceptionBacktrace()
            print "Traceback:"
            print "\n".join(["  %s" % l for l in list(bt)])
            print engine.uncaughtException().toString()
        else:
            print result.toString()
        code = ""
        prompt = ps1
    else:
        prompt = ps2
