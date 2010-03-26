"""after http://groups.google.com/group/pyglet-users/web/twistedpyglet.tgz,
info about that and a proposal to add something like this to pyglet upstream
in http://markmail.org/message/tm6r3cyhrp3dns4r

that work added carbon support for mac, here am doing the same for win32.
"""

# System imports
import sys, pyglet

import ctypes

from pyglet.app import windows, BaseEventLoop
from pyglet.window.win32 import _user32, types, constants

class Win32EventLoopOverride(pyglet.app.win32.Win32EventLoop):
    def pump(self):
        self._setup()

        self._timer_proc = types.TIMERPROC(self._timer_func)
        self._timer = timer = _user32.SetTimer(0, 0, 0, self._timer_proc)
        self._polling = False
        self._allow_polling = True
        msg = types.MSG()
        
        self.dispatch_event('on_enter')

        while not self.has_exit:
            if self._polling:
                while _user32.PeekMessageW(ctypes.byref(msg), 
                                           0, 0, 0, constants.PM_REMOVE):
                    _user32.TranslateMessage(ctypes.byref(msg))
                    _user32.DispatchMessageW(ctypes.byref(msg))
                self._timer_func(0, 0, timer, 0)
                yield 1 #to make a non-blocking version XXX
            else:
                _user32.GetMessageW(ctypes.byref(msg), 0, 0, 0)
                _user32.TranslateMessage(ctypes.byref(msg))
                _user32.DispatchMessageW(ctypes.byref(msg))
            
                # Manual idle event
                msg_types = \
                    _user32.GetQueueStatus(constants.QS_ALLINPUT) & 0xffff0000
                if (msg.message != constants.WM_TIMER and
                    not msg_types & ~(constants.QS_TIMER<<16)):
                    self._timer_func(0, 0, timer, 0)
                yield 2 #to make a non-blocking version XXX

def enable():
    pyglet.app.event_loop = Win32EventLoopOverride()
