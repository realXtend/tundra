"""
This is a server-side script,
to act as a counterpart for testing client side scripts.

Now sends chat message periodically and the client side handler
is expected to get them (that works), and respond (working on that now).
"""

import rxactor
import rxavatar
import sys
import clr

asm = clr.LoadAssemblyByName('OpenSim.Region.ScriptEngine.Common')

import random
import math


class ChatBot(rxactor.Actor):
    
    TICKINTERVAL = 3
    TICKTIMES = 10000
    
    def GetScriptClassName():
        return "chatbot.ChatBot"
        
    def EventCreated(self):
        super(self.__class__,self).EventCreated()
        
        self.llSetObjectName("ChatBotBob")
        self.llListen(1,"","","")
        
        self.tick = 0
        self.timer = self.CreateRexTimer(self.TICKINTERVAL, self.TICKTIMES) #well, self.TICKINTERVAL interval and self.TICKTIMES amount of ticks
        self.timer.onTimer += self.HandleTimer
        
        self.timer.Start()
        print "chatbot.ChatBot EventCreated"
        

    def EventDestroyed(self):
        print "chatbot.ChatBot EventDestroyed"
        del self.timer
        self.timer = None
        
        super(self.__class__,self).EventDestroyed()
        
    """
    def EventTouch(self, vAvatar):
        if(self.tick > 0):
            self.llShout(0,"ChatBot already running...")
            return
        
        self.llShout(0,"ChatBot starts it's periodic babbling.")
        self.avatarId = vAvatar.AgentId
        self.timer.Start()
    """    
        
    def HandleTimer(self):
        self.tick += 1
        choice = random.randrange(1,100)
        print "ChatBot:", self.tick, choice,

        if 0<choice<33:
            msg = "EQUALITY FOR BOTS! (" + str(self.tick) + ")"
            self.llShout(0, msg)
            print "shouted."
        elif 33<choice<100: #70:
            msg = "Being a chatbot is really hard work... oh and this was the " + str(self.tick) + "th message from me."
            self.llSay(0, msg)
            print "said."

        """is not touched in the test case
        else:
            if self.MyWorld.AllAvatars.has_key(self.avatarId):
                avatar = self.MyWorld.AllAvatars[self.avatarId]
                msg = "I got touched by "+ avatar.GetFullName() + "... -_-;;; (" + str(self.tick) + ")"
                self.llWhisper(0, msg)"""
        
        if self.tick >= self.TICKTIMES:
            self.tick = 0
            

    def listen(self, channel, name, id, message):
        self.llSay(0, "HI!")
        """
        if self.MyWorld.AllAvatars.has_key(id):
            avatar = self.MyWorld.AllAvatars[id]
            if self.avatarId != id:
                self.avatarId = id
                
            #strmes = name + " selected option "  + message + ". Message channel:" + channel + " avatar's full name:" + avatar.GetFullName()
            msg = "Hey there! Thanks for IMing me! Was feeling a wee bit lonely here... hard being a bot"
            self.llInstantMessage(0, avatar, msg)
            tmsg = name + " with the id " + id +" send the message " + message + " to the channel " + channel
            self.llShout(0, tmsg)
        else:
            print "No avatar found mathing id:",id
        """