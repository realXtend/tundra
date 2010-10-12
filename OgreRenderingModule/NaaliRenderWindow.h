// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Renderer_NaaliRenderWindow_h
#define incl_Renderer_NaaliRenderWindow_h

#include <QObject>
#include "OgreModuleApi.h"

namespace Ogre
{
class RenderWindow;
class Overlay;
class OverlayElement;
}

class QImage;

/// NaaliRenderWindow stores the main Ogre::RenderWindow that is created by the Renderer.
class OGRE_MODULE_API NaaliRenderWindow : public QObject
{
    Q_OBJECT;
public:
    NaaliRenderWindow();

    void CreateRenderWindow(QWidget *targetWindow, const QString &name, int width, int height, int left, int top, bool fullscreen);

    /// Returns the Ogre main RenderWindow used to display the Naali 3D scene in.
    Ogre::RenderWindow *OgreRenderWindow();

    /// Returns the 2D Ogre overlay that contains a composition of all Naali 2D widgets.
    Ogre::Overlay *OgreOverlay();

//    void RenderFrame();

    /// Fully repaints the Ogre 2D Overlay from the given source image.
    void UpdateOverlayImage(const QImage &src);

    /// Shows or hides whether the 2D Ogre Overlay is visible or not.
    void ShowOverlay(bool visible);

public slots:
    void Resize(int newWidth, int newHeight);

private:
    void CreateRenderTargetOverlay(int width, int height);
    Ogre::RenderWindow *renderWindow;
    Ogre::Overlay *overlay;
    Ogre::OverlayElement *overlayContainer;
};

#endif
