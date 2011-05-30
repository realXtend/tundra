# -*- coding: utf-8 -*-
## Kinect Addon for Blender
## by Hart, may25th 2011
## License: BSD
## Special Thanks: Adminotech Ltd

bl_info = {
	"name": "Pyppet",
	"author": "HartsAntler",
	"version": (0,0,1),
	"blender": (2,5,7),
	"location": "View3D > Properties > Kinect",
	"description": "Export to Ogre xml and binary formats",
	"warning": "libfreenect and opencv dynamic libraries must be in your path so ctypes can load them",
	"category": "3D View"
}

VERSION = bl_info["name"] + " Addon v" + bl_info["version"][0].__str__() + "." + bl_info["version"][1].__str__() + "." + bl_info["version"][2].__str__() 

try:
	import bpy
	from bpy.props import *
except: bpy = None

import rpythonic.rpythonic as rpythonic
print( rpythonic )
print( dir(rpythonic) )
freenect = rpythonic.module( 'libfreenect_sync' )
assert freenect
gui = rpythonic.module( 'highgui' )
assert gui
cv = rpythonic.module( 'cv', secondary=gui )
assert cv

wii = rpythonic.module( 'wiiuse' )		# optional
sdl = rpythonic.module( 'SDL' )		# optional
if sdl: sdl.SDL_Init( sdl.SDL_INIT_JOYSTICK )


import time, math, ctypes

threading = None
try:		# see if we are inside Naali
	import naali
	import circuits
	#import circuits.Component	# ImportError: No module named Component File "pymodules/core/freshimport.py", line 22, in freshimport
	import PythonQt
	from PythonQt import QtGui, QtCore
	from PythonQt.QtCore import Qt, QSize
	from PythonQt.QtGui import QPixmap, QApplication
	PyQt4 = PythonQt

	import thread
	Klock = thread.allocate_lock()
	lock = thread.allocate_lock()
	Alock = thread.allocate_lock()

except:
	naali = None

	import threading
	Klock = threading.Lock()
	lock = threading.Lock()
	Alock = threading.Lock()

############## end of imports ################



SWEEPS = 32
SWEEP_STEP = 2

class Threaded(object):
	def stop(self): self.active = False
	def start(self):
		self.active = True
		self.loops = 0
		if hasattr(self,'setup'): self.setup()
		if threading:
			threading._start_new_thread( self.loop, () )
		else:
			thread.start_new_thread( self.loop, () )

	def sync_blender( self, context ): pass	# overload me


class JoystickThread( Threaded ):
	def __init__( self ):
		self.active = False
		self.joysticks = []

	def setup(self):
		self.active = True
		while self.joysticks:
			joy = self.joysticks.pop()
			joy.close()

		n = sdl.SDL_NumJoysticks()
		for i in range(n):
			joy = sdl.SDL_JoystickOpen(i)
			self.joysticks.append( Joystick( joy, i ) )

	def loop(self):
		print('starting joystick SDL thread')
		while self.active:
			sdl.SDL_JoystickUpdate()
			for joy in self.joysticks: joy.update()
			time.sleep(0.001)
		print('end joystick SDL thread')

	def sync_blender(self, context):
		for joy in self.joysticks: joy.sync_blender( context )

class Joystick(object):
	def close(self): pass	# TODO proper close
	def __init__(self, joy, uid ):
		self.uid = uid
		self.joy = joy
		self.naxes = joy.NumAxes()
		self.nbuttons = joy.NumButtons()
		self.nhats = joy.NumHats()
		print('<joystick-hardware> axes: %s buttons: %s hats: %s'%(self.naxes, self.nbuttons, self.nhats))
		self.state_axes = [.0] * self.naxes
		self.state_buttons = [False] * self.nbuttons
		self.state_hats = [None] * self.nhats

		if naali:
			self.ndev = naali.devices.CreateAndRegisterPositionalDevice("joystick%s"%self.uid)
			if not self.ndev: print('failed to register joystick device')
			else:
				naali.frame.connect("Updated(float)", self.update_naali_signals)

	def update_naali_signals(self, frametime ):
		dev = self.ndev
		g = 0
		for i in range(0,self.naxes-1,2):
			x = self.state_axes[ i ]
			y = self.state_axes[ i + 1 ]
			vec = naali.Vector3df( x,y, .0 )
			dev.PositionEvent( 'analog%s'%g, vec )
			g += 1
		# ask Pforce for dev.BooleanEvent for buttons

	def update(self):
		joy = self.joy
		for i in range(self.naxes):
			self.state_axes[ i ] = joy.GetAxis( i ) / 32767.0
		for i in range(self.nbuttons):
			self.state_buttons[ i ] = joy.GetButton( i )
		for i in range(self.nhats):
			self.state_hats[ i ] = joy.GetHat( i )

	def sync_blender(self, context):
		mode = 1 in self.state_buttons

		if context.active_object:
			bo = context.active_object
			if mode:
				bo.scale.x += self.state_axes[0] * 0.1
				bo.scale.y += self.state_axes[1] * 0.1
				bo.scale.z += self.state_axes[2] * 0.1
			else:
				bo.location.x += self.state_axes[0] * 0.1
				bo.location.y += self.state_axes[1] * 0.1
				bo.location.z += self.state_axes[2] * 0.1




class WiiThread( Threaded ):
	def __init__( self ):
		self.active = False
		self._nmotes = 2
		self._pointer = wii.wiiuse_init( self._nmotes )
		self.wiimotes = []

	def setup( self ):
		while self.wiimotes:
			mote = self.wiimotes.pop()
			mote.close()

		found = wii.find( self._pointer, self._nmotes, 5 )
		assert found
		print( 'found wiimotes', found )
		connected = wii.wiiuse_connect( self._pointer, self._nmotes )
		assert connected
		print( 'connected wiimotes', connected )

		for i in range( self._nmotes ):
			wm = self._pointer[i]
			wii.wiiuse_motion_sensing(wm, 1)
			wii.wiiuse_set_leds( wm, wii.WIIMOTE_LED_2)
			self.wiimotes.append( Wiimote( i ) )

		self.active = True
		return found

		
	def callback( self, state ):	# called from C
		self.wiimotes[ state.contents.uid - 1 ].update( state )

	def loop( self ):
		print('starting wiiuse thread')
		while self.active:
			status = wii.wiiuse_update( self._pointer, self._nmotes, self.callback )
			if not status: time.sleep(0.001)
		print('exit wiiuse thread')
		time.sleep(0.1)
		wii.wiiuse_cleanup( self._pointer, self._nmotes)


	def sync_blender(self, context):
		for wm in self.wiimotes: wm.sync_blender( context )



class Wiimote(object):
	def sync_blender(self, context):
		if context.active_object:
			bo = context.active_object
			bo.location.x += self.x * 0.1
			bo.location.y += self.y * 0.1
			bo.location.z += self.z * 0.1


	def close(self): pass	# TODO

	def __init__(self, uid):
		self.uid = uid		# the uid in 
		self.buttons = {}
		for char in 'ABUDLR-+H': self.buttons[char] = 0
		self.x = .0
		self.y = .0
		self.z = .0
		if naali:
			self.ndev = naali.devices.CreateAndRegisterPositionalDevice("wiimote%s"%self.uid)
			if not self.ndev: print('failed to register wiimote device')
			else:
				naali.frame.connect("Updated(float)", self.update_naali_signals)

	def update_naali_signals(self, frametime ):
		dev = self.ndev
		vec = naali.Vector3df( self.x,self.y,self.z )
		dev.PositionEvent( 'accel', vec )


	def update( self, wm ):
		#assert self.uid == wm.uid-1	# wiiuse devs start at index 1
		bs = self.buttons
		for tag in 'UDLR+H-12AB': bs[ tag ] = 0
		if (wii.IS_PRESSED(wm, wii.WIIMOTE_BUTTON_A)): bs['A'] = 1
		if (wii.IS_PRESSED(wm, wii.WIIMOTE_BUTTON_B)): bs['B'] = 1
		if (wii.IS_PRESSED(wm, wii.WIIMOTE_BUTTON_UP)): bs['U'] = 1
		if (wii.IS_PRESSED(wm, wii.WIIMOTE_BUTTON_DOWN)): bs['D'] = 1
		if (wii.IS_PRESSED(wm, wii.WIIMOTE_BUTTON_LEFT)): bs['L'] = 1
		if (wii.IS_PRESSED(wm, wii.WIIMOTE_BUTTON_RIGHT)): bs['R'] = 1
		if (wii.IS_PRESSED(wm, wii.WIIMOTE_BUTTON_MINUS)): bs['-'] = 1
		if (wii.IS_PRESSED(wm, wii.WIIMOTE_BUTTON_PLUS)): bs['+'] = 1
		if (wii.IS_PRESSED(wm, wii.WIIMOTE_BUTTON_ONE)): bs['1'] = 1
		if (wii.IS_PRESSED(wm, wii.WIIMOTE_BUTTON_TWO)): bs['2'] = 1
		if (wii.IS_PRESSED(wm, wii.WIIMOTE_BUTTON_HOME)): bs['H'] = 1
		#print( bs )
		self.x = (wm.contents.accel.x / 255.0) -0.5
		self.y = (wm.contents.accel.y / 255.0) - 0.5
		self.z = (wm.contents.accel.z / 255.0) - 0.5
		print( self.x, self.y, self.z )



######################## kinect #########################

class Point(object):
	def __len__(self): return 2
	def __getitem__(self,key):
		if key == 0: return self.x
		elif key == 1: return self.y
		else: raise IndexError
	def __init__(self, x,y): self.x = x; self.y = y
	def scale( self, other ): return Point( self.x*other.x, self.y*other.y )
	def dot( self, other ): return self.x*other.x + self.y*other.y
	def length( self ): return math.sqrt( self.dot(self) )
	def angle( self, other ):
		dot = self.dot( other )
		length = self.length() * other.length()
		if not length: return .0
		else:
			try: return math.acos( dot / length )
			except ValueError: return .0	# math domain error





###################### KINECT ######################

class FreenectThread0( Threaded ):
	def __init__(self):
		self.active = False
		self._lenpix = 480 * 640 * 2
		self.buffer_type = ctypes.c_void_p
		self.buffer = self.buffer_type()
		self.pointer = ctypes.pointer( self.buffer )


	def loop(self):
		print( 'starting kinect thread' )
		print( 'setting leds' )
		freenect.sync_set_led( freenect.LED_YELLOW, 0 )

		while self.active:
			self.loops += 1
			status = freenect.sync_get_depth( ctypes.byref(self.pointer), 0, 0, 
				freenect.FREENECT_DEPTH_11BIT
			)
			#print('kinect status', status )
			Klock.acquire()		# seems safe even without locking?
			cv.SetData( OCVThread1.DEPTH16RAW, self.pointer, 640*2 )
			Klock.release()
			#time.sleep(0.01)
		freenect.sync_set_led( freenect.LED_OFF, 0 )
		freenect.sync_stop()
		print( 'exit kinect thread', self.loops )

class Camera( object ):
	def move( self, x, y, z=.0 ):
		self.spin_delta += x
		self.height_delta += y * 0.2
		self.dolly_delta += z

	def __init__(self, cam ):
		self.camera = cam
		self.spin = self.spin_delta = .0
		self.height = self.height_delta = .0
		self.dolly = 10.0
		self.dolly_delta = .0
		dcon = None
		tcon = None
		for con in cam.constraints:
			if con.type == 'LIMIT_DISTANCE': dcon = con
			elif con.type == 'TRACK_TO': tcon = con

		if not dcon: dcon = cam.constraints.new( 'LIMIT_DISTANCE' )
		if not tcon: tcon = cam.constraints.new( 'TRACK_TO' )

		tcon.up_axis = 'UP_Y'
		tcon.track_axis = 'TRACK_NEGATIVE_Z'
		dcon.limit_mode = 'LIMITDIST_INSIDE'
		self.dcon = dcon
		self.tcon = tcon
		self.target = None

	def update(self, context ):	# must be thread safe
		#print( dir( context))
		if not hasattr( context, 'active_object' ):
			print('thread problem'); return

		if not self.target or (self.target != context.active_object.name):
			if context.active_object.type == 'MESH':	# only look at meshes
				print( 'setting new look at target' )
				self.target = context.active_object.name
				self.dcon.target = context.active_object
				self.tcon.target = context.active_object


		self.spin += self.spin_delta
		self.height = self.height_delta
		self.dolly += self.dolly_delta
		self.spin_delta *= 0.2
		self.height_delta *= 0.8
		self.dolly_delta *= 0.2
		if self.dolly > 20.0: self.dolly = 20.0
		elif self.dolly < 6.0: self.dolly = 6.0
		if self.height > 1.0: self.height = 1.0
		elif self.height < -1.0: self.height = -1.0

		if self.target:
			x = math.sin( math.radians( self.spin ) )
			y = math.cos( math.radians( self.spin ) )
			## camera.location is worldspace
			camera = bpy.data.objects['Camera']
			camera.location.x = x * self.dolly
			camera.location.y = y * self.dolly
			camera.location.z += self.height
			if abs(camera.location.z) > 2.0:
				camera.location.z *= 0.995
			self.dcon.distance = self.dolly

###############################################

class Shape(object):
	HAND = None
	storage_hull = cv.CreateMemStorage(0)
	storage_defects = cv.CreateMemStorage(0)

	def __init__( self, poly, depth ):
		self.depth = depth
		self.points = []
		self.angles = [.0]
		self.touching = []
		self.children = []
		self.parents = []
		self.guess = None
		xavg = []; yavg = []
		xmin = ymin = xmax = ymax = None
		#for j in range( poly.contents.total ):
		for j in range( poly.total ):
			_ptr = cv.GetSeqElem( poly, j )	# ugly, opencv API requires casting
			point = ctypes.cast(_ptr, ctypes.POINTER(cv.Point()(type=True)) )
			x = point.contents.x; y = point.contents.y
			p = Point(x,y)
			if self.points:
				a = math.degrees(self.points[-1].angle( p ))
				self.angles.append( a )
				
			self.points.append( p )
			xavg.append( x ); yavg.append( y )
			if j == 0:
				xmin = xmax = x
				ymin = ymax = y
			else:
				if x < xmin: xmin = x
				if x > xmax: xmax = x
				if y < ymin: ymin = y
				if y > ymax: ymax = y

		self.avariance = .0
		self.avariance_points = [.0,.0]
		if self.angles:
			prev = self.angles[0]
			for a in self.angles[1:]:
				v = abs( prev - a )
				self.avariance_points.append( v )
				self.avariance += v
				prev = a
			#print 'variance', self.avariance
			#print 'variance-points', self.avariance_points
			#print 'len len', len(self.points), len(self.avariance_points)

		n = len(self.points)

		self.weight = ( sum(xavg)/float(n), sum(yavg)/float(n) )
		self.width = xmax - xmin
		self.height = ymax - ymin
		self.center = ( int(xmin + (self.width/2)), int(ymin + (self.height/2)) )
		self.rectangle = ( (xmin,ymin), (xmax,ymax) )

		self.dwidth = xmax - xmin
		self.dheight = ymax - ymin
		self.dcenter = ( xmin + (self.dwidth/2), ymin + (self.dheight/2) )
		self.drectangle = ( (xmin,ymin), (xmax,ymax) )

		self.defects = []
		self.center_defects = self.center
		self.convex = cv.CheckContourConvexity( poly )
		if not self.convex:
			T = 80
			dxavg = []; dyavg = []
			hull = cv.ConvexHull2( poly, self.storage_hull, 1, 0 )
			defects = cv.ConvexityDefects( poly, hull, self.storage_defects )
			#n = defects.contents.total	# old API
			n = defects.total
			cdp = ctypes.POINTER(cv.ConvexityDefect()(type=True))
			for j in range( n ):
				D = ctypes.cast( cv.GetSeqElem( defects, j ), cdp )
				s = D.contents.start.contents
				e = D.contents.end.contents
				d = D.contents.depth_point.contents
				start	= ( s.x, s.y )
				end		= ( e.x, e.y )
				depth 	= ( d.x, d.y )

				## ignore large defects ##
				if abs(end[0] - depth[0]) > T or abs(end[1] - depth[1]) > T or abs(start[0] - end[0]) > T or abs(start[1] - end[1]) > T: continue

				dxavg.append( depth[0] )
				dyavg.append( depth[1] )
				self.defects.append( (start, end, depth) )

			xmin = ymin = 999999
			xmax = ymax = -1
			if self.defects:
				n = len(self.defects)
				self.center_defects = ( int(sum(dxavg)/float(n)), int(sum(dyavg)/float(n)) )
				for j,f in enumerate( self.defects ):
					s,e,d = f
					if s[0] < xmin: xmin = s[0]
					if e[0] < xmin: xmin = e[0]
					if s[0] > xmax: xmax = s[0]
					if e[0] > xmax: xmax = e[0]
					if s[1] < ymin: ymin = s[1]
					if e[1] < ymin: ymin = e[1]
					if s[1] > ymax: ymax = s[1]
					if e[1] > ymax: ymax = e[1]

				self.dwidth = xmax - xmin
				self.dheight = ymax - ymin
				self.dcenter = ( xmin + (self.dwidth/2), ymin + (self.dheight/2) )
				self.drectangle = ( (xmin,ymin), (xmax,ymax) )

		cv.ClearMemStorage( self.storage_hull )
		cv.ClearMemStorage( self.storage_defects )


	def touches( self, other ):
		pt1, pt2 = self.rectangle
		if other.rectangle[0] == pt1 or other.rectangle[1] == pt2:
			if other not in self.touching: self.touching.append( other )
		return other in self.touching

	def contains( self, other ):
		#if self.touches( other ): return False
		s1, s2 = self.rectangle
		o1, o2 = other.rectangle
		if s1[0] <= o1[0] and s1[1] <= o1[1] and s2[0] >= o2[0] and s2[1] >= o2[1]:
			if other not in self.children:
				self.children.append( other )
				other.parents.append( self )
		return other in self.children

	def draw_bounds( self, image, mode='rectangle' ):
		if mode=='rectangle':
			cv.Rectangle(	image, self.rectangle[0], self.rectangle[1],
				(64,0,128), 1, 8, 0)
		else:
			cv.Circle( image, self.center, int(self.width), (128,0,128), 3, cv.CV_AA, 0 )
			if self.guess == 'head': cv.Circle( image, self.center, 12, (128,0,128), 5, cv.CV_AA, 0 )


	def draw_defects( self, image ):
		for d in self.defects:
			start,end,depth = d
			cv.Line(image, start, end,
				(0,64,128),
				1,
				8, 
				0
			)
			cv.Line(image, end, depth,
				(0,64,128),
				2,
				8, 
				0
			)
		if len( self.defects ) >= 3 and self.center_defects:
			if self.guess == 'hand':
				cv.Circle( image, self.center_defects, 24, (255,255,0), 15, 7, 0 )
			else:
				cv.Circle( image, self.center_defects, 28, (200,200,0), 1, 7, 0 )

	def draw_variance(self, image ):
		if len(self.points) < 2: return
		color = (128,80,80); width = 1
		if self.avariance > 30: color = (255,155,155)
		elif self.avariance > 10: color = (255,80, 80)
		else: color = (225,60, 60)

		for i in range(0,len(self.points),2):
			w = self.avariance_points[i]
			a = self.points[i-1]
			b = self.points[i]
			cv.Line( image, (a.x,a.y), (b.x,b.y), color, width+int(w*0.15), 8, 0 )




class OCVThread2( Threaded ):
	def update_hand( self, shape ):
		sx,sy = shape.center_defects
		self.y = -((sy / 480.0) - 0.75) * 0.5
		self.z = (shape.depth - 110) * 0.005

		x = sx / 640
		y = -(sy / 480.0)
		z = (shape.depth - 110) * 0.005
		self._hand = ( x, y, z )

	def update_head( self, shape ):
		sx,sy = shape.center
		self.x = ((sx / 640.0) - 0.5) * 3.25

		x = sx / 640
		y = -(sy / 480.0)
		z = (shape.depth - 110) * 0.005
		self._head = ( x, y, z )


	def set_camera( self, ob ): self.camera = Camera( ob )

	def update_camera( self, context ):		# needs to be thread safe
		if not self.camera: return
		self.camera.move( self.x, self.y, self.z )
		self.camera.update( context )

	def __init__(self):
		self.active = False
		self.contours_image = cv.cvCreateImage((640,480), 8, 3)
		self.camera = None
		self.x = .0
		self.y = .0
		self.z = .0
		self._show_window = False
		self._head = self._hand = None

		if naali:
			self.naali_dev = naali.devices.CreateAndRegisterPositionalDevice("kinect")
			if not self.naali_dev: print('failed to register kinect device')
			else:
				naali.frame.connect("Updated(float)", self.update_naali_signals)

	def update_naali_signals(self, frametime ):
		dev = self.naali_dev
		if self._hand:
			x,y,z = self._hand
			vec = naali.Vector3df( x,y,z )
			dev.PositionEvent( 'hand', vec )
		if self._head:
			x,y,z = self._head
			vec = naali.Vector3df( x,y,z )
			dev.PositionEvent( 'head', vec )


	def update_frame( self, shapes ):
		img = self.contours_image

		for s1 in shapes:
			for s2 in shapes:
				if s1 is not s2:
					s1.touches( s2 )
					s1.contains( s2 )		# parents, children

		## first check for obvious hand ##
		hand = head = None
		best = []; maybe = []; poor = []
		for s in shapes:
			if not s.defects: continue
			if len(s.touching) >= 2 and len(s.defects) >= 3:
				if s.dwidth < 320 and s.dheight < 340:
					best.append( s )
					if not hand or ( s.dwidth <= hand.dwidth and s.dheight <= hand.dheight ):
						hand = s
				else: maybe.append( s )
			elif len(s.defects) >= 3:
				poor.append( s )

		if hand:
			hand.guess = 'hand'
			hand.draw_defects( self.contours_image )
			if self.camera: self.update_hand( hand )

		else:
			#head = shapes[0]	# could be noise
			for s in shapes:	# order - start is nearest to kinect, end of list is far from kinect
				if s.width > 80 and s.height > 100:	# filter noise
					if s.width < 240 and s.height < 300: 	# this can be the arm
						if s.center[1] < 380:		# clip bottom, arms?
							head = s; break
			if head:
				head.guess = 'head'
				print( head.depth, len(head.children), head.center[1] )
				head.draw_bounds( self.contours_image, 'circle' )
				if s.center[0] < 220 or s.center[0] > 420:
					self.update_hand( head )
				else: self.update_head( head )

		if self._show_window:
			lock.acquire()
			cv.ShowImage( 'contours', self.contours_image )
			if not naali: cv.WaitKey(1)	# waitkey is called in other ocv thread
			lock.release()

		self.x *= 0.99
		self.y *= 0.9
		self.z *= 0.85

		if head or hand:
			if self.x > -0.1 and self.x < 0.1: self.x = .0
			#if self.y > -0.1 and self.y < 0.1: self.y = .0
			#if self.z > -0.1 and self.y < 0.1: self.z = .0
		#print( 'cam', self.x, self.y, self.z )

	def loop(self):
		#print('starting opencv thread2')
		frame = []
		trash = []
		while self.active:
			self.loops += 1
			#print('ocv thread2', self.loops)
			batch = []
			#f not len(OCVThread1.BUFFER) > 1: time.sleep(0.01)
			lock.acquire()
			while OCVThread1.BUFFER:
				batch.append( OCVThread1.BUFFER.pop() )
			lock.release()
			if not batch:
				time.sleep(0.01)
				trash = []
			while batch:
				#time.sleep(0.01)
				P = batch.pop()
				if P.index == 0:
					self.update_frame( frame )		# final draws and shows
					cv.SetZero( self.contours_image )
					frame = [ Shape( P.poly2, P.depth ) ]
				else:
					frame.append( Shape( P.poly2, P.depth ) )
				P.draw( self.contours_image )	# do this outside of update_frame
				s = frame[-1]
				s.draw_defects( self.contours_image )
				s.draw_bounds( self.contours_image )
				s.draw_variance( self.contours_image )

				#del P		# can randomly segfault?, let python GC del P
				trash.append( P )

		print( 'ocv thread2 exit', self.loops)


class OCVThread1( Threaded ):
	BUFFER = []
	DEPTH16RAW = cv.CreateImage((640,480), cv.IPL_DEPTH_16S, 1)
	def __init__(self):
		self.active = False
		self.depth16raw = OCVThread1.DEPTH16RAW
		self.depth16 = cv.CreateImage((640,480), cv.IPL_DEPTH_16S, 1)
		self.depth8 = cv.cvCreateImage((640,480), 8, 1)
		self.sweep_thresh = [ cv.cvCreateImage((640,480), 8, 1) for i in range(SWEEPS) ]
		self.storage = cv.CreateMemStorage(0)
		#cv.NamedWindow('depth', 1)
		cv.SetZero( self.depth16raw )
		self.show_depth = False

	def loop(self):
		print( 'starting opencv thread' )
		self.active = True
		contour_t = cv.Contour()(type=True)
		while self.active:
			self.loops += 1

			#print('opencv main thread',self.loops)
			#time.sleep(0.01)

			Klock.acquire()
			cv.ConvertScale( self.depth16raw, self.depth8, 0.18, 0 )
			Klock.release()

			# blur helps?
			#cv.Smooth( self.depth8, self.depth8, cv.CV_BLUR, 16, 16, 0.1, 0.1 )
			#cv.Smooth( self.depth8, self.depth8, cv.CV_GAUSSIAN, 13, 13, 0.1, 0.1 )

			if self.show_depth:
				lock.acquire()		# this part of opencv is not threadsafe
				cv.ShowImage( 'depth', self.depth8 )
				cv.WaitKey(1)
				lock.release()

			thresh = 80
			index = 0
			for img in self.sweep_thresh:
				cv.ClearMemStorage( self.storage )
				#print( img )
				cv.Threshold( self.depth8, img, thresh, 255, cv.CV_THRESH_BINARY_INV )
				#cv.Canny( img, img, 0, 255, 3 )	# too slow
				seq = cv.CvSeq()
				#contours = ctypes.pointer( ctypes.pointer( seq ) )
				contours = ctypes.pointer( seq() )

				cv.FindContours(img, self.storage, contours, ctypes.sizeof( contour_t ), cv.CV_RETR_EXTERNAL, cv.CV_CHAIN_APPROX_SIMPLE, (0,0) )

				_total = 0
				try: _total = contours.contents.contents.total
				except:
					thresh += SWEEP_STEP
					continue

				P = ReducedPolygon( contours, index, thresh )
				lock.acquire()
				self.BUFFER.append( P )
				lock.release()
				index += 1
				thresh += SWEEP_STEP

		print( 'exit opencv thread1', self.loops )


class ReducedPolygon( object ):
	def copy_buffer( self, memory ):	# clear local mem on copy?
		return cv.SeqSlice( self.poly2, cv.Slice(0,self.total), memory, 1 )
	def __del__(self):
		cv.ReleaseMemStorage( ctypes.pointer(self.mem1()) )
		cv.ReleaseMemStorage( ctypes.pointer(self.mem2()) )

	def __init__(self, contours, index, depth ):
		self.index = index		# used from other thread, so it knows to clear drawing surface on zero.
		self.depth = depth		# kinect depth level
		self.mem1 = cv.CreateMemStorage(0)
		self.mem2 = cv.CreateMemStorage(0)

		contour_t = cv.Contour()(type=True)

		self.poly1 = cv.ApproxPoly( 	 # pass1
			contours.contents, 
			ctypes.sizeof( contour_t ), 
			self.mem1,
			cv.CV_POLY_APPROX_DP,
			3.0, 1 )
		self.poly2 = cv.ApproxPoly( 	 # pass2
			self.poly1, 
			ctypes.sizeof( contour_t ), 
			self.mem2,
			cv.CV_POLY_APPROX_DP,
			20.0, 1 )

		self.total = self.poly2.total

	def draw(self, image, lowres=False):
		cv.DrawContours(
			image, 
			self.poly1,
			(255,64,128), 	# external color
			(255,255,0),	# hole color
			1, # max levels
			1, # thickness
			cv.CV_AA, # linetype
			(0, 0)
		)
		if lowres:
			cv.DrawContours(
				image, 
				self.poly2,
				(128,64,64), 	# external color
				(255,255,0),	# hole color
				1, # max levels
				1, # thickness
				8, # linetype
				(0, 0)
			)






##################################
class KinectSingleton(object):
	def show_window(self):
		cv.NamedWindow('contours', 1)
		self.threads[-1]._show_window = True
	def hide_window(self):
		self.threads[-1]._show_window = False


	def set_camera( self, ob ):
		self.threads[-1].set_camera( ob )

	def __init__(self):
		self.active = False
		self.threads = [
			FreenectThread0(),
			OCVThread1(),
			OCVThread2(),
		]


	def start(self):
		if self.active: print('already running...'); return
		self.active = True
		self.threads[0].start()
		time.sleep(0.1)
		self.threads[1].start()
		self.threads[2].start()

	def stop(self):
		for t in self.threads: t.active = False
		self.active = False
		time.sleep(1)

	def sync_blender( self, context ):
		#print('threadsafe now...')
		#Alock.acquire()
		self.threads[-1].update_camera( bpy.context )
		#Alock.release()


#######################
Kinect = KinectSingleton()
#######################

if bpy:
	class BlenderSyncSingleton(object):
		def __init__(self):
			self._handle = None
			self.threads = {
				'kinect' : Kinect,
				'wiimotes' : WiiThread(),
				'joysticks' : JoystickThread(),
			}

		def setup(self):
			if not self._handle:
				# bpy.context.window_manager.event_timer_add   - this is not it
				for area in bpy.context.window.screen.areas:
					if area.type == 'VIEW_3D':
						for reg in area.regions:
							if reg.type == 'WINDOW':
								self._handle = reg.callback_add( self.sync, (bpy.context,), 'POST_VIEW' )
								break

		def sync(self, context):
			for o in self.threads.values():
				o.sync_blender( context )

		def start( self, dev ):
			self.setup()
			self.threads[ dev ].start()
			if dev == 'kinect': Kinect.show_window()



	BlenderSync = BlenderSyncSingleton()


	def register():
		print( '-'*80)
		print(VERSION)
		bpy.utils.register_module(__name__)
		print( '-'*80)

	def unregister():
		print('unreg-> kinect')
		bpy.utils.unregister_module(__name__)

	class BindCameraOp(bpy.types.Operator):
		bl_idname = "pyppet.bind_camera_kinect"
		bl_label = "Bind Camera to Kinect"
		bl_description = "camera driven by kinect"
		@classmethod
		def poll(cls, context): return True
		def invoke(self, context, event):
			Kinect.set_camera( bpy.data.objects['Camera'] )
			#bpy.ops.view3d.viewnumpad(type='CAMERA')	# should be able to call this from any context
			return 'FINISHED'


	class EnableOp(bpy.types.Operator):
		bl_idname = "pyppet.enable_device"
		bl_label = "Toggle Device"
		bl_description = "Toggle ..."
		kinect = BoolProperty(name="kinect", default=False)
		wiimotes = BoolProperty(name="wiimotes", default=False)
		joysticks = BoolProperty(name="joysticks", default=False)


		@classmethod
		def poll(cls, context): return True	#return context.mode!='EDIT_MESH'
		def invoke(self, context, event):
			if self.kinect: BlenderSync.start( 'kinect' )
			elif self.wiimotes: BlenderSync.start( 'wiimotes' )
			elif self.joysticks: BlenderSync.start( 'joysticks' )
			return 'FINISHED'

	class DisableOp(bpy.types.Operator):
		bl_idname = "pyppet.disable_device"
		bl_label = "Disable Device"
		bl_description = "end kinect threads"
		kinect = BoolProperty(name="kinect", default=False)
		wiimotes = BoolProperty(name="wiimotes", default=False)
		joysticks = BoolProperty(name="joysticks", default=False)

		@classmethod
		def poll(cls, context): return True
		def invoke(self, context, event):
			if self.kinect: BlenderSync.stop( 'kinect' )
			elif self.wiimotes: BlenderSync.stop( 'wiimotes' )
			elif self.joysticks: BlenderSync.stop( 'joysticks' )

			return 'FINISHED'


	class PyppetPanel(bpy.types.Panel):
		bl_label = "Pyppet"
		bl_space_type = 'PROPERTIES'
		bl_region_type = 'WINDOW'
		bl_context = "physics"


		def draw(self, context):
			layout = self.layout

			box = layout.box()
			box.label(text='Kinect')
			row = box.row()
			op = row.operator(EnableOp.bl_idname, text="Enable"); op.kinect = True
			op = row.operator(DisableOp.bl_idname, text="Disable"); op.kinect = True
			box.operator(BindCameraOp.bl_idname, text="Bind Camera")

			box = layout.box()
			box.label(text='Wiimotes')
			row = box.row()
			op = row.operator(EnableOp.bl_idname, text="Enable"); op.wiimotes = True
			op = row.operator(DisableOp.bl_idname, text="Disable"); op.wiimotes = True

			box = layout.box()
			box.label(text='Joysticks')
			row = box.row()
			op = row.operator(EnableOp.bl_idname, text="Enable"); op.joysticks = True
			op = row.operator(DisableOp.bl_idname, text="Disable"); op.joysticks = True




if naali:
	print('---------------PYPPET - inside Naali----------------')
	class PyppetDevicesWindow( QtGui.QWidget ):
		def closeEvent( self, ev ):
			for dev in self._devices: dev.stop()
			ev.accept()

	class ToggleDev(QtGui.QCheckBox):
		def mouseReleaseEvent(self,ev):
			QtGui.QCheckBox.mouseReleaseEvent(self,ev)
			on = self.isChecked(); print( on )
			if on: self._device.start()
			else: self._device.stop()
	class ToggleKinectWindow(QtGui.QPushButton):
		def mouseReleaseEvent(self,ev):
			QtGui.QPushButton.mouseReleaseEvent(self,ev)
			on = self.isChecked(); print( on )
			if on: Kinect.show_window()
			else: Kinect.hide_window()

	class PyppetQt(object):
		def __init__(self):
			self.win = win = PyppetDevicesWindow()
			self.threads = [
				Kinect,
				WiiThread(),
				JoystickThread(),
			]

			self.win._devices = self.threads
			win.resize(240, 80)
			win.move( 40, 120 )
			win.setWindowTitle('devices')


			a = ToggleDev(win); a._device = self.threads[0]
			a.setText('Kinect'); a.setGeometry( 10, 5, 80, 30 )


			b = ToggleKinectWindow(win); b.setCheckable(True)
			b.setText('view'); b.setGeometry( 120, 5, 60, 30 )

			c = ToggleDev(win)
			c._device = self.threads[1]
			c.setText('Wiimotes')
			c.setGeometry( 10, 40, 100, 30 )

			d = ToggleDev(win); d._device = self.threads[2]
			d.setText('Joysticks'); d.setGeometry( 120, 40, 100, 30 )

			win.show()
			print( '<NaaliPyppet>' )


	class DevicesComponent( circuits.Component ):
		@circuits.handler("update")
		def update(self, deltatime):
			pass	#print('delta okkkk', deltatime)
		def __init__(self):
			circuits.Component.__init__(self)	# don't forget to call __init__ on parent!!!
			self.gui = PyppetQt()


elif __name__ == "__main__":	# run basic test
	Kinect.start()
	Kinect.show_window()
	while True:
		try: time.sleep(10)
		except: break
	Kinect.stop()



