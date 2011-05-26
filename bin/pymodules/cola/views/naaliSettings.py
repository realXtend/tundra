"""
Provides the git-cola settings dialog
"""
import os

from PythonQt import QtGui
from PythonQt import QtCore

from cola import qtutils

class NaaliSettingsDialog(QtGui.QWidget):

    def __init__(self, parent=None):
		QtGui.QWidget.__init__(self, parent)
		self.setWindowTitle(self.tr('Git Settings'))

		print "Settings dialog Title set"
		self._laytv = QtGui.QVBoxLayout()

		self._layth1 = QtGui.QHBoxLayout()
		self._repo_label = QtGui.QLabel('Script repository url')
		settings = QtCore.QSettings(1, 0, "realXtend", "configuration/Git");
		self._repo_url = settings.value('Git/repourl')
		if self._repo_url:
			self._repo_url = QtGui.QLineEdit(self._repo_url)
		else:
			self._repo_url = QtGui.QLineEdit()
		self._layth1.addWidget(self._repo_label)
		self._layth1.addWidget(self._repo_url)

		self._layth2 = QtGui.QHBoxLayout()
		self._branch = settings.value('Git/branch')
		if self._branch:
			self._branch = QtGui.QLineEdit(self._branch)
		else:
			self._branch = QtGui.QLineEdit()
		self._branch_label = QtGui.QLabel('Branch') 
		self._spacer1 = QtGui.QSpacerItem(40,20) 
		self._layth2.addWidget(self._branch_label)
		self._layth2.addItem(self._spacer1)
		self._layth2.addWidget(self._branch)

		self._localrepo = settings.value('Git/localrepo')
		self._gitexedir = settings.value('Git/gitexedir')
		if self._localrepo:
			self._localrepo = self._localrepo.replace('\\', '/')
		if self._gitexedir:            
			self._gitexedir = self._gitexedir.replace('\\', '/')

		self._layth3 = QtGui.QHBoxLayout()
		self._layth3_1 = QtGui.QHBoxLayout()
		self._localrepo_label = QtGui.QLabel('Local script repository folder')  
		self._localrepo_label_path = QtGui.QLabel(self._localrepo)  
		self._localrepo_btn = QtGui.QPushButton('Select repository')
		self._layth3.addWidget(self._localrepo_label)
		self._layth3.addWidget(self._localrepo_btn)
		self._layth3_1.addWidget(self._localrepo_label_path)

		self._layth4 = QtGui.QHBoxLayout()
		self._layth4_1 = QtGui.QHBoxLayout()
		self._gitexedir_label = QtGui.QLabel('Git.exe installation folder')  
		self._gitexedir_label_path = QtGui.QLabel(self._gitexedir)  
		self._gitexedir_btn = QtGui.QPushButton('Select folder')
		self._layth4.addWidget(self._gitexedir_label)
		self._layth4.addWidget(self._gitexedir_btn)
		self._layth4_1.addWidget(self._gitexedir_label_path)
		
		self._layth5 = QtGui.QHBoxLayout()
		self._save_btn = QtGui.QPushButton('Save')
		self._spacer2 = QtGui.QSpacerItem(70,20)
		self._spacer3 = QtGui.QSpacerItem(70,20) 
		self._layth5.addItem(self._spacer2)
		self._layth5.addWidget(self._save_btn)
		self._layth5.addItem(self._spacer3)

		self._laytv.addLayout(self._layth1)
		self._laytv.addLayout(self._layth2)
		self._laytv.addLayout(self._layth3)
		self._laytv.addLayout(self._layth3_1)
		self._laytv.addLayout(self._layth4)
		self._laytv.addLayout(self._layth4_1)
		self._laytv.addLayout(self._layth5)

		self.setLayout(self._laytv)
		print "Setting dialog Layout set"

		self._gitexedir_btn.connect('clicked()', self._opengitexe)
		self._localrepo_btn.connect('clicked()', self._openlocalrepo)
		self._save_btn.connect('clicked()', self._savesettings)
		#@TODO:Fix close
		# self._save_btn.connect('clicked()', 'close()')

		print "Settings dialog buttons connected"

    def _opengitexe(self):
        if self._gitexedir:
            gitexedir = qtutils.opendir_dialog(self,'Select a folder...',self._gitexedir)
        else:
            gitexedir = qtutils.opendir_dialog(self,'Select a folder...',os.getcwd())
            
        if gitexedir:
            self._gitexedir = gitexedir;
			
        print gitexedir
        print self._gitexedir	
        self._gitexedir_label_path.text=self._gitexedir

    def _openlocalrepo(self):
		if self._localrepo:
			localrepo = qtutils.opendir_dialog(self,
										  'Select a folder...',
										  self._localrepo)
		else:
			localrepo = qtutils.opendir_dialog(self,
										  'Select a folder...',
										  os.getcwd())
		if localrepo:
			self._localrepo = localrepo;
		
		print localrepo
		print self._localrepo	
		self._localrepo_label_path.text=self._localrepo

    def _savesettings(self):
		print "Saving values"
		settings = QtCore.QSettings(1, 0, "realXtend", "configuration/Git")
		settings.setValue("Git/localrepo", self._localrepo)
		settings.setValue("Git/gitexedir", self._gitexedir)
		settings.setValue("Git/repourl", self._repo_url.text)
		settings.setValue("Git/branch", self._branch.text)
		print "Values saved"
		self.close();