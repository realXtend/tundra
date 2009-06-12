#include "RexWidget.h"
#include "Ogre3DWidget.h"

#include <QUuid>
#include <QMouseEvent>
#include <QDebug>


RexWidget::RexWidget(QWidget* parent) : QWidget(parent), id_(""), material_(""), texture_(""), texture_unit_(""), initialized_(false)
{

  // Generate unique id.

  id_ = QUuid::createUuid().toString();
  
  // Because we cannot create textures etc, without that render window exist we must done this kind trick. 

  if (Ogre3DWidget::exist())
    init();
    
}

RexWidget::~RexWidget()
{
  if ( initialized_)
    {
      Ogre::TextureManager::getSingleton().remove(texture_.toStdString().c_str());
      Ogre::MaterialManager::getSingleton().remove(material_.toStdString().c_str());
    }
}

void RexWidget::init()
{
  qDebug()<<"RexWidget::init() START";
  
  // Unique material, depends on id_ 
  
  material_ = "mat";
  material_ += id_;
  texture_ = "text";
  texture_ += id_;

  QPixmap pixmap(this->size());
  this->render(&pixmap);
  QImage img = pixmap.toImage();
  
  //img.save("widget.png");
  
  Ogre::PixelFormat format = Ogre::PF_A8R8G8B8;


  pTexture_ = Ogre::TextureManager::getSingleton().createManual(texture_.toStdString().c_str(), 
								Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
								Ogre::TEX_TYPE_2D,
								img.rect().width(), 
								img.rect().height(), 
								0, 
								format,
								Ogre::TU_DEFAULT);


  Ogre::Box dimensions(0,0, img.rect().width(), img.rect().height());
  Ogre::PixelBox pixel_box(dimensions, format, (void*)img.bits());
  pTexture_->getBuffer()->blitFromMemory(pixel_box);   
  

  pMaterial_ = Ogre::MaterialManager::getSingleton().create( material_.toStdString().c_str(),
							    Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);


  texture_unit_ = "unit";
  texture_unit_ += id_;

  pState_ =  pMaterial_->getTechnique(0)->getPass(0)->createTextureUnitState(texture_unit_.toStdString().c_str());
  pState_->setTextureName(texture_.toStdString().c_str());
  initialized_ = true;

   qDebug()<<"RexWidget::init() END";
}


void RexWidget::mouseEvent(QMouseEvent* event)
{
  qDebug()<<"RexWidget::mouseEvent() START";
 
  bool update = false;
  
  switch ( event->type())
    {
    case QEvent::MouseButtonPress:
      {
	mousePressEvent(event);
	update = true;
	break;
      }
    default:
      break;
    }

  if (update && initialized_)
    updateTexture();
  else if (update && Ogre3DWidget::exist())
    {
      init();
      updateTexture();
      
    }
    
  qDebug()<<"RexWidget::mouseEvent() END";
}

void RexWidget::updateTexture()
{
  qDebug()<<"RexWidget::updateTexture() START";
 
  // Save widgets internal data into pixmap and update texture.
  
  Ogre::PixelFormat format = Ogre::PF_A8R8G8B8;
  QPixmap pixmap(this->size());
  
  this->render(&pixmap);
  QImage img = pixmap.toImage();
  
  Ogre::Box dimensions(0,0, img.rect().width(), img.rect().height());
  Ogre::PixelBox pixel_box(dimensions, format, (void*)img.bits());
  pTexture_->getBuffer()->blitFromMemory(pixel_box);  

  
  emit textureUpdated();
 
  qDebug()<<"RexWidget::updateTexture() END";
}



void RexWidget::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event);

  // Now we must delete old texture and create new texture.
  // Note if here is called render() -function it might start recursive painting call which we do not want.

}

