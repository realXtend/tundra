import os, sys, ctypes
IS32BIT = (ctypes.sizeof(ctypes.c_void_p)==4)

## this ctypes file expects to be in the cache directory, 
## with precompiled c-libraries two directories up, if the library is not there try to load from the system.
_clibs_dir = os.path.join( os.path.dirname(os.path.abspath(__file__)), '../../clibs' )
_clibs_dir = os.path.abspath( _clibs_dir )

def _load_ctypes_lib( name ):
	if os.name == 'posix':
		if sys.platform=='linux2':
			name += '.so'
			if IS32BIT: 	path = os.path.join(_clibs_dir,'linux32')
			else: 		path = os.path.join(_clibs_dir,'linux64')
			url = os.path.join( path, name )
			if os.path.isfile( url ): return ctypes.CDLL(url)
			elif os.path.isfile( '/usr/local/lib/%s'%name ): return ctypes.CDLL('/usr/local/lib/%s'%name)
			elif os.path.isfile( '/usr/local/lib64/%s'%name ) and not IS32BIT: return ctypes.CDLL('/usr/local/lib64/%s'%name)
			elif os.path.isfile( '/usr/lib/%s'%name ): return ctypes.CDLL('/usr/lib/%s'%name)
			elif os.path.isfile( './%s'%name ): return ctypes.CDLL('./%s'%name)
			else: return ctypes.CDLL(name) #fallback

		elif sys.platform == 'darwin':
			name += '.dylib'
			if IS32BIT: 	path = os.path.join(_clibs_dir,'osx32')
			else: 		path = os.path.join(_clibs_dir,'osx64')
			url = os.path.join( path, name )
			if os.path.isfile( url ): return ctypes.CDLL(url)
			else: return ctypes.CDLL(name) #fallback

	elif os.name == 'nt':
		name += '.dll'
		if IS32BIT: 	path = os.path.join(_clibs_dir,'win32')
		else: 		path = os.path.join(_clibs_dir,'win64')
		url = os.path.join( path, name )
		if os.path.isfile( url ): return ctypes.CDLL(url)
		else: return ctypes.CDLL(name) #fallback

RPYTHONIC_WRAPPER_FUNCTIONS = {}
RPYTHONIC_WRAPPER_FUNCTIONS_FAILURES = []
RPYTHONIC_AUTOPREFIX_IGNORE = []

## ctypes does not clearly expose these types ##
PyCFuncPtrType = type(ctypes.CFUNCTYPE(ctypes.c_void_p))
PyCArrayType = type( ctypes.c_int * 2 )
PyCPointerType = type( ctypes.POINTER(ctypes.c_int) )
PyCStructType = type( ctypes.Structure )
CArgObject = type( ctypes.byref(ctypes.c_int()) )

class _rpythonic_meta_(object):
	_rpythonic_ = True		# workaround for now, must have a way to know if object is a meta from another module, isinstance(o,_rpythonic_meta_) will fail in those cases. another workaround could be check sys.modules for other rpythonic modules and fetch _rpythonic_meta_ from there.
	def __init__(self, *args, **kw ):	# cheap trick, abuse **kw, and look for "pointer"
		if 'pointer' in kw: self._pointer = kw['pointer']
		elif kw: raise SyntaxError	# sorry, you can not init with keywords
		else: self._pointer = ctypes.pointer( self._ctypes_struct_(*args) )
	def __getattr__(self,name):
		if hasattr( self._pointer.contents, name ):
			return getattr( self._pointer.contents, name )

		else:	# when rpythonic failed to generate good bindings - these lookups should be cached
			for parent in self._rpythonic_parent_classes_:
				if hasattr( parent, name ):
					method = getattr( parent, name )	# should check if it really is an unbound method
					#func = method._rpythonic_function_
					#return lambda *args: func( self, *args )	# what!, not self._pointer ??
					func = parent._rpythonic_unbound_lookup_[ method ]
					n = func.name
					if len(func.argnames) > 1:
						argnames = func.argnames[ 1 : ]
						a = ',' + '=None,'.join( argnames ) + '=None'
						b = ','.join( argnames )
					else: a = b = ''
					lamb = eval( 'lambda self %s: %s( self._pointer, %s )' %(a,n,b) )
					setattr( self.__class__, name, lamb )
					#return lamb	# this would return the unbound lambda, must call getattr again
					return getattr( self, name )

			## last resort, load from global name space ##
			G = globals()
			if name in G: return lambda *args: G[name](self._pointer, *args)
			else:
				for prefix in self._autoprefix_:
					n = prefix + name
					if n in G: return lambda *args: G[n](self._pointer, *args)
				print( 'possible auto-prefixes available', self._autoprefix_ )
				raise AttributeError

	def __call__(self, type=False):
		if type: return self._ctypes_struct_
		else: return self._pointer

PYTHON_RESERVED_KEYWORDS = 'for while in as global with try except lambda return raise if else elif eval exec and not or break continue finally print yield del def class assert'.split()


def _rpythonic_generate_subclass_( name, struct, functions ):
	head = 'class %s( _rpythonic_meta_ ):' %name
	body = [ 
		'_rpythonic_parent_classes_ = []' ,
		'_rpythonic_unbound_lookup_ = {}' 
	]

	names = [ func.name for func in functions ]

	possibles = {}
	rank = []		# rank by longest name
	for n1 in names:
		prefix = ''
		for i,char in enumerate(n1):
			prefix += char
			if prefix not in possibles:
				possibles[ prefix ] = 0
				for n2 in names:
					if n2.startswith( prefix ):
						possibles[ prefix ] += 1

				if not rank or len(prefix) > len(rank[-1]) and possibles[prefix] > len(names)/4:
					rank.append( prefix )

	top = []
	while rank:
		best = rank.pop()
		if possibles[best] > len(functions)/2 and best not in names:
			if best.endswith('_set_') or best.endswith('_get_'): best = best[ : -4 ]
			elif best.endswith('Set') or best.endswith('Get'): best = best[ : -3 ]

			rem = []
			for other in rank:
				if best.startswith(other): rem.append( other )
			for r in rem: rank.remove( r )

			if best not in top: top.append( best )

		if len(top) > 3: break

	for n in names:		# find shortest prefixes #
		prefix = ''
		for i,char in enumerate(n):		# cammelCase
			if i==0: prefix += char; continue
			if char.isupper() and len(prefix) >= 2: break
			prefix += char
		if prefix and prefix != n and len(prefix) >= 2:
			hits = 0
			for other in names:
				if other.startswith( prefix ): hits += 1
			if hits >= 2 and prefix not in top:
				top.append( prefix )
				if len(top) >= 6: break

	## setup full names
	for func in functions:
		n = func.name
		if len(func.argnames) > 1:
			argnames = func.argnames[ 1 : ]
			a = ',' + '=None,'.join( argnames ) + '=None'
			b = ','.join( argnames )
		else: a = b = ''

		fhead = 'def %s( self %s ):' %(n,a)
		fbody = ['return %s(self._pointer, %s)' %(func.name,b)]
		g = fhead + '\n\t\t' + '\n\t\t'.join( fbody )
		body.append( g )
		#body.append( '%s._rpythonic_function_ = %s' %(func.name, func.name) )

	## setup short names ##
	for n in names:
		for prefix in top:
			if n.startswith(prefix) and n[len(prefix):] not in names:
				alt = n[ len(prefix) : ]
				if alt and alt != n and alt not in PYTHON_RESERVED_KEYWORDS and not alt.isdigit() and not alt[0].isdigit():
					body.append( '%s = %s' %(alt,n) )
					names.append( alt )

	gen = head + '\n\t' + '\n\t'.join( body )
	try: exec( gen )
	except:
		print( gen )
		raise SyntaxError

	klass = locals()[name]
	klass._ctypes_struct_ = struct
	klass._autoprefix_ = top
	for func in functions:
		unbound = getattr( klass, func.name )
		klass._rpythonic_unbound_lookup_[ unbound ] = func
		# klass.longname is klass.shortname = False
		# klass.longname == klass.shortname = True
	return klass

def _rpythonic_convert_structs_to_objects():
	G = globals()
	for klass in _OOAPI_:
		altname = name = klass.__name__
		prefix = ''
		for i,char in enumerate(name):
			if i==0: prefix += char; continue
			if char.isupper(): break
			prefix += char
		if prefix and prefix != name:
			hits = 0
			for other in _OOAPI_:
				if other is not klass:
					if other.__name__.startswith( prefix ): hits += 1
			if hits >= 2:
				altname = name[ len(prefix) : ]

		funcs = _OOAPI_[ klass ]
		newklass = _rpythonic_generate_subclass_( altname, klass, funcs )
		klass._rpythonic_wrapper_class_ = newklass
		G[ name ] = newklass	# replace struct with wrapper
		if altname not in G: G[ altname ] = newklass	# safely define with nicer name
		elif altname != name: # odd cases, maybe a function that returns the object, almost never happens.
			print('WARN - not replacing something with struct wrapper:', G[altname] )


def _rpythonic_setup_return_wrappers():
	R = _rpythonic_function_
	for klass in _OOAPI_:
		if klass in _OOAPI_RETURNS_OBJECT_:
			for f in _OOAPI_RETURNS_OBJECT_[klass]:
				f.object_oriented = True
				if not f.return_wrapper:	# just in case the ctypes footer defines it
					f.return_wrapper = klass._rpythonic_wrapper_class_

def _rpythonic_function_( name, result=ctypes.c_void_p, args=[]):
	mname = '_metafunc_%s' %name
	exec( 'class %s( _rpythonic_metafunc_ ): pass' %mname )
	k = locals()[mname]
	return k( name, result, args )

_OOAPI_ = {}
_OOAPI_RETURNS_OBJECT_ = {}

class _rpythonic_metafunc_(object):
	def __init__(self, name, result=ctypes.c_void_p, args=[]):
		self.name = name
		self.result = result
		self.argtypes = []
		self.argnames = []
		self.argtypestypes = []
		for i,arg in enumerate(args):
			n,t = arg
			if n in PYTHON_RESERVED_KEYWORDS: n = 'C_'+n
			if n in self.argnames: n = '%s%s' %(n,i)
			self.argnames.append( n )
			self.argtypes.append( t )
			self.argtypestypes.append( type(t) )		# precomputed for speed
		self.argtypes = tuple( self.argtypes )
		self.argnames = tuple( self.argnames )
		self.argtypestypes = tuple( self.argtypestypes )
		self.numargs = len( self.argtypes )
		self.callbacks = [None] * self.numargs
		self.return_wrapper = None
		self.object_oriented = False
		self.function = None
		try:
			func = self.function = getattr(_ctypes_lib_, self.name )
			RPYTHONIC_WRAPPER_FUNCTIONS[ name ] = self
		except:
			RPYTHONIC_WRAPPER_FUNCTIONS_FAILURES.append( name )
		if self.function: self.reset()


	def reset(self):
		if self.argnames:
			a = ',' + '=None,'.join( self.argnames ) + '=None'
			b = ','.join( self.argnames )
		else: a = b = ''

		callmeth = eval( 'lambda self %s: self._call_( %s )' %(a,b) )
		setattr( self.__class__, '__call__', callmeth )

		self.function.restype = self.result
		self.function.argtypes = self.argtypes

		if type( self.result ) is PyCPointerType and type(self.result._type_) is PyCStructType:
			klass = self.result._type_
			if klass not in _OOAPI_RETURNS_OBJECT_: _OOAPI_RETURNS_OBJECT_[klass] = []
			_OOAPI_RETURNS_OBJECT_[klass].append( self )

		self.defaults = []
		for i in range( self.numargs ):
			T = self.argtypes[ i ]
			if type(T) is PyCFuncPtrType:
				p = T()	# func pointers can not be None
				self.defaults.append( p )
				self.callbacks[ i ] = p					# save reference
			elif T in (ctypes.c_int, ctypes.c_uint, ctypes.c_long, ctypes.c_ulong):
				self.defaults.append( 0 )
			elif T in (ctypes.c_float, ctypes.c_double):
				self.defaults.append( .0 )
			else: self.defaults.append( None )	# None is allowed for all other types

			## generate OO API ##
			if i == 0 and type(T) is PyCPointerType and type(T._type_) is PyCStructType:
				klass = T._type_
				if klass not in _OOAPI_: _OOAPI_[ klass ] = []
				_OOAPI_[ klass ].append( self )

	def _call_( self, *args ):			# allow flexible calling types
		cargs = list( self.defaults )
		for i,arg in enumerate(args):
			if isinstance( arg, _rpythonic_meta_ ): arg = arg._pointer
			elif hasattr( arg, '_rpythonic_' ): arg = arg._pointer	# workaround

			t = type(arg)
			k = self.argtypes[ i ]
			kt = self.argtypestypes[ i ]
			if arg is None and cargs[i] is not None:	# use user defaults, very rare cases
				continue

			elif t is bool and k is ctypes.c_int:
				if arg: cargs[i] = 1	#ctypes.c_int(1)
				else: cargs[i] = 0	#ctypes.c_int(0)

			elif t in (list,tuple):				# convert lists and tuples into array
				if kt is PyCArrayType:
					cargs[ i ] = k(*arg)
				elif kt is PyCStructType:
					if k._array_wrapper_: cargs[ i ] = k(arg)	# allow easy array init
					else: cargs[ i ] = k(*arg)							# allow multiple args
				elif kt is PyCPointerType:
					cargs[ i ] = _convert_nested_list_to_pointer( k, arg )
				else: assert 0

			elif isinstance( arg, ctypes._Pointer ) and t is not k and kt is PyCPointerType:
				cargs[ i ] = ctypes.cast( arg, k )		# generic's that need to be cast

			elif kt is PyCStructType and isinstance( arg, ctypes._Pointer ):
				cargs[ i ] = arg.contents	# fixed may25

			elif kt is PyCPointerType and not isinstance( arg, (ctypes._Pointer,CArgObject) ):
				if t in (int,float,bool): ptr = k( k._type_(arg) )
				elif t is str:
					ptr = k( k._type_() )		# not k() otherwise null pointer error
					for j, char in enumerate(arg): ptr[ j ] = char.encode('ascii')
				elif t in (PyCStructType, PyCArrayType):
					ptr = ctypes.cast( ctypes.pointer( arg ), k )
				else: ptr = arg	# buggy TODO
				#else:		# wrap py object - not safe!!
				#	ptr = ctypes.cast( ctypes.pointer(ctypes.py_object(arg)), k )
				cargs[ i ] = ptr
			elif kt is PyCFuncPtrType:
				cargs[ i ] = self.callbacks[ i ] = k( arg )		# assume arg is a callable
			else:
				cargs[ i ] = arg		# directly pass

		## if you define your own return_wrapper, it must take argument pointer=...
		#print( cargs )
		if self.return_wrapper: return self.return_wrapper( pointer=self.function( *cargs ) )
		else: return self.function( *cargs )



def _convert_nested_list_to_pointer( k, arg ):
	depth = 0; s = k
	while True:
		if type(s) is PyCPointerType:
			s = getattr( s, '_type_' )
			depth += 1
		else: break
	assert depth and depth <= 2
	if depth == 1:
		T = k._type_
		ptr = k( k._type_() )
		for i in range( len(arg) ):
			ptr[ i ] = T( *arg[i] )
	elif depth == 2:
		T = k._type_._type_
		_ptr = k._type_( k._type_._type_() )
		for i in range(len( arg )):
			for j in range( len(arg[i]) ):
				_ptr[ j ] = T( *arg[ i ][ j ] )
		ptr = k( _ptr )
	return ptr


def __freeze_rpythonic_struct( cls, fields ):
	if cls not in _OOAPI_: _OOAPI_[ cls ] = []	# wrap all structs
	try: setattr( cls, '_fields_', fields )
	except:
		print( 'WARN - bad order struct freeze', cls )
		#cls._fields_ = []

class _rpythonic_struct_( ctypes.Structure ):
	_array_wrapper_ = False
	_fields_ = []
	_methods_ = {}
	def __call__(self): return self
	def __init__(self, *args, **kw ):
		cargs = []
		argtypes = []
		for a in self._fields_: argtypes.append( a[1] )
		if len(args) > len(argtypes): args = [args]	# allow both calling conventions
		for i,arg in enumerate( args ):
			if isinstance( arg, _rpythonic_meta_ ): arg = arg._pointer
			t = type(arg)
			k = argtypes[ i ]
			if t in (list,tuple):
				if k.__class__.__name__	== 'PyCArrayType':
					cargs.append( k(*arg) )
				elif k.__class__.__name__ == 'PyCStructType':
					if k._array_wrapper_: cargs.append( k(arg) )	# allow easy array init
					else: cargs.append( k(*arg) )							# allow multiple args
			elif isinstance( arg, ctypes._Pointer ) and t is not k:
				cargs[ i ] = ctypes.cast( arg, k )	# generic's that need to be cast
			elif k.__class__.__name__ == 'PyCArrayType' and t in (float,int,bool):
				cargs.append( k(arg) )		# support init array from single value
			else:
				cargs.append( arg )		# directly pass

		ctypes.Structure.__init__(self, *cargs, **kw)


def _rpythonic_make_nice_global_enums_():
	G = globals()
	for name in RPYTHONIC_GLOBAL_ENUMS:
		if '_' in name and name.index('_') <= 4:
			altname = name[ name.index('_') + 1 : ]
			if altname not in G:
				G[altname] = RPYTHONIC_GLOBAL_ENUMS[ name ]

def _rpythonic_clean_up_missing_functions_():
	G = globals()
	for f in RPYTHONIC_WRAPPER_FUNCTIONS_FAILURES:
		G.pop( f )
	print( "C functions loaded: %s" %len(RPYTHONIC_WRAPPER_FUNCTIONS) )
	print( "C functions failed: %s" %len(RPYTHONIC_WRAPPER_FUNCTIONS_FAILURES) )


