#ifndef incl_Renderer_NaaliRenderWindow_h
#define incl_Renderer_NaaliRenderWindow_h

#include <QObject>

namespace Ogre
{
class RenderWindow;
class Overlay;
class OverlayElement;
}

class QImage;

class NaaliRenderWindow : public QObject
{
    Q_OBJECT;
public:
    NaaliRenderWindow();

    void CreateRenderWindow(QWidget *targetWindow, const QString &name, int width, int height, int left, int top, bool fullscreen);

    Ogre::RenderWindow *OgreRenderWindow();
    Ogre::Overlay *OgreOverlay();

//    void RenderFrame();
    void UpdateOverlayImage(const QImage &src);

public slots:
    void Resize(int newWidth, int newHeight);

private:
    void CreateRenderTargetOverlay(int width, int height);
    Ogre::RenderWindow *renderWindow;
    Ogre::Overlay *overlay;
    Ogre::OverlayElement *overlayContainer;
};

#endif
