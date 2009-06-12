
#ifndef OGRE3DWIDGET_H_
#define OGRE3DWIDGET_H_


#include <QWidget>
#include <Ogre.h>
#include <QRect>
#include <QDrag>

class RexWidget;
class Ogre3DWidget : public QWidget
{
  Q_OBJECT

public:
  
  Ogre3DWidget(QWidget* parent = 0, const QRect& geometry = QRect());
  virtual ~Ogre3DWidget();
  
  void registerWidget(RexWidget* widget);
  Ogre::Entity* entityAt(const QPoint& point);

  static bool exist() { return sceneExist_; }

  enum ShowMode { Plane, Overlay, Window };

protected:

  void paintEvent(QPaintEvent* event);
  void mousePressEvent(QMouseEvent* event); 
  void mouseReleaseEvent(QMouseEvent* event);

  void dropEvent(QDropEvent *event);
  void dragMoveEvent(QDragMoveEvent *event);
  void dragEnterEvent(QDragEnterEvent *event);

private:

  void createScene();
  
  Ogre::RaySceneQuery* createRayQuery(const Ogre::Ray& ray);
  
  void getMeshInformation(const Ogre::MeshPtr mesh,
			  size_t& vertex_count,
			  Ogre::Vector3* &vertices,
			  size_t& index_count,
			  unsigned long* &indices,
			  const Ogre::Vector3& position,
			  const Ogre::Quaternion& orient,
			  const Ogre::Vector3& scale);

  Ogre::RenderWindow* pRenderWindow_;
  Ogre::SceneManager* pSceneManager_;
  Ogre::Camera* pCamera_;
  Ogre::Viewport* pViewport_;
  Ogre::Overlay* pOverlay_;
  Ogre::OverlayContainer* pContainer_;
  
  QString activeChild_;
  ShowMode activeMode_;
  QList<RexWidget* > lstChilds_;
      
  QWidget* pDraggedWidget_;
  QPoint hotSpot_;
  static bool sceneExist_;

};


#endif 
