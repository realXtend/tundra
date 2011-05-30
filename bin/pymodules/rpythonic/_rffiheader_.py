import sys
sys.path.append( '../../pypy' )
from pypy.rpython.lltypesystem import lltype, rffi
from pypy.rpython.annlowlevel import llhelper


def generate_rpythonic_structure_wrapper( name, struct, *fields ):
	# allows simple access to type by attribute name #
	# simpler API that getting from the struct._fields_ tuple
	exec( 'class %s_PROTOTYPES(object): pass' %name )
	singleton = eval( '%s_PROTOTYPES()' %name )		# assigned below

	head = 'class %s( object ):' %name
	body = []
	for field in fields:
		n,t = field
		setattr( singleton, n, t )
		fhead = 'def %s( self, *args ):' %n
		fbody = ['if not args: return self._POINTER_.c_%s' %n]
		if isinstance( t, lltype.Ptr ):
			if isinstance( t.TO, lltype.FuncType ):
				g = 'llhelper(  self.PROTOTYPES.%s ,  args[0]  )' %n
				fbody.append( 'else: self._POINTER_.c_%s = %s' %(n,g) )
			else:
				g = 'self.PROTOTYPES.%s' %n
				fbody.append( 'else: self._POINTER_.c_%s = rffi.cast(%s,args[0])' %(n,g) )

		else:
			fbody.append( 'else: self._POINTER_.c_%s = args[0]' %n )
		g = fhead + '\n\t\t' + '\n\t\t'.join( fbody )
		body.append( g )
	gen = head + '\n\t' + '\n\t'.join( body )
	print( gen )
	exec( gen )
	klass = locals()[name]
	print( klass )
	klass.PROTOTYPES = singleton
	return klass






