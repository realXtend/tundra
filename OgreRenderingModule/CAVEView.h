// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingModule_CaveView_h
#define incl_OgreRenderingModule_CaveView_h

#include <Ogre.h>
#include <QWidget>

class QResizeEvent;
class QKeyEvent;
namespace OgreRenderer
{

    class Renderer;
    class CAVEView: public QWidget
    {
    public:
        CAVEView(Renderer* renderer);
        ~CAVEView();

        //Initialize AFTER the main camera has already been attached to a scenenode etc.
        void Initialize(const QString& name,  Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos);
        void Initialize(const QString& name, qreal window_width, qreal window_height, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos);
        void ResizeWindow(int width, int height);

        //Assumes that the renderwindow and camera_ have been initialized
        void ReCalculateProjection(Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos);

    protected:
        void resizeEvent(QResizeEvent *e);
        void keyPressEvent(QKeyEvent *e);

    private:
        Ogre::RenderWindow* CreateRenderWindow(const std::string &name,  int width, int height, int left, int top, bool fullscreen);
        Ogre::Camera* camera_;
        Ogre::RenderWindow* render_window_;
        Renderer* renderer_;
    };
}
#endif