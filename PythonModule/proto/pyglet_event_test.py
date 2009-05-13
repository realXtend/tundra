import pyglet


class RealxtendViewer(pyglet.event.EventDispatcher):
    def clank(self):
        self.dispatch_event('on_clank')

    def click(self, clicks):
        self.dispatch_event('on_clicked', clicks)

    def on_clank(self):
        print 'Default clank handler.'
        
    def run(self, deltatime):
        pass
        print ".",
        self.dispatch_event('update', deltatime)
        
print RealxtendViewer

RealxtendViewer.register_event_type('on_clank')
RealxtendViewer.register_event_type('on_clicked')
RealxtendViewer.register_event_type('update')

viewer = RealxtendViewer()

print viewer

@viewer.event
def on_clank():
    pass

@viewer.event
def on_clicked(clicks):
    pass

def override_on_clicked(clicks):
    pass
    
@viewer.event
def update(deltatime):
    print "_",
    

#make the mock viewer run
pyglet.clock.schedule_interval(viewer.run, 0.1)
viewer.push_handlers(on_clicked=override_on_clicked)

print "done scheduling"

pyglet.app.run()
