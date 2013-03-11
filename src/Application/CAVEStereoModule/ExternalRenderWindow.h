// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <QWidget>

namespace Ogre
{
    class RenderWindow;
}

class QResizeEvent;
class QKeyEvent;

namespace CAVEStereo
{
    class ExternalRenderWindow: public QWidget
    {
        Q_OBJECT
    public:
        ExternalRenderWindow();
        ~ExternalRenderWindow();

        Ogre::RenderWindow* getRenderWindow(){return render_window_;}

        /// Creates a new renderwindow.
        /** @param name of the view (must be unique)
            @param window_width render window width
            @param window_height render window height
            @param left left coordinate of the topleft corner of the window
            @param top top coordinate of the topleft corner of the window
            @param fullscreen this is ignored at the moment.
        */
        Ogre::RenderWindow* CreateRenderWindow(const std::string &name,  int width, int height, int left, int top, bool fullscreen);

    public slots:
        void resizeEvent(QResizeEvent *e);
        void ResizeWindow(int width, int height);
        void keyPressEvent(QKeyEvent *e);

    private:
        Ogre::RenderWindow* render_window_;
    };
}
