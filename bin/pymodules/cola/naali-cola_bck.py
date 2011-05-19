from circuits import Component
import naali
import rexviewer as r
import os
from threading import Thread
import circuits

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
        import threading
                      
        class DownloadGit(QtCore.QThread):
            def __init__(self,actionview,branch,localrepo,repourl):
                QtCore.QThread.__init__(self)
                self.branch = branch
                self.localrepo = localrepo
                self.repourl = repourl
                self.actionview = actionview
                self.git = git.instance()
        
                        
            def run(self):
                try:
                    #qtutils.log(0, 'Starting download...')         
                    list = os.listdir(self.localrepo)
                    
                    valid=True
                    if os.path.exists(self.localrepo): 
                        if list!=[]:
                            #qtutils.log(0, 'Git Error: the local directory is not empty')
                            valid=False
                                    
                    if valid:
                        self.git.clone(self.repourl,self.localrepo) 
                    
                        valid = self.model.use_worktree(self.localrepo)
                        
                        if not valid:
                            qtutils.log(0, 'Git Error: Check the url repository')
                        else:
                            qtutils.log(0, 'Updating...')
                            cola.model.update_status()
                            #cola.cmds.CheckoutBranch(self.branch)
                            self.git.checkout(self.branch)
                            qtutils.log(0, 'Download finished successfully')
        
                    self.actionview.button_close.setEnabled(True)
                    self.actionview.button_hide.setEnabled(False)   
        
                except Exception as ex:
                    print "Error at download action:", ex

        settings = QtCore.QSettings(1, 0, "realXtend", "configuration/Git");
        self.repourl = settings.value('Git/repourl')
        self.branch = settings.value('Git/branch')
        self.localrepo = settings.value('Git/localrepo')
    
        valid = self.model.use_worktree(self.localrepo)
        
        if valid: #Existing git working copy
            qtutils.information('Git cola', 'Update of existing working copy not available')
        else:
           self.actionview = naaliactions.ActionView('Download scripts')
           naali.ui.AddWidgetToScene(self.actionview,False,True)
           naali.ui.ShowWidget(self.actionview)
           download = DownloadGit(self.actionview,self.branch,self.localrepo,self.repourl);
           download.start();
           
    def on_exit(self):
		pass
		#pattern = cola.model().tmp_file_pattern()
		#for filename in glob.glob(pattern):
		#    os.unlink(filename)




            
