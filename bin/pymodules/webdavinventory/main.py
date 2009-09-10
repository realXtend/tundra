'''
Created on 8.9.2009

@author: jonnena
'''

from connection import WebDav
import gui

class MainFunction():

    def __init__(self):
        self.setConnection()
        self.app = gui.Application(self.webdav)
        
    def setConnection(self):
        self.webdav = WebDav()
        
if __name__ == '__main__':
    MainFunction()
