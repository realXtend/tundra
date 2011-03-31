#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set sw=3 sts=3 ts=3

"""(Example) IRC Client

A basic IRC client with a very basic qt interface
"""

import os
from socket import gethostname

from circuits import handler, Component
from circuits.net.sockets import TCPClient, Connect
from circuits.net.protocols.irc import IRC, PRIVMSG, USER, NICK, JOIN, Nick

DEBUG = True
if DEBUG:
    from circuits import Debugger

IRCCHANNEL = "#realxtend"
NICKNAME = "rexuser"
IRCSERVER = "irc.freenode.net"
PORT = 6667

class Client(Component):
    channel = "ircclient"

    def __init__(self, nick=NICKNAME, ircchannel=IRCCHANNEL, host=IRCSERVER, port=PORT):
        super(Client, self).__init__()

        self.host = host
        self.port = port
        self.hostname = gethostname()

        self.nick = nick
        self.ircchannel = ircchannel

        self += (TCPClient(channel=self.channel) + IRC(channel=self.channel))
        if DEBUG:
            self += Debugger()
            
    def connect(self):
        self.push(Connect(self.host, self.port), "connect")

    def connected(self, host, port):
        print "Connected to %s:%d" % (host, port)

        nick = self.nick
        hostname = self.hostname
        name = "%s on %s using circuits" % (nick, hostname)

        self.push(USER(nick, hostname, host, name))
        self.push(NICK(nick))

    def disconnected(self):
        self.push(Connect(self.host, self.port), "connect")

    def numeric(self, source, target, numeric, args, message):
        if numeric == 1:
            self.push(JOIN(self.ircchannel))
        elif numeric == 433:
            self.nick = newnick = "%s_" % self.nick
            self.push(Nick(newnick), "NICK")

    def join(self, source, channel):
        if source[0].lower() == self.nick.lower():
            print "Joined %s" % channel
        else:
            print "--> %s (%s) has joined %s" % (source[0], source, channel)

    def notice(self, source, target, message):
        print "-%s- %s" % (source[0], message)

    def message(self, source, target, message):
        if target[0] == "#":
            print "<%s> %s" % (target, message)
        else:
            print "-%s- %s" % (source, message)

    #@handler("read", target="stdin")
    #def stdin_read(self, data):
    #    self.push(PRIVMSG(self.ircchannel, data.strip()))

#for standalone testing outside Naali
def main():
    Client(NICKNAME, IRCCHANNEL, IRCSERVER, PORT).run()

if __name__ == "__main__":
    main()
