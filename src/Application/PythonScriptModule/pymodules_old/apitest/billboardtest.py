'''
Test case for EC_BillboardWidget component

call test_ecbillboard -> empty entity with a billboard component is created.
                      -> widget contains a image and text element 
					  -> billboard size is animated
					  -> widget content is animated (text)
'''
import circuits
import naali
import PythonQt
import time, math
from PythonQt.QtGui import QImage, QSizePolicy, QIcon, QVBoxLayout, QMessageBox, QWidget, QVBoxLayout, QLabel, QPixmap
from PythonQt.QtCore import QSize, QTimer

class MyTest():
	def __init__(self):
		pass
		
	def testBillboard(self):	
		avatar = naali.getUserAvatar()
		avatar_pos = avatar.GetComponentRaw("EC_Placeable")
		
		self.pixmap_label = QLabel()
		self.pixmap_label.size = QSize(200,200)
		self.pixmap_label.scaledContents = True
		self.pixmap_label.setPixmap(QPixmap('./data/ui/images/naali_icon.png'))

		self.text_label = QLabel()
		self.text_label.size = QSize(300,50)
		self.text_label.text = "Hello world"

		self.my_widget = QWidget()
		self.my_widget.setLayout(QVBoxLayout())
		self.my_widget.layout().addWidget(self.pixmap_label)
		self.my_widget.layout().addWidget(self.text_label)
		
		if self.my_widget is None:
			print("Cannot find image file.")
			return
			
		self.my_entity = naali.createEntity()
		position = self.my_entity.GetOrCreateComponentRaw("EC_Placeable", "pos", PythonQt.private.AttributeChange.Disconnected, False)
		position.position = avatar_pos.position
		
		self.ec_billboard = self.my_entity.GetOrCreateComponentRaw("EC_BillboardWidget", "test", PythonQt.private.AttributeChange.Disconnected, False)
		if self.ec_billboard is None:
			print("Cannot create EC_Billboard component.")
			return
	
		self.ec_billboard.SetPosition(0,0,1)
		self.ec_billboard.SetWidth(1)
		self.ec_billboard.SetHeight(1)
		self.ec_billboard.SetWidget(self.my_widget)
		
		self.size_timer = QTimer()
		self.size_timer.connect("timeout()", self.animateSize)
		self.size_timer.start(100)
		 
		self.text_timer = QTimer()
		self.text_timer.connect("timeout()", self.animateText)
		self.text_timer.start(1000)
		
	def animateSize(self):
		min_width = 1.0
		max_width = 2.0
		min_height = 1.0
		max_height = 2.0
		delta_width = max_width-min_width
		delta_height = max_height-min_height
		width = min_width + 0.5*delta_width + 0.5*delta_width*math.sin(time.time()*1.324)
		height = min_height + 0.5*delta_height + 0.5*delta_height*math.cos(time.time()*2.232)
		self.ec_billboard.SetSize(width, height)
		
	def animateText(self):	
		text = str(float(time.time()))
		self.text_label.text = text
		self.ec_billboard.Refresh()


def test_ecbillboard():
	test = MyTest()
	test.testBillboard()
