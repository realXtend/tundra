#!/usr/bin/python
'''
by HartsAntler bhartsho@yahoo.com
License: BSD
Version: 0.0.5

Dev-Notes:
	Feb9th: Qt and SDL versions working
	Feb10th:
		. qt works nice, faster
		. finalized C hacks, published source code
		. making compatible with Realxtend Naali (Tundra)
	Feb11th:
		. big speed up with viewer dual threaded
			seems like the ctypes-threading trick is do not lock.acquire/release in threadloop,
			locking is done in callbacks on data, if data is a pointer it must be copied in the mainthread.
		. threading server smooths out data flow
		. fixed threading issues in server, clean shutdown
		. server and client work with Tundra!
	Feb12th:
		. fake mouse click for windows
	Feb13th:
		. enable/disable checkbox
	Feb16th:
		. building dll's for windows

	May11th 2011:
		. updated for new RPythonic-API-0.3.6

	May 19th:
		. passes RPythonic-API-0.3.6 final

Platforms:
	tested on ubuntu10.10
	(works on any platform, just requires libvncserver libvncclient to be compiled)
	

Requires:
	ctypeslibs: wrappers for libvncserver and libvncclient
	patched libvncserver and libvncclient
	PyQt4	( apt-get install python-qt4 )

How to Patch:

//add to libvncclient/vncviewer.c
void rfbSetClientCallback(rfbClient* client, const char* type, void * callback ) {
	if (strcmp(type, "GotFrameBufferUpdate" )==0) { 
		rfbClientLog( "setting got frame callback \n" );
		client->GotFrameBufferUpdate = callback;
	} else if (strcmp(type, "MallocFrameBuffer" )==0) {
		rfbClientLog( "setting malloc callback \n" );
		client->MallocFrameBuffer = callback;
	} else if (strcmp(type, "HandleKeyboardLedState" )==0) {
		rfbClientLog( "setting keyboard callback \n" );
		client->HandleKeyboardLedState = callback;
	}
}

//add to libvncserver/main.c
char* rfbMallocFrameBuffer( rfbScreenInfoPtr screen, int width, int height, int bpp ) {
	screen->frameBuffer=(char*)malloc( width*height*bpp);
	return screen->frameBuffer;  /* must return because frameBuffer.contents from ctypes is NULL */
}
void rfbSetMouseCallback( rfbScreenInfoPtr screen, void* callback ) {
	screen->ptrAddEvent = callback;
}
void rfbSetKeyboardCallback( rfbScreenInfoPtr screen, void* callback ) {
	screen->kbdAddEvent = callback;
}
void rfbSetNewClientCallback( rfbScreenInfoPtr screen, void* callback ) {
	screen->newClientHook = callback;
}
void rfbSetClientGoneCallback( rfbClientPtr client, void* callback ) {
	client->clientGoneHook = callback;
}
////////////////////////// end of patch


Extra Hacks:
	1. CMakeLists.txt:32		(may only be required on linux32?)
		## building libvncclient.so from source, CMakeLists.txt:32 must remove TLS
		#if(GNUTLS_FOUND)	# not compatible with ctypes?
		#  set(LIBVNCSERVER_WITH_CLIENT_TLS 1)
		#endif(GNUTLS_FOUND)
		-- ctypes error --
		  File "/usr/lib/python2.6/ctypes/__init__.py", line 353, in __init__
		    self._handle = _dlopen(self._name, mode)
		OSError: /home/brett/libvnc-build/libvncclient.so: undefined symbol: gnutls_global_init

Win32 Compile Notes:
	Autotools Way - thanks Christian Beier!
		wget http://www.dlldump.com/cgi-bin/testwrap/downloadcounts.cgi?rt=count&path=dllfiles/W/ws2_32.dll
		sudo cp ws2_32.dll /usr/i586-mingw32msvc/lib/libws2_32.dll
		cd libvncserver/
		CPPFLAGS=-DWINVER=0x0501 LDFLAGS=-no-undefined ./autogen.sh --host=i586-mingw32msvc --with-gnutls=no --with-gcrypt=no --with-sdl=no
		make -j4
		cp libvncserver/.libs/libvncserver-0.dll libvncserver.dll
		cp libvncclient/.libs/libvncclient-0.dll libvncclient.dll


	THIS WONT WORK:
		create Toolchain-mingw32.cmake
			# the name of the target operating system
			SET(CMAKE_SYSTEM_NAME Windows)
			# which compilers to use for C and C++
			SET(CMAKE_C_COMPILER i586-mingw32msvc-gcc)
			SET(CMAKE_CXX_COMPILER i586-mingw32msvc-g++)
			# here is the target environment located
			SET(CMAKE_FIND_ROOT_PATH  /usr/i586-mingw32msvc /home/brett/mingw-install )
			# adjust the default behaviour of the FIND_XXX() commands:
			# search headers and libraries in the target environment, search 
			# programs in the host environment
			set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
			set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
			set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
		cd
		mkdir mingw-install
		apt-get install mingw32
		mkdir build
		cd build
		cmake -DCMAKE_TOOLCHAIN_FILE=~/Toolchain-mingw32.cmake -DCMAKE_INSTALL_PREFIX=/home/brett/mingw-install ../libvncserver/


'''
BPP = 4	# bytes per pixel
HALFSIZE = True

import os,sys, time, ctypes

PyQt4 = gtk = PythonQt = None

naali = None
try:		# see if we are inside Naali
	#import rexviewer	# deprecated
	import naali
	import circuits
	import circuits.Component
	import PythonQt
	from PythonQt import QtGui, QtCore
	from PythonQt.QtCore import Qt, QSize
	from PythonQt.QtGui import QPixmap, QApplication
	PyQt4 = PythonQt
except:
	try:
		import PyQt4
		from PyQt4 import QtGui, QtCore
		from PyQt4.QtCore import Qt, QSize
		from PyQt4.QtGui import QPixmap, QApplication
		app = QApplication(sys.argv)
	except:
		try: import PythonQt
		except:
			import gtk
			import gtk.gdk, numpy


if naali:
	#if os.path.split( os.path.abspath('.') )[-1] != 'pymodules':
	#	sys.path.append( os.path.abspath('pymodules') )
	#	sys.path.append( os.path.abspath('pymodules/lib') )
	#	sys.path.append( os.path.abspath('pymodules/DLLs') )
	#	os.chdir('pymodules')

	class NaaliServerHandler(circuits.Component):
		def __init__(self):
			circuits.Component.__init__(self)
			self.server = Server()
		@circuits.handler("update")
		def update(self, deltatime):
			#print('delta', deltatime)
			if self.server.active:
				self.server.iterate()

	class NaaliClientHandler(circuits.Component):
		def __init__(self):
			circuits.Component.__init__(self)
			self.client = Client()
		@circuits.handler("update")
		def update(self, deltatime):
			#print('delta', deltatime)
			if self.client.active:
				self.client.iterate()


if '..' not in sys.path: sys.path.append( '..' )
import rpythonic
_rfb_ = rpythonic.module( 'vncserver' )
_rfbclient_ = rpythonic.module( 'vncclient' )
assert _rfb_
assert _rfbclient_


if PyQt4:
	class VncServerWindow( QtGui.QWidget ):
		def closeEvent( self, ev ):
			self._server.exit()
			ev.accept()
	class _VNCServer_antialias(QtGui.QCheckBox):
		def mouseReleaseEvent(self,ev):
			QtGui.QCheckBox.mouseReleaseEvent(self,ev)
			on = self.isChecked(); print( on )
			self._server._antialias = on
	class _VNCServer_allow_remote(QtGui.QCheckBox):
		def mouseReleaseEvent(self,ev):
			QtGui.QCheckBox.mouseReleaseEvent(self,ev)
			on = self.isChecked(); print( on )
			self._server._allow_mouse_warp = on
			self._server._allow_mouse_click = on
	class _VNCServer_halfsize(QtGui.QCheckBox):
		def mouseReleaseEvent(self,ev):
			QtGui.QCheckBox.mouseReleaseEvent(self,ev)
			on = self.isChecked(); print( on )
			self._server.HALFSIZE = on

	class _VNCServer_start(QtGui.QPushButton):
		def mousePressEvent(self,ev):
			QtGui.QPushButton.mousePressEvent(self,ev)
			if not self._server.active: self._server.start()


class Server(object):
	def thread(self):
		print('start thread')
		while self.active: _rfb_.rfbProcessEvents( self.serverPtr, 1000 )
		print('thread exit')

	def exit(self):
		self.active = False
		if self.serverPtr:
			_rfb_.rfbShutdownServer( self.serverPtr, 1 )
			_rfb_.rfbScreenCleanup( self.serverPtr )

	def __init__(self):
		self.active = False
		self.serverPtr = None
		self.windll = None
		if sys.platform == 'win32': self.windll = ctypes.windll
		self.HALFSIZE = True
		if PyQt4:	# server config window
			self.create_window()
		else: self.start()

	def loop(self):
		while self.active:
			self.iterate()
			if not self.clients: time.sleep(1.0)
			else: time.sleep(0.001)
		print('loop main thread exit')
		#self.exit()

	def iterate( self ):
		if PyQt4:
			if self.ready:	# set from thread
				if not self.remotepointer:
					self.remotepointer = widget = QtGui.QWidget()
					widget.resize(24, 24)
					widget.setWindowTitle('remote pointer')
					flags = Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
					widget.setWindowFlags(flags)
					widget.setAttribute( Qt.WA_TranslucentBackground )
					self.remotepointer_button = but = QtGui.QToolButton(widget)
					but.setArrowType( Qt.UpArrow )
					but.setGeometry( 0, 0, 24, 24 )	#but.setText('xx')
				self.remotepointer.show()

			if self.remotepointer and not self.clients:
				self.remotepointer.hide()

			if self.ready_mouse:	# set from thread
				button,x,y = self.ready_mouse
				if button and not self._button_down:
					self._button_down = True
					self.remotepointer_button.setArrowType( Qt.DownArrow )
				elif not button and self._button_down:
					self._button_down = False
					self.remotepointer_button.setArrowType( Qt.UpArrow )


				_clicktxt = 'click!'
				if len(self._clicks)>1: _clicktxt = 'double-click!'
				if self._allow_mouse_warp:
					if self.windll:
						self.windll.user32.SetCursorPos( x, y )
						if self._allow_mouse_click and self._clicks:
							for x,y in self._clicks:
								self.windll.user32.SetCursorPos( x, y )	# seems like this won't work under wine
								self.windll.user32.mouse_event(2, 0, 0, 0,0) # left down
								self.windll.user32.mouse_event(3, 0, 0, 0,0) # left up

					elif gtk:
						display = gtk.gdk.display_get_default()
						display.warp_pointer(display.get_default_screen(), x, y)

				self.remotepointer.move( x+2, y-10 )

				#if button and (not self._button_down or not QtGui.QToolTip.isVisible()):
				#	self._button_down = True
				#	if QtGui.QToolTip.isVisible(): QtGui.QToolTip.hideText()
				#	QtGui.QToolTip.showText( QtCore.QPoint(x,y), _clicktxt )	# very unthreadsafe
				#elif not button: self._button_down = False
				if self._clicks:
					if button == 2: _clicktxt = 'right-click!'
					elif button == 4: _clicktxt = 'middle-click!'
					if QtGui.QToolTip.isVisible(): QtGui.QToolTip.hideText()
					QtGui.QToolTip.showText( QtCore.QPoint(x,y), _clicktxt )
				self.ready_mouse = None
				self._clicks = []

			if not naali: app.processEvents()

		if _rfb_.rfbIsActive(self.serverPtr):
			if time.time() - self._lastupdate > self._refresh:
				self._lastupdate=time.time()
				rect = self._rects[ self._index ]
				x,y,w,h = rect
				self._buffer  +=  self.get_screen_shot( self._antialias, rect )
				self.lock.acquire()
				ctypes.memmove( self.framebuffer, self._buffer, len(self._buffer) )
				self.lock.release()
				_rfb_.rfbMarkRectAsModified(self.serverPtr,x,y,w,h)
				self._index += 1
				if self._index == len(self._rects): self._index = 0; self._buffer = ''


	if gtk:	# works!
		def get_screen_size(self): return gtk.gdk.get_default_root_window().get_size()
		def get_screen_shot(self, antialias=False, rect=None ):
			w = gtk.gdk.get_default_root_window(); sz = w.get_size()
			pb = gtk.gdk.Pixbuf(gtk.gdk.COLORSPACE_RGB,False,8,sz[0],sz[1])
			pb = pb.get_from_drawable(w,w.get_colormap(),0,0,0,0,sz[0],sz[1])
			if self.HALFSIZE: pb = pb.scale_simple( sz[0]/2, sz[1]/2, gtk.gdk.INTERP_BILINEAR )
			#pb.save('/tmp/screenshot.png', 'png')	# is ok
			array = pb.get_pixels_array()
			assert array.dtype == numpy.uint8	# char is uint8 ( byte is int8 )
			return array.tostring()

	elif PyQt4:	# use PythonQt
		def create_window( self ):
			self.win = win = VncServerWindow()
			self.win._server = self
			win.resize(300, 40)
			win.move( 40, 40 )
			win.setWindowTitle('pyvnc-server')
			b = _VNCServer_start(win); b._server = self
			b.setText('start'); b.setGeometry( 10, 5, 60, 30 )
			b = _VNCServer_antialias(win); b._server = self; b.setChecked(True)
			b.setText('antialias'); b.setGeometry( 90, 5, 100, 30 )
			b = _VNCServer_halfsize(win); b._server = self; b.setChecked(True)
			b.setText('1/2-size'); b.setGeometry( 200, 5, 100, 30 )

			if self.windll:
				win.resize(300,75)
				b = _VNCServer_allow_remote(win); b._server = self
				b.setText('allow remote mouse clicks'); b.setChecked(False)
				b.setGeometry( 10, 35, 220, 30 )
			win.show()

		def get_screen_size(self):
			_size = QApplication.desktop().size()
			WIDTH = _size.width()
			HEIGHT = _size.height()
			return WIDTH, HEIGHT
		def get_screen_shot( self, antialias=False, rect=None ):
			WIDTH,HEIGHT = self.get_screen_size()
			if rect:
				x,y,w,h = rect
				img = QPixmap.grabWindow(QApplication.desktop().winId(), x,y,w,h).toImage()
				WIDTH = w; HEIGHT = h
			else:
				img = QPixmap.grabWindow(QApplication.desktop().winId()).toImage()	# returns 32bits

			if self.HALFSIZE:
				length = (WIDTH/2)*(HEIGHT/2)*BPP
				## note Qt.SmoothTransform only works on 32bit images
				if antialias: img = img.scaled( QSize(WIDTH/2, HEIGHT/2), transformMode=Qt.SmoothTransformation )
				else: img = img.scaled( QSize(WIDTH/2, HEIGHT/2) )
				#img = img.convertToFormat( img.Format_RGB888 )	# WRONG!
				return ctypes.string_at( int(img.bits()), length )
			else:
				length = WIDTH*HEIGHT*BPP
				return ctypes.string_at( int(img.bits()), length )


	mallocframebuffer = _rfb_(		
		"rfbMallocFrameBuffer", 
		ctypes.POINTER(ctypes.c_char),
		("rfbScreen", ctypes.POINTER( _rfb_.rfbScreenInfo()(type=True) )),
		("width", ctypes.c_int),
		("height", ctypes.c_int),
		("bpp", ctypes.c_int),
	)

	setmousecallback = _rfb_(		
		"rfbSetMouseCallback",
		ctypes.c_void_p,
		("rfbScreen", ctypes.POINTER( _rfb_.rfbScreenInfo()(type=True) )),
		("callback", ctypes.c_void_p)
	)

	setkeyboardcallback = _rfb_(		
		"rfbSetKeyboardCallback",
		ctypes.c_void_p,
		("rfbScreen", ctypes.POINTER( _rfb_.rfbScreenInfo()(type=True) )),
		("callback", ctypes.c_void_p) 
	)

	setnewclientcallback = _rfb_(		
		"rfbSetNewClientCallback",
		ctypes.c_void_p,
		("rfbScreen", ctypes.POINTER( _rfb_.rfbScreenInfo()(type=True) )),
		("callback", ctypes.c_void_p) 
	)


	def start( self ):
		self.active = True
		self._buffer = ''
		self._index = 0
		self._xtiles = 1	# must stay at 1
		self._ytiles = 8
		WIDTH,HEIGHT = self.get_screen_size()
		xstep = WIDTH / self._xtiles
		ystep = HEIGHT / self._ytiles
		self._rects = []
		for xt in range(self._xtiles):
			x = xt * xstep
			for yt in range(self._ytiles):
				y = yt * ystep
				self._rects.append( (x,y,xstep,ystep) )
				print( self._rects[-1] )

		self._antialias = True
		self._refresh = 0.01
		self._lastupdate = .0

		if self.HALFSIZE: WIDTH /= 2; HEIGHT /=2
		self.WIDTH = WIDTH
		self.HEIGHT = HEIGHT

		argv = (ctypes.c_char_p * 1)()
		argv[0] = 'pyvnc'
		self.serverPtr = _rfb_.rfbGetScreen( 
			#ctypes.byref(ctypes.c_int(1)), argv, 
			None, None,
			WIDTH, HEIGHT, 8, 3, BPP 
		)
		#self.serverPtr.contents.alwaysShared = 1

		#try: _rfb_.rfbInitServer( self.serverPtr )
		#except: _rfb_.rfbInitServerWithPthreadsAndZRLE( self.serverPtr )
		if sys.platform == 'linux2':
			_rfb_.rfbInitServerWithPthreadsAndZRLE( self.serverPtr )
		elif sys.platform == 'win32':	# quick hack feb18th
			rfbInitServer = _rfb_( 
				"rfbInitServerWithoutPthreadsAndZRLE", 
				ctypes.c_void_p,
				("rfbScreen", ctypes.POINTER( _rfb_._rfbScreenInfo ))
			)
			rfbInitServer( self.serverPtr )
		else: print('this platfom is not supported yet')

		cbuff = Server.mallocframebuffer( self.serverPtr, WIDTH, HEIGHT, BPP )
		self.framebuffer = cbuff
		## fast method to set framebuffer from python - make sure buffer write wont crash
		ctypes.memmove( cbuff, '\128'*WIDTH*HEIGHT*BPP, WIDTH*HEIGHT*BPP )

		self._cfuncs = []	# ctypes segfaults if pointers to prototyped c-callbacks are not saved
		
		proto = ctypes.CFUNCTYPE( ctypes.c_void_p, 
			ctypes.c_int, # button
			ctypes.c_int, # x
			ctypes.c_int, # y
			ctypes.POINTER( _rfb_.rfbScreenInfo()(type=True) ) )
		cb = proto( self.mouse_callback ); self._cfuncs.append( cb )
		Server.setmousecallback( self.serverPtr, ctypes.cast(cb, ctypes.c_void_p) )

		proto = ctypes.CFUNCTYPE( ctypes.c_int, ctypes.POINTER( _rfb_.rfbClientRec()(type=True) ) )
		cb = proto( self.new_client_callback ); self._cfuncs.append( cb )
		Server.setnewclientcallback( self.serverPtr, ctypes.cast(cb, ctypes.c_void_p) )

		# gone callback is passed clientrec, returns void
		proto = ctypes.CFUNCTYPE( ctypes.c_void_p, ctypes.POINTER( _rfb_.rfbClientRec()(type=True) ) )
		self._client_gone_callback = proto( self.client_gone_callback )
		# assigned in new client callback below

		self._allow_mouse_warp = False
		self._allow_mouse_click = False
		self.remotepointer = None
		self.clients = self._clients = []
		self._button_down = False
		self._clicks = []	# special +100 hack

		self.ready = False
		self.ready_mouse = None
		import thread
		self.lock = thread.allocate_lock()
		thread.start_new_thread( self.thread, () )
		if not naali: self.loop()


	def mouse_callback( self, button, x, y, serverPtr ):	# not threadsafe
		if self.HALFSIZE: x *= 2; y *= 2
		self.ready_mouse = (button,x,y)
		if button > 100:# and self._allow_mouse_click:	# hijacking, so we can always capture double clicks
			for i in range(button-100): self._clicks.append( (x,y) )

	setclientgonecallback = _rfb_(		
		"rfbSetClientGoneCallback",
		ctypes.c_void_p,
		("client", ctypes.POINTER( _rfb_.rfbClientRec()(type=True) )),
		("callback", ctypes.c_void_p) 
	)

	def new_client_callback( self, cptr ):		# not threadsafe
		print('hello new client', cptr)	#	cptr = rfbClientRec
		self.ready = True
		self._clients.append( cptr )
		Server.setclientgonecallback( cptr, ctypes.cast(self._client_gone_callback, ctypes.c_void_p) )
		return _rfb_.rfbNewClientAction[ "RFB_CLIENT_ACCEPT" ]	# "RFB_CLIENT_REFUSE"

	def client_gone_callback( self, cptr ):	# not threadsafe
		print('goodbye client', cptr)
		#self._clients.remove( cptr )	# won't work
		self._clients.pop()	# good enough for now
		#if not self._clients and self.remotepointer: self.remotepointer.hide()	# not threadsafe




if PyQt4:

	class _VNCClient_connect(QtGui.QPushButton):
		def mousePressEvent(self,ev):
			QtGui.QPushButton.mousePressEvent(self,ev)
			if not self._client.active:
				host = str(self._host.text())	#.toAscii()	# QLineEdit	# QString.toUtf8
				print('connecting to host', host)
				self._client.start( host=host )

	class VncViewerWindow( QtGui.QWidget ):
		def closeEvent( self, ev ):
			self._client.exit()
			ev.accept()
	class _VncViewer(QtGui.QLabel):
		def mouseMoveEvent(self, ev):	# defaults only when clicked
			pos = ev.pos()
			self._client.mouse['x'] = int(pos.x())
			self._client.mouse['y'] = int(pos.y())
		def mousePressEvent(self,ev):	# how to trap double clicks
			self._client.mouse['buttonMask'] = ev.button()
			pos = ev.pos()
			self._client.update_mouse( ev.button(), pos.x(), pos.y() )
		def mouseReleaseEvent(self,ev):
			self._client.mouse['buttonMask'] = 0
			#self._client._clicks.append( (ev.pos().x(), ev.pos().y()) )
			pos = ev.pos()
			self._client.update_mouse( ev.button()+100, pos.x(), pos.y() )

class Client(object):
	def loop(self):
		print('starting mainloop')
		while self.active: self.iterate()
		print('exit mainloop')

	def __init__(self):
		self.active = False
		if not PyQt4: self.start()
		else:
			self.win = win = QtGui.QWidget()	# note, need to save pointer to win
			win.setWindowTitle('pyvnc-client')
			win.resize(320, 50)
			win.move( 400, 40 )
			b = _VNCClient_connect( win )
			b._client = self
			b.setText('connect')
			b.setGeometry( 10, 10, 70, 30 )
			e = QtGui.QLineEdit( win )
			e.setText('localhost')
			e.setGeometry( 90, 10, 200, 24 )
			b._host = e
			win.show()

	def thread(self):
		print('start thread')
		while self.active:
			wait = _rfbclient_.WaitForMessage( self.clientPtr, 1000 )
			if wait: res = _rfbclient_.HandleRFBServerMessage(self.clientPtr)
			time.sleep(0.001)
		print('exit thread')

	def exit(self):
		#_rfbclient_.rfbClientCleanup( self.clientPtr )	# segfaults, why?
		self.active=False

	setclientcallback = _rfbclient_( 
		"rfbSetClientCallback", 	# function name
		ctypes.c_void_p,			# function result
		("client", ctypes.POINTER(_rfbclient_.rfbClient()(type=True) )),
		("type", ctypes.c_char_p),
		("cb", ctypes.c_void_p), 
	)

	def new_vncviewer( self ):
		print('constructing qt viewer')
		win = VncViewerWindow()
		win.setWindowTitle('pyvnc-viewer')
		win._client = self
		r = _VncViewer( win )
		r.setMouseTracking( True )
		r._client = self
		win._vnc_container = r
		return win

	def iterate(self):
		if self.widget: self._iterate_Qt()
		else: self._iterate_SDL()


	def _iterate_Qt(self):
		if self.ready:
			if not self._widget_shown:
				w=self._width; h=self._height
				self._widget_shown = True
				self.widget.resize( w+20, h+20 )
				self.widget._vnc_container.setGeometry(10, 10, w, h )
				self.widget.show()

			self.lock.acquire()
			data = str(self.data)	# trick is to copy, then it becomes threadsafe
			self.ready = False
			self.lock.release()
			qimage = QtGui.QImage(data, self._width, self._height, QtGui.QImage.Format_RGB32)
			pix = QtGui.QPixmap.fromImage(qimage)
			self.widget._vnc_container.setPixmap( pix )

			#t = time.time()
			#d = t-self.lastredraw; self.deltas.append( d )
			#self.lastredraw = t
			#if len(self.deltas)==100: print( 'avg', sum(self.deltas)/100.0 ); self.deltas=[]
			#if d > 0.001: print d

		if not naali: app.processEvents()

		if time.time() - self._lastupdate > self._refresh:
			self._lastupdate = time.time()
			_rfbclient_.SendPointerEvent(self.clientPtr, self.mouse['x'], self.mouse['y'], self.mouse['buttonMask'])
		## send clicks differently - special +100 hack - may slightly confuse other vnc-servers (but we don't care)
		#if self._clicks:
		#	clicks = len( self._clicks ) + 100; self._clicks = []
		#	print('sending clicks', clicks)
		#	_rfbclient_.SendPointerEvent(self.clientPtr, self.mouse['x'], self.mouse['y'], clicks)

	def update_mouse( self, button, x, y ): _rfbclient_.SendPointerEvent(self.clientPtr, x, y, button)


	def _iterate_SDL(self):
		e = SDL_Event()
		update = SDL_PollEvent( ctypes.byref(e) )
		if update:
			if e.type == SDL_MOUSEMOTION:
				mme = e.motion	# SDL_MouseMotionEvent
				x = ctypes.pointer(ctypes.c_int()); y = ctypes.pointer(ctypes.c_int())
				SDL_GetMouseState( x, y )
				self.mouse['x'] = x.contents.value
				self.mouse['y'] = y.contents.value
			elif e.type == SDL_MOUSEBUTTONDOWN: self.mouse['buttonMask'] = 1
			elif e.type == SDL_MOUSEBUTTONUP: self.mouse['buttonMask'] = 0
			if time.time() - self._lastupdate > self._refresh:
				self._lastupdate = time.time()
				_rfbclient_.SendPointerEvent(self.clientPtr, self.mouse['x'], self.mouse['y'], self.mouse['buttonMask'])




	def start( self, host='localhost', port=5900, name='pyvncclient' ):
		self.active = True
		self.lastredraw = .0; self.deltas = []
		self._clicks = []	# special
		self._refresh_read = 0.05
		self._lastupdate_read = 0
		self._refresh = 0.08	# dont flood server with lots of mouse updates
		self._lastupdate = 0
		self.mouse = {'x':0, 'y':0, 'buttonMask':0}
		self.surface = None
		self.clientPtr = _rfbclient_.rfbGetClient( 8, 3, BPP )	# here BPP must be 4

		print( 'client init...', host, port )
		argv = (ctypes.c_char_p * 2)()
		argv[0] = name
		argv[1] = '%s:%s' %(host,port)
		inited = _rfbclient_.rfbInitClient( self.clientPtr, 2, ctypes.pointer(argv) )
		assert inited != 0
		print( 'client init ok' )

		self.proto_update_frame_buffer = ctypes.CFUNCTYPE( 
			ctypes.c_void_p, 
			ctypes.POINTER(_rfbclient_.rfbClient()(type=True) ), 
			ctypes.c_int, ctypes.c_int, ctypes.c_int, ctypes.c_int 
		)
		self.cfunc = self.proto_update_frame_buffer( self.update_frame_buffer )
		Client.setclientcallback( self.clientPtr, "GotFrameBufferUpdate", 
			ctypes.cast(self.cfunc, ctypes.c_void_p) )

		self.widget = None
		self._widget_shown = False
		if PyQt4: self.widget = self.new_vncviewer()
		else: print( 'falling back to SDL' )

		self.ready = False
		import thread
		self.lock = thread.allocate_lock()
		thread.start_new_thread( self.thread, () )

		if not naali: self.loop()

	def update_frame_buffer( self, c, x, y, w, h ):	# not threadsafe
		if self.widget: self._update_Qt( c )
		else: self._update_SDL( c )

	def _update_Qt( self, c ):
		client = c.contents
		self._width = client.width; self._height = client.height
		#cpixels = ctypes.cast( client.frameBuffer, ctypes.POINTER(ctypes.c_char) )	# dont need to cast
		length = client.width * client.height * BPP
		self.lock.acquire()
		self.data = ctypes.string_at( client.frameBuffer, length )
		self.lock.release()
		self.ready = True


	def _update_SDL(self,c):
		if not self.surface: self.initsdl()
		client = c.contents
		length = client.width * client.height * BPP
		spixels = ctypes.cast( self.surface.contents.pixels, ctypes.POINTER(ctypes.c_char))
		cpixels = ctypes.cast( client.frameBuffer, ctypes.POINTER(ctypes.c_char) )
		ctypes.memmove( spixels, cpixels, length )
		SDL_Flip( self.surface )	# no need to flip unbuffered?  may not work with all hardware, safer to always flip


	def initsdl(self):
		client = self.clientPtr.contents
		SDL_Init( SDL_INIT_VIDEO )
		self.surface = SDL_SetVideoMode( client.width, client.height, 32, 0 )
		SDL_WM_SetCaption('ctypes-sdl-fallback-viewer',None)
		format = self.surface.contents.format	#SDL_PixelFormat


if __name__ == '__main__':
	if '--client' in sys.argv:
		client = Client()
		app.exec_()
	else:
		server = Server()
		app.exec_()
	print('clean exit')


