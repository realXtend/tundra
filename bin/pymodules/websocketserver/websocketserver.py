import circuits

class NaaliWebsocketServer(circuits.BaseComponent):
    @circuits.handler("update")
    def update(self, t):
        print "websocket server"


