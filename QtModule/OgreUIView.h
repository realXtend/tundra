// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtModule_OgreUIView_h
#define incl_QtModule_OgreUIView_h

#include <Ogre.h>

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QWidget>
#include <QRect>
#include <QDrag>

namespace QtUI
{

/** OgreUIView represents a QGraphicsView that has a Ogre texture surface associated to it.
    This texture surface will be used as the render canvas for a single QGraphicsScene, and the 
    produced surface can later on be composited on the 2D overlay screen or be used as a texture
    on an object in the 3D world.
    \todo Decide whether we want to have the same abstraction as Qt and have the separation of 
    the canvas into two tiers (scene & view). If not necessary, it's more convenient to just
    have a single OgreUICanvas that encompasses and manages both. */
class OgreUIView : public QGraphicsView
{
    Q_OBJECT

public:
    OgreUIView();
    virtual ~OgreUIView();

    /// Call this to render the canvas onto the Ogre surface that is associated to this view.
    /// OgreUIView manages the Ogre overlays by itself so after calling this the new updated
    /// canvas is automatically shown.
    void RenderSceneToOgreSurface();

    /// Sends a MouseMove message to the associated QGraphicsScene widget.
    /// This function is for both hover and drag messages.
    void InjectMouseMove(int x, int y);

    /// Sends a MousePress message to the associated QGraphicsScene widget.
    /// A corresponding MouseRelease needs to be sent afterwards.
    void InjectMousePress(int x, int y);

    /// Sends a MouseRelease message on the QGraphicsScene in the given position.
    void InjectMouseRelease(int x, int y);

    /// Resizes the UI canvas size and reallocates Qt and Ogre surfaces to the new size.
    void SetViewCanvasSize(int width, int height);

    /// Call this to pass in the size of the owning window.
    void SetParentWindowSize(int windowWidth, int windowHeight);

    /// @return The Ogre overlay container associated to this view.
    Ogre::OverlayContainer *GetContainer() const { return container_; }

protected:
    /// Override Qt's QGraphicsView's background drawing to enable alpha on the empty
    /// areas of the canvas.
    void drawBackground(QPainter *painter, const QRectF &rect);

private:
    /// Creates an Ogre Overlay and an Ogre Texture surface of the given size.
    void CreateOgreResources(int width, int height);

    /// Recreates an Ogre texture when the canvas size needs to change.
    void ResizeOgreTexture(int width, int height);

    ///\todo There might be a way to utilize the way that Qt's QGraphicsView
    /// stores these parameters, but they're currently internal/private so
    /// need to track this ourselves.
    ///\todo Possibly refactor these from here to QtModule.cpp to store?
    /// These issues will be revisited when implementing multiple scenes/views.
    int mousePressX;
    int mousePressY;
    bool mouseDown;

    Ogre::TexturePtr texture_;
    Ogre::MaterialPtr material_;
    Ogre::Overlay *overlay_;
    Ogre::OverlayContainer *container_;
};

}

#endif 
