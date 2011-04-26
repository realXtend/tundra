from circuits import Component
from PythonQt.QtCore import QFile, Qt #, QSettings
from time import strftime


import rexviewer as r
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
        self.time_update = naali.config.Get("AutoSaveSettings", "time_for_update")
        if self.time_update is None:
            self.time_update = 120
        else:
            self.time_update = int(self.time_update)            
        self.save = True
        self.exiting = False
        
        #path values
        self.path_sys = r.getApplicationDataDirectory() + "/" 
        self.path_rel = naali.config.Get("AutoSaveSettings", "path")
        if self.path_rel is None:
            self.path_rel = str("autoRecovery/")
        else:
            self.path_rel = str(self.path_rel)
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
            r.logError("No sceneManager available to connect")
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
            
        r.logInfo("autoSave initialized properly")

        if(self.save):
            #Connect timing
            naali.frame.DelayedExecute(self.time_update).connect('Triggered(float)', self.on_timeout)
            
    def createSaveEntity(self):
        #initialize entity_saver
        r.logDebug("entity saver removed, let's create a new one")
        self.entity_saver_conf = self.scene.CreateEntityRaw(self.scene.NextFreeId(), '', 3, True)            
        self.entity_saver_conf.SetName("Auto_Saver_Entity")
        self.component_saver_conf = self.entity_saver_conf.GetOrCreateComponentRaw("EC_DynamicComponent", 3, True)         
        
        self.component_saver_conf.CreateAttribute('bool', 'save');
        self.component_saver_conf.SetAttribute('save', self.save);
        self.component_saver_conf.CreateAttribute('string', 'path');
        self.component_saver_conf.SetAttribute('path', self.path_rel);
        self.component_saver_conf.CreateAttribute('int', 'time_for_update');
        self.component_saver_conf.SetAttribute('time_for_update', self.time_update);
        
        self.scene.EmitEntityCreated(self.scene.GetEntity("Auto_Saver_Entity"))
        self.component_saver_conf.connect('OnAttributeChanged(IAttribute* , AttributeChange::Type )', self.on_entitySaverModified)
    
    def on_changelocation(self):
        self.deleting_files = True       
        aux = self.path
        for infile in glob.glob(os.path.join(aux, '*.txml')):
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
            r.logDebug("Name of world founded!")                   
            self.on_changelocation()
            self.path_world = str(entity.GetDescription()) + "/"
            self.path = self.path_sys + self.path_rel + self.path_world + self.path_temp
            r.logInfo("Changing new location of files to: " + self.path)
            d = os.path.dirname(self.path)
            if not os.path.exists(d):
                os.makedirs(d)                          
            self.dirtyEntities = self.entities[:]
            
        
        if entity.GetName() == "unique_restore_world":
            #Get component
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
                r.logInfo("Restore World: going to restore files from %s"  % str(self.path_restore))
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
        r.logWarning("Scene going to be removed, save last changes")
        self.on_timeout()
        self.exiting = True 
    
    def on_exit(self):
        r.logInfo("AutoSave exiting")
             
        
    def restoreScene(self):
        #Read all the .txml files in directory and create them
        for infile in glob.glob(os.path.join(self.path_restore, '*.txml')):
            infile = str(infile).replace('/', '\\')
            self.scene.LoadSceneXMLRaw(infile, False, True, 0)
        self.restore = False
                
    def on_timeout(self):
        r.logDebug("Timeout reached, lets write files in " + str(self.path))
        if not self.exiting:
            naali.frame.DelayedExecute(self.time_update).connect('Triggered(float)', self.on_timeout)
        
        if not self.save:
            r.logWarning("save canceled (save to false)")
            return
        if self.restore:
            r.logWarning("save cancelled (waiting for restore)")
            return
        if self.deleting_files:
            r.logWarning("save canceled (waiting for the change of directory)")
            return            
                
        for ent_save in self.dirtyEntities:
            if ent_save.Id < 0:
                f = open(self.path + str(ent_save.Id * (-1)) + ".txml", 'w')
            else:
                f = open(self.path + str(ent_save.Id) + ".txml", 'w')            
            f.write(str(self.scene.GetEntityXml(ent_save)))
            f.close()
            
        for ent_del in self.removedEntities:
            os.remove(self.path + str(ent_del) + ".txml")       
        
        #clean array
        del self.dirtyEntities[:]
        del self.removedEntities[:]
        
    def on_entitySaverModified(self, attribute, type):
        if self.path_rel != self.component_saver_conf.GetAttribute('path'):
            self.on_changelocation()
            self.path_rel = str(self.component_saver_conf.GetAttribute('path'))
            self.path = self.path_sys + self.path_rel + self.path_world + self.path_temp
            r.logInfo("Changing new location of files to: " + self.path)
            d = os.path.dirname(self.path)
            if not os.path.exists(d):
                os.makedirs(d)
            #self.settings.setValue("path", self.path_rel)
            naali.config.Set("AutoSaveSettings","path", self.path_rel)
            self.dirtyEntities = self.entities[:]
            
        self.save = self.component_saver_conf.GetAttribute('save');     
        self.time_update = self.component_saver_conf.GetAttribute('time_for_update');
        #self.settings.setValue("time_update", int(self.time_update))
        naali.config.Set("AutoSaveSettings","time_for_update", int(self.time_update))
        
