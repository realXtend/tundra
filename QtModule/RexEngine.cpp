
#include "RexEngine.h"
#include "RexWidget.h"
#include "Ogre3DWidget.h"

#include <QtWebKit>
#include <QDebug>
#include <QPushButton>
#include <QBoxLayout>
#include <QTextEdit>
#include <QFile>

RexEngine::RexEngine() : pWidget_(0), pController_(0)
{
  // Create OGRE root object

  pRoot_ = new Ogre::Root();
  
  // Show config dialog
  
  if (!pRoot_->showConfigDialog())
    return;
  
  // Init Ogre, no renderwindow yet
  
  pRoot_->initialise(false);

  // Load resource paths from config file
  
  Ogre::ConfigFile cf;
  if ( !QFile::exists("resources.cfg") )
    {
      qDebug()<<"main() : resources.cfg file was not found. ";
      return;
    }
  else
    cf.load("resources.cfg");
  
  // Go through all sections & settings in the file
  
  Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
  
  Ogre::String secName, typeName, archName;
  while (seci.hasMoreElements())
    {
      secName = seci.peekNextKey();
      Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
      Ogre::ConfigFile::SettingsMultiMap::iterator i;
      for (i = settings->begin(); i != settings->end(); ++i)
	{
	  typeName = i->first;
	  archName = i->second;
#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE

	  // OS X does not set the working directory relative to the app,
	  // In order to make things portable on OS X we need to provide
	  // the loading with it's own bundle path location
	  Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
									 String(macBundlePath() 
										+ "/" + archName), 
									 typeName, secName);
#else
	  Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
									 archName, 
									 typeName, 
									 secName);
#endif
	}
    }


}

RexEngine::~RexEngine()
{
  // Some notes: because Ogres memory-allocation-sigleton thingies 
  // we must delete our objects in correct order.

  qDebug()<<"RexEngine::RexEngine() Destructor START";
  delete pRoot_;
  qDebug()<<"RexEngine::RexEngine() Destructor END";
}

void RexEngine::startEngine()
{
  // Now we will generate first Ogre render window which will hold some widgets inside of it as panels. 

  QRect geometry(10,10,800,600);
  pWidget_ = new Ogre3DWidget(0,geometry);
  pWidget_->setWindowTitle("Qt and Ogre together window");
  
  // This widget will be inside of render window (and it is own by render window) 
  
  RexWidget* pBrowser = new RexWidget;
  pBrowser->setPosition(Ogre::Vector3(200,200,0));
  pBrowser->setGeometry(50,50,500,500);
  
  
  QWebView* pView = new QWebView(pBrowser);
  pView->setGeometry(0,0,500,500);
//  pView->load(QUrl("http://www.ampparit.com"));
  pView->show();
  
  // Here we register browser window inside Ogre render window. 
  
  pWidget_->registerWidget(pBrowser);
  
  
  // This widget will be top of render window, but not inside of it 
  
  pController_ = new QWidget(pWidget_);
  
  pController_->setWindowTitle("Browser controller");
  pController_->setGeometry(100,200,250,70);
  
  
  QPushButton *pReload = new QPushButton("&Reload");
  QPushButton *pLoad = new QPushButton("&Load");
  
  
  QHBoxLayout *pBarLayout = new QHBoxLayout;
  pBarLayout->addWidget(pLoad);
  pBarLayout->addWidget(pReload);
  
  QTextEdit *pUrlField = new QTextEdit;
  
  QVBoxLayout *pLayout = new QVBoxLayout;
  
  pLayout->addWidget(pUrlField);  
  pLayout->addLayout(pBarLayout);

  
  pController_->setLayout(pLayout);
  pController_->show();
  
  
  // Here we generate connections between button, view and browser
  
  QObject::connect(pReload, SIGNAL(clicked(bool)), pView, SLOT(reload()));
  QObject::connect(pView,SIGNAL(loadFinished(bool)), pBrowser, SLOT(updateTexture()));
		   
  // Show render window. 
  
  pWidget_->show();  
  


}

void RexEngine::stopEngine()
{
  // Some notes: because Ogres memory-allocation-sigleton thingies 
  // we must delete our objects in correct order.

  qDebug()<<"RexEngine::stopEngine() START";
  
  pWidget_->hide();
  pController_->hide();  
  
  // Now controller parent is Ogre3DWidget, so when it dies it will kill all it children. 
  
  delete pWidget_;
 
  pWidget_ = 0;
  pController_ = 0;

  qDebug()<<"RexEngine::stopEngine() END";

}

void RexEngine::update()
{
  // Our almost endless game loop.
 
  emit updateStart();

  qDebug()<<"RexEngine::update() START";
  qDebug()<<"RexEngine::update() END";
  
  emit updateDone();

}
