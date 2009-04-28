import sys
##from IMDemo import Account

class DemoUI:
    def __init__(self):
        
##      status values not connected, connecting, connected
        self.status = 0  #"not connected"
        self.uiMainSelect = {
            0: self.doUnconnected,
            1: self.doConnecting,
            2: self.doConnected,
            3: self.doChat}
            
        self.uiConnectionSelect = {
            '0': self.doConnect,
            '1': self.doRegisterAccount,
            '2': self.doReadAccountAndConnect,
            '3': self.doExit}     
        pass
    
##====================================================
##    Display options
##====================================================
    def runUI(self):
        pass
        print self.status
##        action, arg =
        self.uiMainSelect[self.status]()
##        method = self.uiMainSelect[self.status]
##        action, arg = method()
        
##        answer = read_answer()
##        ch = answer[0:1]
##        action, arg = self.uiConnectionSelect[ch]()
##        return action, arg
        
##====================================================
##    Read user input
##====================================================
    def GetUserInput(self):
        answer = read_answer()
        if(self.status!=3):
            ch = answer[0:1]
            print ch
            select = None
            if(self.status==0):
                #action, arg = self.uiConnectionSelect[ch]()
                select = {
                '0': self.doConnect,
                '1': self.doRegisterAccount,
                '2': self.doReadAccountAndConnect,
                '3': self.doExit}  
                action, arg = select[ch]()
            elif(self.status==1):
                select = {
                '3': self.doExit}
                action, arg = select[ch]()
            elif(self.status==2):
                select = {
                '0': self.disconnect,
                '1': self.startChat,
                '2': self.addFriend, 
                '3': self.doExit,
                '4': self.doChatWithFriend}
                action, arg = select[ch]()            
            return action, arg

        if(answer=="end_chat"):
            return "endchat", answer
        else:
            return "sendmess", answer

    def ProcessConnInput(self, answer):
        action = None
        arg = None
        if(self.status!=3):
            ch = answer[0:1]
            print ch
            select = None
            if(self.status==0):
                #action, arg = self.uiConnectionSelect[ch]()
                select = {
                '0': self.doConnect,
                '1': self.doRegisterAccount,
                '2': self.doReadAccountAndConnect,
                '3': self.doExit}
                action, arg = select[ch]()
            elif(self.status==1):
                select = {
                '3': self.doExit}
                action, arg = select[ch]()
            elif(self.status==2):
                select = {
                '0': self.disconnect,
                '1': self.startChat,
                '2': self.addFriend, 
                '3': self.doExit,
                '4': self.doChatWithFriend}
                
                try:
                    action, arg = select[ch]()
                except:
                    print "not valid command"
            return action, arg

        if(answer=="end_chat"):
            return "endchat", answer
        else:
            return "sendmess", answer
        pass

    def setStatus(self, st):
        self.status = st

##====================================================
##    UNCONNECTED
##====================================================

    def doUnconnected(self):
        print "-- connect to server --"
        #print "(0) connect"
        #print "(1) register account"
        print "(2) just read account.txt file and connect"
        print "(3) exit"

    def askRegister(self):
        pass

    def doConnect(self):
        acc = askAccount()
        return "connect", acc
        pass
    def doRegisterAccount(self):
        pass
    def doReadAccountAndConnect(self):
        d = loadAccountFile()
        return "accountconnect", d
        pass

    def doExit(self):
        return "exit", None

##====================================================
##    CONNECTING
##====================================================
              
    def doConnecting(self):
        print "-- connecting --"
        print "(3) exit"
        return None, None        

##====================================================
##    CONNECTED
##====================================================
    
    def doConnected(self):
        print "-- connected --"
##        print "(0) Disconnect"
        print "(1) Start chat"
##        print "(2) Add friend"
        print "(3) exit"
##        print "(4) Chat with friend"
        return None, None        

    def disconnect(self):
        return None, None

    
    def startChat(self):
        print "give counterpart jid address or 'cancel' to abort"
        jid = read_answer()
##        print jid
        if jid=="cancel":
            self.status = 2
            return "continue", None
        else:
            return "startchat", jid
        
        return None, None        
    def addFriend(self):
        return None, None        

##====================================================
##    CHAT
##====================================================

    def doChat(self):
        print "--chat--"
        print "input end_chat for ending chat"

    def doChatWithFriend(self):
        print "--chat--"
        print "unimplemented.."


##====================================================
##    UTIL
##====================================================

def read_answer():
    line = sys.stdin.readline().rstrip("\r\n")
##    ch = sys.stdin.read(1)
    return line 

def askAccount():
    print "-- give account --"
    acc = read_answer()        
    print acc
    return acc

def loadAccountFile():
    print "loading account.txt file"
    #accoutFileStr = file("Account.txt").read()
    accoutFileStr = file("pymodules/Account.txt").read()
    d = parse_account(accoutFileStr)
    return d

# copied from telep example
def parse_account(s):
    lines = s.splitlines()
    pairs = []
    
    manager = None
    protocol = None

    for line in lines:
        if not line.strip():
            continue

        k, v = line.split(':', 1)
        k = k.strip()
        v = v.strip()

        if k == 'manager':
            manager = v
        elif k == 'protocol':
            protocol = v
        else:
            if k not in ("account", "password"):
                if v.lower() == "false":
                    v = False
                elif v.lower() == "true":
                    v = True
                else:
                    try:
                        v = dbus.UInt32(int(v))
                    except:
                        pass
            pairs.append((k, v))

    d = dict(pairs)
    

    a = d['account']
    print a
    return d
