#include "StableHeaders.h"

#include "OgreUIView.h"
#include "OgreUIScene.h"
#include "Profiler.h"

#ifndef Q_WS_WIN
#include <QX11Info>
#endif

#include <QCoreApplication>
#include <QApplication>
#include <QDebug>
#include <QMouseEvent>
#include <QPicture>
#include <QGraphicsSceneEvent.h>

#include <OgreHardwarePixelBuffer.h>
#include <OgreTexture.h>
#include <OgreMaterial.h>
#include <OgreTextAreaOverlayElement.h>
#include <OgreFontManager.h> 
#include <OgrePanelOverlayElement.h>
#include <OgreTextureUnitState.h>

namespace QtUI
{

OgreUIView::OgreUIView()
:overlay_(0),
container_(0),
mouseDown(false)
{
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
}

OgreUIView::~OgreUIView()
{
}

void OgreUIView::SetViewCanvasSize(int width, int height)
{
    this->move(0, 0);
    this->resize(width, height);

    CreateOgreResources(width, height);
}

void OgreUIView::ResizeOgreTexture(int width, int height)
{
    texture_->freeInternalResources();
    texture_->setWidth(width);
    texture_->setHeight(height);
    texture_->createInternalResources();
}

void OgreUIView::CreateOgreResources(int width, int height)
{
    // If we've already created the resources, just resize the texture to a new size.
    if (texture_.get())
    {
        if (width == texture_->getWidth() && height == texture_->getHeight())
            return;
        ResizeOgreTexture(width, height);
        assert(overlay_);
        assert(container_);
        assert(material_.get());
        return;
    }

    overlay_ = Ogre::OverlayManager::getSingleton().create("OgreUIViewOverlay");

    container_ = static_cast<Ogre::OverlayContainer*>(Ogre::OverlayManager::getSingleton()
                                     .createOverlayElement("Panel", "OgreUIViewContainer"));

    // Make the overlay cover 100% of the render window. Note that the UI surface will be 
    // rendered pixel perfect without stretching only if the GraphicsView surface dimension 
    // matches the render window size.
    container_->setPosition(0,0);
    container_->setDimensions(1.0,1.0);

    // Add container in default overlay
    overlay_->add2D(container_);

    ///\todo Tell Ogre not to generate a mipmap chain. This texture only needs to have only one
    /// mip level.
    const char surfaceName[] = "OgreUIViewSurface";
    texture_ = Ogre::TextureManager::getSingleton().createManual(surfaceName, 
						        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
						        Ogre::TEX_TYPE_2D, width, height, 0, 
						        Ogre::PF_A8R8G8B8, Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

    const char surfaceMaterial[] = "OgreUIViewMaterial";
    material_ = Ogre::MaterialManager::getSingleton().create(surfaceMaterial,
						        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    Ogre::TextureUnitState *state = material_->getTechnique(0)->getPass(0)->createTextureUnitState();
    material_->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);
    state->setTextureName(surfaceName);

    container_->setMaterialName(surfaceMaterial);
    container_->show();
    container_->setEnabled(true);
    container_->setColour(Ogre::ColourValue(1,1,1,1));

    overlay_->show();
}

void OgreUIView::SetParentWindowSize(int windowWidth, int windowHeight)
{
    // Compute the appropriate Ogre overlay scale so that the resulting composition will be pixel perfect.
    float relWidth = (float)texture_->getWidth()/windowWidth;
    float relHeight = (float)texture_->getHeight()/windowHeight;

    ///\todo Offer a method for the user to place the canvas as well as resizing it.
    /// For now we do a fixed positioning on the bottom right.
    container_->setPosition(1.f - relWidth, 1.f - relHeight);
    container_->setDimensions(relWidth, relHeight);
}

void OgreUIView::drawBackground(QPainter *painter, const QRectF &rect)
{
    QBrush black(Qt::transparent);
    painter->fillRect(rect, black);
}

void OgreUIView::RenderSceneToOgreSurface()
{
    PROFILE(RenderSceneToOgre);
    // We draw the GraphicsView area to an offscreen QPixmap and blit that onto the Ogre GPU surface.
    QPixmap pixmap(this->size());
    {
        PROFILE(FillEmpty);
        pixmap.fill(Qt::transparent);
    }
    assert(pixmap.hasAlphaChannel());
    QImage img = pixmap.toImage();
//  assert(texture_->getWidth() == img.rect().width());
//  assert(texture_->getHeight() == img.rect().height());
    QPainter painter(&img);
    QRectF destRect(0, 0, pixmap.width(), pixmap.height());
    QRect sourceRect(0, 0, pixmap.width(), pixmap.height());
    {
        PROFILE(RenderUI);
        this->render(&painter, destRect, sourceRect);
    }
    assert(img.hasAlphaChannel());

    ///\todo Can optimize an extra blit away if we paint directly onto the GPU surface.
    Ogre::Box dimensions(0,0, img.rect().width(), img.rect().height());
    Ogre::PixelBox pixel_box(dimensions, Ogre::PF_A8R8G8B8, (void*)img.bits());
    {
        PROFILE(UIToOgreBlit);
        texture_->getBuffer()->blitFromMemory(pixel_box);
    }
}

void OgreUIView::InjectMouseMove(int x, int y)
{
    // Translate the mouse position from QGraphicsView coordinate frame onto
    // the QGraphicsScene coordinate frame.
    QPointF pos = this->mapToScene(x, y);
    QPoint mousePos((int)pos.x(), (int)pos.y());

    QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseMove);
    if (mouseDown)
    {
        mouseEvent.setButtonDownScenePos(Qt::LeftButton, QPoint(mousePressX, mousePressY));
        mouseEvent.setButtonDownScreenPos(Qt::LeftButton, QPoint(mousePressX, mousePressY));
    }
    else
    {
        mouseEvent.setButtonDownScenePos(Qt::NoButton, mousePos);
        mouseEvent.setButtonDownScreenPos(Qt::NoButton, mousePos);
    }
    mouseEvent.setScenePos(mousePos);
    mouseEvent.setScreenPos(mousePos);
    mouseEvent.setLastScenePos(mousePos);
    mouseEvent.setLastScreenPos(mousePos);
    mouseEvent.setButtons(mouseDown ? Qt::LeftButton : Qt::NoButton);
    mouseEvent.setButton(mouseDown ? Qt::LeftButton : Qt::NoButton);
    mouseEvent.setModifiers(0);
    mouseEvent.setAccepted(false);
    QApplication::sendEvent(this->scene(), &mouseEvent);
}

void OgreUIView::InjectMousePress(int x, int y)
{
    // Translate the mouse position from QGraphicsView coordinate frame onto
    // the QGraphicsScene coordinate frame.
    QPointF pos = this->mapToScene(x, y);
    QPoint mousePos((int)pos.x(), (int)pos.y());

    mouseDown = true;
    mousePressX = mousePos.x();
    mousePressY = mousePos.y();

    QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMousePress);
    mouseEvent.setButtonDownScenePos(Qt::LeftButton, mousePos);
    mouseEvent.setButtonDownScreenPos(Qt::LeftButton, mousePos);
    mouseEvent.setScenePos(mousePos);
    mouseEvent.setScreenPos(mousePos);
    mouseEvent.setLastScenePos(mousePos);
    mouseEvent.setLastScreenPos(mousePos);
    mouseEvent.setButtons(Qt::LeftButton);
    mouseEvent.setButton(Qt::LeftButton);
    mouseEvent.setModifiers(0);
    mouseEvent.setAccepted(false);
    QApplication::sendEvent(this->scene(), &mouseEvent);
}

void OgreUIView::InjectMouseRelease(int x, int y)
{
    // Translate the mouse position from QGraphicsView coordinate frame onto
    // the QGraphicsScene coordinate frame.
    QPointF pos = this->mapToScene(x, y);
    QPoint mousePos((int)pos.x(), (int)pos.y());

    mouseDown = false;

    QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseRelease);
    mouseEvent.setButtonDownScenePos(Qt::NoButton, mousePos);
    mouseEvent.setButtonDownScreenPos(Qt::NoButton, mousePos);
    mouseEvent.setScenePos(mousePos);
    mouseEvent.setScreenPos(mousePos);
    mouseEvent.setLastScenePos(mousePos);
    mouseEvent.setLastScreenPos(mousePos);
    mouseEvent.setButtons(Qt::NoButton);
    mouseEvent.setButton(Qt::LeftButton);
    mouseEvent.setModifiers(0);
    mouseEvent.setAccepted(false);
    QApplication::sendEvent(this->scene(), &mouseEvent);
}

}
