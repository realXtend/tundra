#some imports naturally
import rex #or what ever the module is called

class Tooltip: #maybe some parent perhaps ?(object):
    
    def __init__(self): #params?
        self.tooltip = None #changed whether the tooltip is for an object or an Avatar ?
        
        self.doneMouseOver = False #flag to prevent multiple server queries for one object 
        
        """
        Need to register the types that are tooltipped, i.e. avatars, items, etc.
        
        """
        
    #called when the mouse hovers over some object which is registered before hand
    def onMouseEnter(self, evt):
        """
            evt has the (x,y) coordinates and the target under the mouse cursor
            
            the getItem function in the rex-module whatnot, returns an object presentation
            of the item from under the mouse. could possibly have the following attributes:
            
                itemid      if items are identified with one?
                name        well every item does have a name, can't go about looking at <nondescript thingie>s
                desc        description of the said item
                price       generally, stuff sold by vendors do have a price they are sold (depending on the 
                            context, the selling prices is half of this)
                                
        
        """
        if evt.target.Type() == "ITEM": 
            self.doneMouseOver = True
            self.setToolTip(evt.target, tiptype=TooltipFrameItem)
        
        """
            could prolly use a more generalized function for getting the stuff for items and avatars, but wanted
            to make a distinction between different objects in the virtual world / game.
            
            like getItem, getAvatar returns an object presentation of the avatar in question (under the mouse cursor)
            could have the following attributes:
            
                avatarid    if avatars are identified with one?
                name        whats an avatar without a name
                desc        not really needed, is it?
                npc         flag, true for nonplayer character etc...
            
        """
        if evt.target.Type() == "AVATAR": #so now we're hovering over an avatar, be it a NPC or a PC
            self.doneMouseOver = True
            self.setToolTip(evt.target, tiptype=TooltipFrameAvatar)
        else:
            return #shouldn't get here in theory, WHEN every type of obj, avatar etc is slapped in here
            
    def onMouseLeave(self):
        self.hideTooltip()    
    
    def setTooltipItem(self, obj, tiptype):
        self.tooltip = tiptype(obj)
        self.showTooltip()
        
    def showTooltip(self):
        self.tooltip.Show()
        
    def hideTooltip(self):
        self.tooltip.Hide()
        self.doneMouseOver = False


class TooltipFrame:
    WIDTH = 4
    HEIGHT = 4
    
    def __init__(self, name="<nondescript thingie>", desc="<insert flashy description here>"):
        self.frame = lotsOfNiftyCodeHere(self.WIDTH, self.HEIGHT) #create frame with self.HEIGHT and self.WIDTH
        self.frame.Hide()
        self.name = name
        self.desc = desc
        self.setFrame()
        
    def Show(self):
        self.frame.Show()
        
    def Hide(self):
        self.frame.Hide()

    def setFrame(self):
        #sets the name and desc into the frame
        
#not sure if we'd actually need a different class for different objects... oh well, bad habbit i guess       
class TooltipFrameAvatar(TooltipFrame):
    def __init__(self, avatar):
        TooltipFrame.__init__(self, avatar.name(), avatar.desc())
        self.npc  = avatar.npc() #returns true if the avatar is a NPC
        self.setFrame()
        
    def setFrame(self):
        #whether the avatar is a npc or not, it'll add a <NPC> tag after the name
        if self.npc:
            self.name += " <NPC>"

class TooltipFrameItem(TooltipFrame):
    def __init__(self, item):
        TooltipFrame.__init__(self, item.name(), item.desc())
        self.price = item.price()
        self.invAmount = rex.getItemCountInInventory(item.id())
                
    def setFrame(self):
        #adds the price and amount of the item in the inventory (if any) into the tooltip frame
        
