from circuits import Component
import naali
import rexviewer as r
import os
import threading

try:
    from PythonQt import QtCore
    from PythonQt import QtGui
except ImportError:
    print 'Sorry, you do not seem to have PythonQt installed.'

try:

    import cola
    
    from cola.views import startup
    from cola.views import naaliactions
    from cola.views import naaliSettings
            
    from cola import qtutils
    from cola import cmds
    from cola import signals
    from cola import git
    from cola import gitcfg
    from cola import gitcmds
    
except Exception:
    print "Error importing cola modules"    

class Cola(Component):
    def __init__(self):
		Component.__init__(self)

		#Configure Git Path
		try:      
			settings = QtCore.QSettings(1, 0, "realXtend", "configuration/Git");
			gitexedir = settings.value('Git/gitexedir')
			if gitexedir:
				path_entries = os.environ.get('PATH', '').split(os.pathsep)
				path_entries.insert(0, os.path.dirname(gitexedir))
				os.environ['PATH'] = os.pathsep.join(path_entries)
		except Exception:
			print "Error setting git path" 

		# Register model commands
		try:
			cmds.register()
		except Exception:
			print "Error registering model commands" 

		try:
			self.model = cola.model()
		except Exception:
			print "Error creating cola model"
		   
		try:
			down_action = QtGui.QAction('Download scripts', QtGui.QApplication.instance().activeWindow())
			naali.ui.AddExternalMenuAction(down_action, 'Download scripts', 'Scripts')
			down_action.connect('triggered()',self.action_download)

			self.settings_dlg = naaliSettings.NaaliSettingsDialog()
			naali.ui.AddWidgetToScene(self.settings_dlg)
			naali.ui.AddWidgetToMenu(self.settings_dlg, "Git Settings", "Scripts")
            
		except Exception as ex:
			print "Error creating cola menu entries", ex

    def action_download(self):
        '''
        
        '''
        settings = QtCore.QSettings(1, 0, "realXtend", "configuration/Git");
        self.repourl = settings.value('Git/repourl')
        self.branch = settings.value('Git/branch')
        self.localrepo = settings.value('Git/localrepo')
        self.actionview = naaliactions.ActionView('Download scripts')
        naali.ui.AddWidgetToScene(self.actionview,False,True)
        naali.ui.ShowWidget(self.actionview)
        valid = self.model.use_worktree(self.localrepo)
        
        if valid: #Existing git working copy
            qtutils.information('Git cola', 'Update of existing working copy not available')
        else:
           self.git = git.instance()
           self.result = 0
           qtutils.log(0, 'Starting download...')      
           self.loadThread = threading.Thread(target=self.download)
           self.loadThread.start()         
           if naali.debug.IsDebugBuild():
                self.loadThread.join()
                self.write_result()

    def on_exit(self):
		pass
    
    def download(self):
        try:   
            list = os.listdir(self.localrepo)
            
            valid=True
            if os.path.exists(self.localrepo): 
                if list!=[]:
                    self.result = 1
                    valid=False
                            
            if valid:
                self.git.clone(self.repourl,self.localrepo) 
                valid = self.model.use_worktree(self.localrepo)
                if not valid:
                    self.result = 2
                else:
                    self.model.update_status()
                    self.git.checkout(self.branch)
                        
            if not naali.debug.IsDebugBuild():
                self.write_result()
                
        except Exception as ex:
            print "Error at download action:", ex

    def write_result(self):
        if self.result == 0:
            qtutils.log(0, 'Download finished successfully')    
        elif self.result == 1:       
            qtutils.log(0, 'Git Error: the local directory is not empty')
        elif self.result == 2:
            qtutils.log(0, 'Git Error: Check the url repository')

        self.actionview.button_close.setEnabled(True)

