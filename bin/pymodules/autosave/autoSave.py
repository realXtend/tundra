from circuits import Component
from PythonQt.QtCore import QFile, Qt, QSettings
from time import strftime

import naali
import os, glob
    
class AutoSave(Component):
  
    def __init__(self):        
        Component.__init__(self)        
        self.entities = []
        self.dirtyEntities = []
        self.removedEntities = []
        self.deleting_files = False
        
        #initial values
        self.settings = QSettings(1, 0, 'realXtend', "configuration/AutoSaveSettings")
        self.time_update = int(self.settings.value("time_update", 120))
        self.save = True
        self.exiting = False
        #path values
        self.path_sys = naali.config.GetConfigFolder()
        self.path_rel = self.settings.value("path", str('TestRecovery/'))       
        self.path_world = 'Default/'
        self.path_temp = str(strftime("%Y%m%d%H%M%S")) + "/"
        
        self.path = self.path_sys + self.path_rel + self.path_world + self.path_temp
        
        d = os.path.dirname(self.path)
        if not os.path.exists(d):
            os.makedirs(d)             
        self.restore = False
                        
    def on_sceneadded(self, name):
        if not naali.server.IsRunning():
            return
        #connect with scene to know 
        self.scene = naali.getScene(name)
        if self.scene is None:
            print "No sceneManager available to connect"
        else:         
            self.scene.connect('EntityCreated(Scene::Entity*, AttributeChange::Type)', self.on_entityCreated)
            self.scene.connect('EntityRemoved(Scene::Entity*, AttributeChange::Type)', self.on_entityRemoved)
            
            self.scene.connect('ComponentAdded(Scene::Entity*, IComponent*, AttributeChange::Type )', self.on_componentAdded)
            self.scene.connect('ComponentRemoved(Scene::Entity*, IComponent*, AttributeChange::Type)', self.on_componentRemoved)
            
            self.scene.connect('AttributeChanged(IComponent*, IAttribute*, AttributeChange::Type)', self.on_attributeChanged)
            self.scene.connect('AttributeAdded(IComponent*, IAttribute*, AttributeChange::Type', self.on_attributeChanged)
            self.scene.connect('AttributeRemoved(IComponent*, IAttribute*, AttributeChange::Type)', self.on_attributeChanged)
            
            self.createSaveEntity()
            self.scene.connect('SceneCleared(Scene::SceneManager*)', self.createSaveEntity)
            naali.framework.application.connect('ExitRequested()', self.on_sceneRemoved)              
            
        print "autoSave initialized properly"

        if(self.save):
            #Connect timing
            naali.frame.DelayedExecute(self.time_update).connect('Triggered(float)', self.on_timeout)
            
    def createSaveEntity(self):
        #initialize entity_saver
        print "entity saver removed!!"
        self.entity_saver_conf = self.scene.CreateEntityRaw(self.scene.NextFreeId(), '', 3, True)            
        self.entity_saver_conf.SetName("Auto_Saver_Entity")
        self.component_saver_conf = self.entity_saver_conf.GetOrCreateComponentRaw("EC_DynamicComponent", 3, True)         
        
        self.component_saver_conf.CreateAttribute('bool', 'save');
        self.component_saver_conf.SetAttribute('save', self.save);
        self.component_saver_conf.CreateAttribute('string', 'path');
        self.component_saver_conf.SetAttribute('path', self.path_rel);
        self.component_saver_conf.CreateAttribute('int', 'time_for_update');
        self.component_saver_conf.SetAttribute('time_for_update', self.time_update);
        
        self.component_saver_conf.connect('OnAttributeChanged(IAttribute* , AttributeChange::Type )', self.on_entitySaverModified)
    
    def on_changelocation(self):
        self.deleting_files = True       
        aux = self.path
        for infile in glob.glob(os.path.join(aux, '*.stxml')):
                os.remove(infile)
        os.rmdir(aux)
        self.deleting_files = False
                    
            
    def on_entityCreated(self, entity, type):
        if entity.IsTemporary() or entity.IsLocal():
            return
        
        if entity.Id == self.entity_saver_conf.Id:
            return
                
        if entity.GetName() == "unique_world_name" and entity.GetDescription != "":
            #not needed because it has been already updated
            if self.restore: 
                return  
            print "Name of world founded!"                   
            self.on_changelocation()
            self.path_world = str(entity.GetDescription()) + "/"
            self.path = self.path_sys + self.path_rel + self.path_world + self.path_temp
            d = os.path.dirname(self.path)
            if not os.path.exists(d):
                os.makedirs(d)                          
            self.dirtyEntities = self.entities[:]
            
        
        if entity.GetName() == "unique_restore_world":
            #Get component
            print "restore previous world"
            self.restore = True
            del self.entities[:]
            del self.dirtyEntities[:]
            del self.removedEntities[:]
            if entity.HasComponent("EC_DynamicComponent", "restore_info"):
                comp = entity.GetComponentRaw("EC_DynamicComponent", "restore_info")
                self.path_world = str(comp.GetAttribute("world_name")) + "/"                
                path_temp_restore = str(comp.GetAttribute("version"))
                if path_temp_restore == "0":
                    for directory in os.listdir(self.path_sys + self.path_rel + self.path_world):
                        if int(path_temp_restore) < int(directory):
                            path_temp_restore = directory
                        
                self.path_restore = self.path_sys + self.path_rel + self.path_world + str(path_temp_restore) + "/"
                self.path = self.path = self.path_sys + self.path_rel + self.path_world + self.path_temp
                d = os.path.dirname(self.path)
                if not os.path.exists(d):
                    os.makedirs(d)
                print "going to restore files from " + str(self.path_restore)
                self.restoreScene()
                return
            
        self.entities.append(entity)
        self.dirtyEntities.append(entity)
        
    
    def on_entityRemoved(self, entity, type):
        if str(type) is not '3':
            return
        
        if entity in self.entities:
            if not entity in self.removedEntities:
                if entity.Id > 0:
                    self.removedEntities.append(entity.Id)
                else:
                    self.removedEntities.append(entity.Id * (-1))          
            self.entities.remove(entity)
    
    def on_componentAdded(self, entity, component, type):
        if str(type) is not '3':
            return
        if entity in self.entities:
            if not entity in self.dirtyEntities:
                self.dirtyEntities.append(entity)
    
    def on_componentRemoved(self, entity, component, type):
        if str(type) is not '3':
            return
        if entity in self.entities:
            if not entity in self.dirtyEntities:
                self.dirtyEntities.append(entity)
    
    def on_attributeChanged(self, component, attribute, type):
        if str(type) is not '3':
            return        
        if component.GetParentEntity() in self.entities:       
            if not component.GetParentEntity() in self.dirtyEntities:
                self.dirtyEntities.append(component.GetParentEntity())
        
    def on_sceneRemoved(self):
        if self.exiting:
            return
        print "Scene going to be removed"
        self.on_timeout()
        self.exiting = True 
    
    def on_exit(self):
        print "AutoSave exiting"
             
        
    def restoreScene(self):
        #Read all the .stxml files in directory and create them
        for infile in glob.glob(os.path.join(self.path_restore, '*.stxml')):
            infile = str(infile).replace('/', '\\')
            self.scene.LoadSceneXMLRaw(infile, False, True, 0)
        self.restore = False
                
    def on_timeout(self):
        print "Timeout reached, lets write files in " + str(self.path)
        if not self.exiting:
            naali.frame.DelayedExecute(self.time_update).connect('Triggered(float)', self.on_timeout)
        
        if not self.save:
            print "save canceled (save to false)"
            return
        if self.restore:
            print "save cancelled (waiting for restore)"
            return
        if self.deleting_files:
            print "save canceled (waiting for the change of directory)"
            return            
                
        for ent_save in self.dirtyEntities:
            if ent_save.Id < 0:
                f = open(self.path + str(ent_save.Id * (-1)) + ".stxml", 'w')
            else:
                f = open(self.path + str(ent_save.Id) + ".stxml", 'w')            
            f.write(str(self.scene.GetEntityXml(ent_save)))
            f.close()
            
        for ent_del in self.removedEntities:
            os.remove(self.path + str(ent_del) + ".stxml")       
        
        #clean array
        del self.dirtyEntities[:]
        del self.removedEntities[:]
        
    def on_entitySaverModified(self, attribute, type):
        if self.path_rel != self.component_saver_conf.GetAttribute('path'):
            self.on_changelocation()
            self.path_rel = str(self.component_saver_conf.GetAttribute('path'))
            self.path = self.path_sys + self.path_rel + self.path_world + self.path_temp
            d = os.path.dirname(self.path)
            if not os.path.exists(d):
                os.makedirs(d)
            self.settings.setValue("path", self.path_rel)
            self.dirtyEntities = self.entities[:]
            
        self.save = self.component_saver_conf.GetAttribute('save');     
        self.time_update = self.component_saver_conf.GetAttribute('time_for_update');
        self.settings.setValue("time_update", int(self.time_update))
        
