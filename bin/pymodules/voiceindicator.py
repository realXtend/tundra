
from PythonQt.QtGui import QImage, QSizePolicy, QIcon, QVBoxLayout, QMessageBox, QWidget, QVBoxLayout, QLabel, QPixmap
from PythonQt.QtCore import QSize, QTimer
import naali
import naali
from circuits import Component
import circuits
import PythonQt

class VoiceIndicator:
    def __init__(self, voiceParticipant):
        self._voiceParticipant = voiceParticipant
        if self._voiceParticipant is not None:
            self._voiceParticipant.connect("StartSpeaking()", self.onStartSpeaking)
            self._voiceParticipant.connect("StopSpeaking()", self.onStopSpeaking)

        self._entity = self._getAvatarEntity(self._voiceParticipant.AvatarUUID())
        self._billboard = None
        if self._entity is not None:
            self._setupBillboard()
        else:
            pass

    def getEntity(self):
        return self._entity

    def setEntity(self, ent):
        self._entity = ent
        self._setupBillboard()

    def onStartSpeaking(self):
        if self._billboard is not None:
            self._billboard.SetVisible(True)

    def onStopSpeaking(self):
        if self._billboard is not None:        
            self._billboard.SetVisible(False) 

    def _setupBillboard(self):       
        self._createBillboardWidgets()
        self._billboard = self._entity.GetOrCreateComponentRaw("EC_BillboardWidget", "voiceIndicator", PythonQt.private.AttributeChange.Disconnected, False)        
        self._billboard.SetPosition(0,0,2.5)
        self._billboard.SetWidth(1.45)
        self._billboard.SetHeight(1)
        self._billboard.SetWidget(self._voiceBillboardWidget)
        self._billboard.SetVisible(False) 

    def _createBillboardWidgets(self):
        self.pixmap_label = QLabel()
        self.pixmap_label.size = QSize(512,352)
        self.pixmap_label.scaledContents = True
        self.pixmap_label.setPixmap(QPixmap('./data/ui/images/comm/voiceindicator.png'))
        self._voiceBillboardWidget = self.pixmap_label 

    def _getAvatarEntity(self, uuid):
        entities = naali.getDefaultScene().GetEntitiesWithComponentRaw("EC_OpensimPresence")
        for e in entities:
            presence = e.GetComponentRaw("EC_OpenSimPresence")
            if presence is not None:
                if presence.QGetUUIDString() == uuid:
                    return e
        return None


class PresenceMonitor:
    def __init__(self, ent, handler):
        self._entity = ent
        self._changeHandler = handler
        presence = ent.GetComponentRaw("EC_OpenSimPresence")
        if presence is None:
            return
        presence.connect("OnAttributeChanged(IAttribute*, AttributeChange::Type)", self._onOpenSimPresenceChanged)

    def _onOpenSimPresenceChanged(self):
        self._changeHandler(self._entity)


class VoiceIndicatorManager(Component):
    def __init__(self):
        Component.__init__(self)
        self._commService = naali.communicationsservice
        self._voiceSession = None
        self._indicators = {}
        self._commService.connect("InWorldVoiceAvailable()", self.onNewVoiceSession)

    def onNewVoiceSession(self):
        naali.getDefaultScene().connect("ComponentAdded(Scene::Entity*, IComponent*, AttributeChange::Type)", self._onComponentAdded)    
        self._indicators = {} # map uuid to VoiceIndicator objects
        self._presenceMonitors = []
        self._voiceSession = self._commService.InWorldVoiceSession() 
        if self._voiceSession is not None:
            print self._voiceSession
            print type(self._voiceSession)
            self._voiceSession.connect("ParticipantJoined(Communications::InWorldVoice::ParticipantInterface*)", self.onParticipantJoined)

    def onParticipantJoined(self, p):
        indicator = VoiceIndicator(p)
        self._indicators[p.AvatarUUID()] = indicator

    def _onComponentAdded(self, ent, comp, update):
        if comp.TypeName == "EC_OpenSimPresence":
            uuid = comp.QGetUUIDString()
            if self._indicators.has_key(uuid):
                self._indicators[uuid].setEntity(ent)
            else:
                monitor = PresenceMonitor(ent, self._onPresenceEntityChanged)
                self._presenceMonitors.append(monitor)

    def _onPresenceEntityChanged(self, ent):
        presence = ent.GetComponentRaw("EC_OpenSimPresence")
        uuid = presence.QGetUUIDString()
        if self._indicators.has_key(uuid):
            self._indicators[uuid].setEntity(ent)
