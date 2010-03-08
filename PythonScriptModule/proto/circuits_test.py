from circuits import handler, Event, Component, Manager

# Create a new Component
class Foo(Component):
    def a(self):
        print "Foo's Event Handler for 'a'"
        
    def b(self):
        print "Foo's Event Handler for 'b'"
        return "Foo's Event Handler for 'b'"
      
    @handler('a')
    def arbitarily_named_a(self):
        print "Foo's arbitarily_named_a"
      
class NonComponentFoo(Component):
    """if some component doesn't want to 
    subclass circuits comp for some reason"""

    @handler("a")
    def my_non_a(self):
        print "NonComponentFoos Event Handler for 'a'"
        
#@handler("a")
def a_handler():
    """another foo handler - now a plain function"""
    print "arbitary function 'a' handler"

# Create a new Manager
m = Manager()

# Start the Manager in ThreadMode
m.start()

# To Start the Manager in ProcessMode
#m.start(process=True)

# Create a new instance of the Component Foo
foo = Foo()
#noncompfoo = NonComponentFoo()

# Register foo with the Manager
m += foo # Equivalent to: foo.register(m)
#m += noncompfoo
#m._add(a_handler, "a")
"""the plain function doesn't have attribute 'priority' (etc. i guess) 
so a no-go i figure"""

# Push an Event to a Channel called "a"
m.push(Event(), "a")

# Send an Event to a Channel called "b"
x = m.send(Event(), "b")
print x # x contains: "Foo's Event Handler for 'b'"

while 1:
    pass

# Unregister foo
foo.unregister() # Or: m -= foo

# Stop the Manager
m.stop()
