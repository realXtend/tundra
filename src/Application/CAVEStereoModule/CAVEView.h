// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "OgreModuleFwd.h"

#include <Ogre.h>

#include <QWidget>

class QResizeEvent;
class QKeyEvent;

namespace CAVEStereo
{
    class ExternalRenderWindow;

    /// This class is responsible of calculating the projection matrices and manage the render window and camera of the view
    class CAVEView : public QObject
    {
        Q_OBJECT

    public:
        explicit CAVEView(const OgreRenderer::RendererPtr &renderer);
        ~CAVEView();

        /*Initialize AFTER the main camera has already been attached to a scene node etc.
        @param name of the view (must be unique)
        @param top_left top left corner of the view
        @param bottom_left bottom left corner of the view
        @param bottom_right bottom right corner of the view
        @param eye_pos position of the eye, relative to view
        */
        void Initialize(const QString& name,  Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos);

        void InitializePanorama(const QString& name, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos, int n);

        /// Initialize AFTER the main camera has already been attached to a scene node etc.
        /** @param name of the view (must be unique)
            @param window_width render window width
            @param window_height render window height
            @param top_left top left corner of the view
            @param bottom_left bottom left corner of the view
            @param bottom_right bottom right corner of the view
            @param eye_pos position of the eye, relative to view
        */
        void Initialize(const QString& name, qreal window_width, qreal window_height, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos);

        void ResizeWindow(int width, int height);

        void InitializePanorama(const QString& name, qreal window_width, qreal window_height, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos, int window_number);

        /// Calculates the custom projection matrix to be used on the rendering
        /** Assumes that the render window and camera_ have been initialized.
            @param top_left top left corner of the view
            @param bottom_left bottom left corner of the view
            @param bottom_right bottom right corner of the view
            @param eye_pos position of the eye, relative to view
        */
        void ReCalculateProjection(Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos);

        /// Returns the window associated with this view
        ExternalRenderWindow* GetExternalRenderWindow() const { return render_window_; }

        /// Returns the parameters used to calculate current projection matrix (provided that the projection matrix has been already calculated at least once)
        /** @param top_left top left corner of the view
            @param bottom_left bottom left corner of the view
            @param bottom_right bottom right corner of the view
            @param eye_pos position of the eye, relative to view
        */
        void GetProjectionParameters( Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos);

    protected:
        void resizeEvent(QResizeEvent *e);
        void keyPressEvent(QKeyEvent *e);

    private:
        Ogre::Vector3 lb, rb, tl, ep;
        Ogre::Camera* camera_;
        ExternalRenderWindow* render_window_;
        OgreRenderer::RendererWeakPtr renderer_;
    };
}
