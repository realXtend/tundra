// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_QOgreWorldView_h
#define incl_OgreRenderer_QOgreWorldView_h

#include <string>

namespace Ogre
{
    class Root;
    class Viewport;
    class RenderWindow;
    class Overlay;
    class OverlayElement;
    class PixelBox;
}

namespace OgreRenderer
{
    class QOgreWorldView
    {
    public:
        QOgreWorldView(Ogre::RenderWindow *win);
        virtual ~QOgreWorldView();

        void InitializeOverlay(int width, int height);

        void ResizeWindow(int width, int height);
        void ResizeOverlay(int width, int height);

        void RenderOneFrame();
        void OverlayUI(Ogre::PixelBox &ui);

        void ShowUiOverlay();
        void HideUiOverlay();

    private:
        Ogre::Root *root_;
        Ogre::Viewport *view_;
        Ogre::RenderWindow *win_;
        Ogre::Overlay *ui_overlay_;
        Ogre::OverlayElement *ui_overlay_container_;
        std::string texture_name_;
    };
}

#endif // incl_OgreRenderer_QOgreWorldView_h
