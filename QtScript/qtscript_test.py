"""a pyqt test app that embeds javascript using QtScript,
defines qt properties and slots in py, and exposes them to js"""

import sys
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4.QtScript import QScriptEngine, QScriptValue


ps1 = ">>> "
ps2 = "... "

app = QApplication(sys.argv)

class Python(QObject):
    def __init__(self):
        QObject.__init__(self)
        self.setObjectName("python")
        # Does not work as expected :(
        self.setProperty("app", QVariant(qApp))
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

print "Ctrl+D to quit"
prompt = ps1
code = ""
while True:
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
