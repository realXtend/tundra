

#ifndef REXWIDGET_H_
#define REXWIDGET_H_

#include <QWidget>
#include <QString>
#include <Ogre.h>

class RexWidget : public QWidget
{
  Q_OBJECT
    
 public:
  
  RexWidget(QWidget* parent = 0);
  virtual ~RexWidget();
  
  QString getID() const { return id_;}
  QString getMaterialName() const { return material_;}
  QString getTextureName() const { return texture_; }
  QString getTextureUnitName() const { return texture_unit_; }
 
  void setPosition(const Ogre::Vector3& position) { position_ = position;}
  Ogre::Vector3 getPosition() const { return position_;}

 public slots:
  
  void mouseEvent(QMouseEvent* event);
  void updateTexture();

 signals:
  void textureUpdated();

 protected:
  
  void resizeEvent(QResizeEvent *event);
  void init();

 private:

  QString id_;
  QString material_;
  QString texture_;
  QString texture_unit_;
  bool initialized_;
  Ogre::Vector3 position_;

  Ogre::TexturePtr pTexture_;
  Ogre::MaterialPtr pMaterial_;
  Ogre::TextureUnitState* pState_;


};



#endif // REXWIDGET_H_
