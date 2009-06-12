
#ifndef REXENGINE_H_
#define REXENGINE_H_

#include <QObject>
#include <Ogre.h>


class Ogre3DWidget;
class RexEngine : public QObject
{
 Q_OBJECT

 public:
  RexEngine();
  virtual ~RexEngine();

  Ogre3DWidget* getRenderWindow() { return pWidget_; }


 public slots:
  void startEngine();
  void stopEngine();
  void update();

 signals:
  void engineStarted();
  void engineStopped();
  void updateDone();
  void updateStart();

 private:

  // Main render window. 

  Ogre3DWidget* pWidget_;
  QWidget* pController_;
  Ogre::Root* pRoot_;


};


#endif // REXENGINE_H_
