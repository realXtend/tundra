// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingModule_CaveView_h
#define incl_OgreRenderingModule_CaveView_h

#include <Ogre.h>
#include <QWidget>

class QResizeEvent;
class QKeyEvent;

//!this class is responsible of calculating the projection matrices and manage the render window and camera of the view
namespace OgreRenderer
{

    class Renderer;
    class CAVEView: public QWidget
    {
    public:
        CAVEView(Renderer* renderer);
        ~CAVEView();

        /*Initialize AFTER the main camera has already been attached to a scenenode etc.
		@param name of the view (must be unique)
			@param top_left topleft corner of the view
			@param bottom_left bottom left corner of the view
			@param bottom_right bottom right corner of the view
			@param eye_pos position of the eye, relative to view
		*/
        void Initialize(const QString& name,  Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos);
		/*Initialize AFTER the main camera has already been attached to a scenenode etc.
		@param name of the view (must be unique)
		@param window_width render window width
		@param window_height render window height
			@param top_left topleft corner of the view
			@param bottom_left bottom left corner of the view
			@param bottom_right bottom right corner of the view
			@param eye_pos position of the eye, relative to view
		*/
        void Initialize(const QString& name, qreal window_width, qreal window_height, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos);
        void ResizeWindow(int width, int height);

        /*Assumes that the renderwindow and camera_ have been initialized. Calculates the custom projection matrix to be used on the rendering
					@param top_left topleft corner of the view
			@param bottom_left bottom left corner of the view
			@param bottom_right bottom right corner of the view
			@param eye_pos position of the eye, relative to view
		
		*/
        void ReCalculateProjection(Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos);

    protected:
        void resizeEvent(QResizeEvent *e);
        void keyPressEvent(QKeyEvent *e);

    private:

		/*
		creates a new renderwindow.
		@param name of the view (must be unique)
		@param window_width render window width
		@param window_height render window height
		@param left left coordinate of the topleft corner of the window
		@param top top coordinate of the topleft corner of the window
		@param fullscreen this is ignored at the moment.
		*/
        Ogre::RenderWindow* CreateRenderWindow(const std::string &name,  int width, int height, int left, int top, bool fullscreen);
        Ogre::Camera* camera_;
        Ogre::RenderWindow* render_window_;
        Renderer* renderer_;
    };
}
#endif