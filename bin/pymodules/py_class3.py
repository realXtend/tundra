

#import traceback
import rexviewer as r
'''py_class.py - Python source designed to demonstrate the use of python embedding'''

class Multiply: 
	def __init__(self): 
		self.a = 6 
		self.b = 5
		self.d = 1
		self.method = None
		self.method2 = None
		self.key2 = None
    
	def multiply(self):
		c = self.a*self.b
		print 'The result of', self.a, 'x', self.b, ':', c
		print "*****************************************"
		#self.d = c
		return c
    
	def multiply2(self, a, b):
		c = a*b*self.d
		print 'The result of', a, 'x', b, 'x', d, ':', c
		return c                  

	def setCallback(self, aMethod):
		print "!!"
		print str(aMethod)
		self.method = aMethod

	def setCallback2(self, aMethod, aKey):
	#def setCallback2(self, aMethod2):
		print "a"
		print aKey
		r.pyEventCallback("a","a")
		self.method2 = aMethod
		#print "****"


	def TestCallback2(self, aKey):
		#self.method2(self.key2, "hoijaa")
		print "TestCallback2"
		print aKey
		#self.method2("key", "hoijaa")
		r.pyEventCallback("key","a")

	def multiply3(self):
		c = self.a*self.b*self.d
		print 'The result of', self.a, 'x', self.b, 'x', self.d, ':', c
		if(self.method!=None):
			print "calling back"
			#str(self.method)
			#dir(self.method)
			self.method(1,2)
		return c