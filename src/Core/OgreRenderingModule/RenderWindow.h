// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "OgreModuleApi.h"

#include <QObject>
#include <string>

class Framework;

namespace Ogre
{
    class RenderWindow;
    class Overlay;
    class OverlayElement;
}

class QImage;

/// Stores the main Ogre::RenderWindow that is created by the Renderer.
class OGRE_MODULE_API RenderWindow : public QObject
{
    Q_OBJECT

public:
    RenderWindow();

    void CreateRenderWindow(QWidget *targetWindow, const QString &name, int width, int height, int left, int top, bool fullscreen, Framework *fw);

    /// Returns the Ogre main RenderWindow used to display the 3D scene in.
    Ogre::RenderWindow *OgreRenderWindow() const;

    /// Returns the 2D Ogre overlay that contains a composition of all 2D widgets.
    Ogre::Overlay *OgreOverlay() const;

//    void RenderFrame();

    std::string OverlayTextureName() const;

signals:
    /// This signal is triggered when the Ogre3D render target has been resized.
    void Resized(int newWidth, int newHeight);

public slots:
    void Resize(int newWidth, int newHeight);

    /// Fully repaints the Ogre 2D Overlay from the given source image.
    void UpdateOverlayImage(const QImage &src);

    /// Shows or hides whether the 2D Ogre Overlay is visible or not.
    void ShowOverlay(bool visible);

    int Width() const;

    int Height() const;

private:
    void CreateRenderTargetOverlay(int width, int height);
    Ogre::RenderWindow *renderWindow;
    Ogre::Overlay *overlay;
    Ogre::OverlayElement *overlayContainer;
};

