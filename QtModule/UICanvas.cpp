// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "DebugOperatorNew.h"

#include "UICanvas.h"
#include "Profiler.h"

#ifndef Q_WS_WIN
#include <QX11Info>
#endif

#include <QCoreApplication>
#include <QPicture>
#include <QGraphicsSceneEvent>
#include <QUuid>
#include <QWidget>
#include <QGraphicsProxyWidget>
#include <QDebug>
#include <Ogre.h>
#include <QSize>

#include "UILocationPolicy.h"
#include "UIAppearPolicy.h"

#include <OgreHardwarePixelBuffer.h>
#include <OgreTexture.h>
#include <OgreMaterial.h>
#include <OgreTextAreaOverlayElement.h>
#include <OgreFontManager.h> 
#include <OgrePanelOverlayElement.h>
#include <OgreTextureUnitState.h>

#include "MemoryLeakCheck.h"


namespace QtUI
{

UICanvas::UICanvas(DisplayMode mode, const QSize& parentWindowSize): 
    dirty_(true),
    renderwindow_changed_(false),
    surfaceName_(""),
    overlay_(0),
    container_(0),
    state_(0),
    renderWindowSize_(parentWindowSize),
    mode_(mode),
    id_(QUuid::createUuid().toString()),
    locationPolicy_(new UILocationPolicy),
    appearPolicy_(new UIAppearPolicy),
    view_(new UIGraphicsView)
{
    view_->setScene(new QGraphicsScene);
    view_->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    QObject::connect(view_, SIGNAL(ViewResized(QResizeEvent*)),this, SLOT(ResizeEvent(QResizeEvent*)));
    
    if (mode_ == Internal)
    {
        QSize size = view_->size();
        
        CreateOgreResources(size.width(), size.height());
        QObject::connect(view_->scene(),SIGNAL(changed(const QList<QRectF>&)),this,SLOT(Redraw()));
    }
}

UICanvas::~UICanvas()
{
    // We own all the widgets in the scene so free them.
    for (int i = scene_widgets_.size(); i--;)
    {
        QGraphicsProxyWidget* widget = scene_widgets_.takeAt(i);
        delete widget;
    }

    // UICanvas also owns the following Ogre-related resources that are used for
    // compositing the canvases onto the screen. Free them as well.

    // Destroy the Ogre overlay texture and material if they exist.
    if (state_ != 0) 
    {
        Ogre::TextureManager::getSingleton().remove(surfaceName_.toStdString().c_str());
        QString surfaceMaterial = QString("mat") + id_;
        Ogre::MaterialManager::getSingleton().remove(surfaceMaterial.toStdString().c_str());
    }

    // Destroy the Ogre overlay element.
    if (container_)
    {
        QString containerName = QString("con") + id_;
        Ogre::OverlayManager::getSingleton().destroyOverlayElement(containerName.toStdString().c_str());
        container_ = 0;
    }

    // Destroy the Ogre overlay container.
    if (overlay_)
    {
        QString overlayName = QString("over") + id_;
        Ogre::OverlayManager::getSingleton().destroy(overlayName.toStdString().c_str());
        overlay_ = 0;
    }
    
    delete locationPolicy_;
    locationPolicy_ = 0;
    delete appearPolicy_;
    appearPolicy_ = 0;
    delete view_->scene();
    delete view_;
    view_ = 0;
}

QPointF UICanvas::GetPosition() const
{
    if (mode_ == Internal)
        return QPointF(container_->getLeft() * renderWindowSize_.width(),
                container_->getTop() * renderWindowSize_.height());
    else
        return view_->pos();
}

void UICanvas::Resize(int width, int height, Corner anchor)
{
    using namespace std;

    if (mode_ == External) 
    {
        SetSize(width, height);
        return;
    }

    // Resize is called for user-initiated resizing. If the
    // window policy restricts that, return.
    if (!appearPolicy_->IsResizable())
        return;

    QSize current_size = view_->size();

    // Enforce that the new size is appropriate.
    const QSize minimum = view_->minimumSize();
    width = max(minimum.width(), width);
    height = max(minimum.height(), height);

    // Also require at least some positive size if view doesn't have a minimum size.
    width = max(width, 5);
    height = max(height, 5);

    const QSize maximum = view_->maximumSize();
    width = min(width, maximum.width());
    height = min(height, maximum.height());
    
    const int cMaxTextureSize = 2048;
    width = min(width, cMaxTextureSize);
    height = min(height, cMaxTextureSize);

    // Ensure that the Ogre surface is large enough.
    Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().getByName(surfaceName_.toStdString().c_str());
    // Would like to use this for performance, but not possible due to Ogre not supporting sub-surface blits properly.
    /// \todo Modify Ogre so that we can do this, it will avoid unnecessary surface recreation when making the canvas smaller.
//    if (texture->getWidth() < width || texture->getHeight() < height) 
    if (texture->getWidth() != width || texture->getHeight() != height)
        ResizeOgreTexture(width, height);
          
    Ogre::PanelOverlayElement* element = static_cast<Ogre::PanelOverlayElement* >(container_);  
    Ogre::Real top = element->getTop();
    Ogre::Real left = element->getLeft();
   
    int diffWidth = width - current_size.width();
    int diffHeight = height - current_size.height();

    // Adjust the overlay position based on the anchor corner.
    Ogre::Real xPos = left;
    if (anchor == TopRight || anchor == BottomRight)
        xPos = left - diffWidth/Ogre::Real(renderWindowSize_.width());
        
    Ogre::Real yPos = top;
    if (anchor == BottomRight || anchor == BottomLeft)
        yPos = top - diffHeight/Ogre::Real(renderWindowSize_.height());  

    element->setPosition(xPos,yPos);

    // Ogre stores overlay sizes as relative [0, 1] of the main render window.
    // Recompute those.
    float relWidth = (float)width/double(renderWindowSize_.width());
    float relHeight = (float)height/double(renderWindowSize_.height());
    container_->setDimensions(relWidth, relHeight);  
           
    // Update the uv mapping of the overlay texture.
    /// Currently this line is equivalent to setUV(0,0,1,1); since we enforce an exact fit. 
    /// See the comment above about sub-surface blits. \todo Remove this comment once sub-surface
    /// blits are supported.
    element->setUV(0, 0, width/texture->getWidth(), height/texture->getHeight());
        
    // Ensure that the QGraphicsView always matches the size of the canvas.
    view_->resize(width,height);
   
    // Make the root widget of the QGView cover the whole view.
    ResizeWidgets(width, height);

    dirty_ = true;
    RenderSceneToOgreSurface();
}

QPoint UICanvas::MapToCanvas(int x, int y)
{
    if (mode_ == Internal)
    {
        // 1. Map from render window onto the canvas (Ogre overlay/QGraphicsView)
        x -= container_->getLeft() * renderWindowSize_.width();
        y -= container_->getTop() * renderWindowSize_.height();
        // 2. Map from QGraphicsView to QGraphicsScene.
        return view_->mapToScene(QPoint(x,y)).toPoint();
    }
    else // External mode
        return QPoint(x, y);
}

/** This is a workaround to make Qt perform the proper windowing
    logic and animation that is expected. If this is not performed,
    Qt's widgets will not properly render as being active, e.g.
    the blinking keyboard text caret won't render, and selected
    text in a text edit will be painted gray instead of blue. */
void UICanvas::Activate()
{
    // This workaround is not required in external mode - in that case just return.
    if (mode_ == External)
        return;

#ifdef Q_WS_WIN
    QSize current_size = view_->size();
    view_->setWindowFlags(Qt::FramelessWindowHint);
    SetSize(1,1);
    view_->show();
    WId win_id = view_->effectiveWinId();
    ShowWindow(static_cast<HWND>(win_id),SW_HIDE);
    SetSize(current_size.width(), current_size.height());   
#endif 
    ///\todo Figure out what is needed on Linux and Mac. 
    /// Can someone confirm if nothing like this is necessary in those platforms?
}

void UICanvas::BringToTop()
{
    emit ToTop(id_);
}

void UICanvas::PushToBack()
{
    emit ToBack(id_);
}

void UICanvas::SetSize(int width, int height)
{
    view_->resize(width, height);

    if (mode_ == Internal)
    {
        CreateOgreResources(width, height);
        Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().getByName(surfaceName_.toStdString().c_str());
        float relWidth = (float)texture->getWidth()/double(renderWindowSize_.width());
        float relHeight = (float)texture->getHeight()/double(renderWindowSize_.height());
        container_->setDimensions(relWidth, relHeight);
        Ogre::PanelOverlayElement* element = static_cast<Ogre::PanelOverlayElement* >(container_);  

        /// Currently this line is equivalent to setUV(0,0,1,1); since we enforce an exact fit. 
        /// See the comment above about sub-surface blits. \todo Remove this comment once sub-surface
        /// blits are supported.
        element->setUV(0, 0, width/texture->getWidth(), height/texture->getHeight());
        
        ResizeWidgets(width, height);

        // Repaint the canvas. 
        dirty_ = true;
        RenderSceneToOgreSurface();
    }
}

void UICanvas::SetPosition(int x, int y)
{
    if (mode_ == Internal)
    {
        float relX = (double)x / renderWindowSize_.width();
        float relY = (double)y / renderWindowSize_.height();
        container_->setPosition(relX, relY);
/*
        ///\todo Bug? We should be content with the view position fixed to (0,0), which shows
        /// the whole scene, and the widgets are authored so that they stay well in view.
        /// We should *never* have to move the view origin in this function, since this
        /// is only for moving the Ogre coordinates of the canvas.
        if (view_->pos().x() != x || view_->pos().y() != y)
        {
            view_->move(x,y);
            dirty_ = true;
        }*/
    }
    else // External
        view_->move(x,y);
}

bool UICanvas::IsHidden() const
{
    if (mode_ == Internal)
    {
        if (!overlay_)
        {
            assert(false && "Canvas in internal mode needs to have an Ogre overlay!");
            return false;
        }

        ///\todo Refactor so that it uses AppearPolicy.. so that it resolves hidden state depending animation state.
        return !overlay_->isVisible(); 
    }
    else // External mode
    {
        return !view_->isVisible();
    }
}

void UICanvas::Render()
{
    ///\todo The 'container_->isVisible()' will have to be removed at some point since the canvas may be hidden from 2D, 
    /// but be visible on a surface in 3D.
    if (mode_ == Internal) //&& container_->isVisible())
    {
        ///\todo use AppearPolycy
        /*
        if ( fade_ )
        {
            int time = clock_.elapsed();
            if ( time != 0)
            {
                double timeSinceLastFrame = double((time - lastTime_))/1000.0;
                lastTime_ = time;
                Fade(timeSinceLastFrame);
            }
         
        }
        */

        RenderSceneToOgreSurface();    
    }
}

void UICanvas::SetRenderWindowSize(const QSize& size)
{
    QPoint oldpos = GetPosition().toPoint();
//    QPoint oldpos = view_->pos();
    
    renderWindowSize_ = size;
   
    if (mode_ == Internal)
    {  
        Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().getByName(surfaceName_.toStdString().c_str());
        float relWidth = (float)texture->getWidth() /  renderWindowSize_.width();
        float relHeight = (float)texture->getHeight() / renderWindowSize_.height();
        container_->setDimensions(relWidth, relHeight);
     
        // Reset position so that canvas stays pixel-perfect
        SetPosition(oldpos.x(), oldpos.y());

        renderwindow_changed_ = true;
        emit RenderWindowSizeChanged(renderWindowSize_);
    }
}

void UICanvas::Show()
{
    QList<QGraphicsProxyWidget* >::iterator iter = scene_widgets_.begin();
    for(; iter != scene_widgets_.end(); ++iter)
        (*iter)->show();

    if ( mode_ == Internal)
    {        
        ///\todo use appearpolicy here.
        /*
        if ( use_fading_ )
        {
            alpha_ = 0.0;
            current_dur_ = 0.0;
            fade_ = true;
            state_->setAlphaOperation( Ogre::LBX_MODULATE, Ogre::LBS_MANUAL, Ogre::LBS_TEXTURE, alpha_);
        
            clock_.start();
        }
        */
        container_->show();
        overlay_->show();
      
        dirty_ = true;
        RenderSceneToOgreSurface();
    }
    else // External
    {
        view_->show();
    }
    emit Shown();
}

void UICanvas::Hide()
{
    if (mode_ == Internal)
    {
        //todo use appearpolicy here.
        /*
        if ( use_fading_ )
        {
            fade_ = true;
            fade_on_hiding_ = true;
            current_dur_ = total_dur_;
            clock_.restart();
            return;
        }
        */          
        container_->hide();
        overlay_->hide();
        
//        dirty_ = true;
//        RenderSceneToOgreSurface();
    }
    else // External
    {
        QList<QGraphicsProxyWidget* >::iterator iter = scene_widgets_.begin();
        for(; iter != scene_widgets_.end(); ++iter)
            (*iter)->hide();

        view_->hide();
    }

    emit Hidden();
}

void UICanvas::AddWidget(QWidget* widget)
{ 
    QGraphicsScene* scene = view_->scene();
    scene_widgets_.append(scene->addWidget(widget));
}

void UICanvas::AddProxyWidget(QGraphicsProxyWidget *proxy_widget)
{
    QGraphicsScene* scene = view_->scene();
    scene->addItem(proxy_widget);
    // We will not add the proxy widget to scene_widgets_ due we dont have the permission to delete it
    // when canvas is deleted. This is because it was not initialized by UICanvas as it is when adding a QWidget
}

QGraphicsProxyWidget *UICanvas::Remove3DProxyWidget()
{
    QGraphicsItem *item = view_->scene()->items()[0];
    view_->scene()->removeItem(item);
    return (QGraphicsProxyWidget*)item;
}

void UICanvas::SetWindowTitle(const QString& title) 
{
	view_->setWindowTitle(title);
}

void UICanvas::SetWindowIcon(const QIcon& icon) 
{
	view_->setWindowIcon(icon);
}

void UICanvas::SetZOrder(int order)
{
    if (mode_ == Internal)
        overlay_->setZOrder(order);
}

int UICanvas::GetZOrder() const
{
    if (mode_ == Internal)
        return overlay_->getZOrder();
    else
        return -1;
}

void UICanvas::drawBackground(QPainter* painter, const QRectF &rect)
{
    QBrush black(Qt::transparent);
    painter->fillRect(rect, black);
}

void UICanvas::RenderSceneToOgreSurface()
{

    // Render if and only if scene is dirty.
    if ((!dirty_ && !renderwindow_changed_) || mode_ == External)
        return;

    PROFILE(RenderSceneToOgre);

    const QSize canvasSize = view_->size();
    /// \todo Note the '!=' and '<' are deliberate. Want '<' || '<' but can't until we have proper subsurface blits.
    if (scratchSurface.width() != canvasSize.width() || scratchSurface.height() < canvasSize.height())
        scratchSurface = QImage(canvasSize, QImage::Format_ARGB32);

    scratchSurface.fill(Qt::transparent);
    assert(scratchSurface.hasAlphaChannel());

    QPainter painter(&scratchSurface);
    QRectF destRect(0, 0, canvasSize.width(), canvasSize.height());
    QRect sourceRect(0, 0, canvasSize.width(), canvasSize.height());
    {
        PROFILE(RenderUI);
        view_->render(&painter, destRect, sourceRect);
    }

    ///\todo Can optimize an extra blit away if we paint directly onto the GPU surface.
    Ogre::Box sourceBlitRect(0,0, canvasSize.width(), canvasSize.height());
    Ogre::Box destBlitRect(0,0, canvasSize.width(), canvasSize.height());
    Ogre::PixelBox pixel_box(sourceBlitRect, Ogre::PF_A8R8G8B8, (void*)scratchSurface.bits());
    {
        PROFILE(UIToOgreBlit);
        Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().getByName(surfaceName_.toStdString().c_str());
        ///\todo Issue: This blit scales the source rectangle to the destination rectangle (calls D3DXLoadSurfaceFromMemory
        ///             with the option D3DX_FILTER_LINEAR). We want to optimize resize memory usage and not filter. Have to modify Ogre.
        texture->getBuffer()->blitFromMemory(pixel_box, destBlitRect);
    }

    if (renderwindow_changed_)
    {
        renderwindow_changed_ = false;
        dirty_ = true;
    }
    else
    {
        dirty_ = false;
    }

}

void UICanvas::ResizeEvent(QResizeEvent* event)
{
    if ( mode_ != External || scene_widgets_.size() != 1)
        return;

    scene_widgets_[0]->resize(event->size());

}


/** This function resizes the root level widget of the scene to cover the whole QGraphicsView.
    If there are more than one root level widget, this function does nothing. */
void UICanvas::ResizeWidgets(int width, int height)
{
    int root_widgets = 0;
    for(int i = 0; i < scene_widgets_.size(); ++i)
        if (scene_widgets_[i]->parent() == 0)
            ++root_widgets;

    // If there are zero or more than one root widget, do not resize.
    if (root_widgets != 1)
        return;

    ///\todo Also set the proper position!
    for (int i = 0; i < scene_widgets_.size(); ++i)
        if (scene_widgets_[i]->parent() == 0)
            scene_widgets_[i]->resize(QSizeF(width, height));
}

void UICanvas::ResizeOgreTexture(int width, int height)
{
    Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().getByName(surfaceName_.toStdString().c_str());
    texture->freeInternalResources();
    texture->setWidth(width);
    texture->setHeight(height);
    texture->createInternalResources();
}

void UICanvas::CreateOgreResources(int width, int height)
{
    // If we've already created the resources, just resize the texture to a new size.
    if (surfaceName_ != "")
    {
        Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().getByName(surfaceName_.toStdString().c_str());
        if (width == texture->getWidth() && height == texture->getHeight())
            return;
        ResizeOgreTexture(width, height);
        assert(overlay_);
        assert(container_);
        return;
    }

    QString overlayName = QString("over") + id_;
    overlay_ = Ogre::OverlayManager::getSingleton().create(overlayName.toStdString().c_str());

    QString containerName = QString("con") + id_;
    container_ = static_cast<Ogre::OverlayContainer*>(Ogre::OverlayManager::getSingleton()
                                     .createOverlayElement("Panel", containerName.toStdString().c_str()));

    // This is not done so currently -- tuoki 
    // Make the overlay cover 100% of the render window. Note that the UI surface will be 
    // rendered pixel perfect without stretching only if the GraphicsView surface dimension 
    // matches the render window size.
    //container_->setDimensions(1.0,1.0);

    container_->setPosition(0,0);

    // Add container in default overlay
    overlay_->add2D(container_);

    ///\todo Tell Ogre not to generate a mipmap chain. This texture only needs to have only one
    /// mip level.

    surfaceName_ = QString("tex") + id_;

    Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().createManual(surfaceName_.toStdString().c_str(), 
                                Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
                                Ogre::TEX_TYPE_2D, width, height, 0, 
                                Ogre::PF_A8R8G8B8, Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);


    QString surfaceMaterial = QString("mat") + id_;
    Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create(surfaceMaterial.toStdString().c_str(),
                                Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    state_ = material->getTechnique(0)->getPass(0)->createTextureUnitState();
    material->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);
    state_->setTextureName(surfaceName_.toStdString().c_str());

    // Generate pixel perfect texture. 
    float relWidth = (float)texture->getWidth() / renderWindowSize_.width();
    float relHeight = (float)texture->getHeight() / renderWindowSize_.height();
    container_->setDimensions(relWidth, relHeight);

    container_->setMaterialName(surfaceMaterial .toStdString().c_str());
    container_->setEnabled(true);
    container_->setColour(Ogre::ColourValue(1,1,1,1));
}
/*
void UICanvas::Fade(double timeSinceLastFrame )
{
    
    if ( !IsHidden() && state_ != 0 && !fade_on_hiding_)
    {
      
       // If fading in decrease alpha until it reaches 1.0
       state_->setAlphaOperation( Ogre::LBX_MODULATE, Ogre::LBS_MANUAL, Ogre::LBS_TEXTURE, alpha_);
       current_dur_ += timeSinceLastFrame;
       alpha_ = current_dur_ / total_dur_;
       if ( alpha_ > 1.0)   
       {
        
         lastTime_ = 0;
         alpha_ = 1.0;
         state_->setAlphaOperation( Ogre::LBX_MODULATE, Ogre::LBS_MANUAL, Ogre::LBS_TEXTURE, alpha_);
         fade_ = false;
       }
       
    }
    else if ( state_ != 0 && fade_on_hiding_)
    {
        // If fading out increase alpha until it reaches 0.0
        state_->setAlphaOperation( Ogre::LBX_MODULATE, Ogre::LBS_MANUAL, Ogre::LBS_TEXTURE, alpha_);
        current_dur_ -= timeSinceLastFrame;
	    alpha_ = current_dur_ / total_dur_;
	    if( alpha_ < 0.0 )
        {
           fade_ = false;
           
           hide();

            QList<QGraphicsProxyWidget* >::iterator iter = scene_widgets_.begin();
            for (; iter != scene_widgets_.end(); ++iter)
                (*iter)->hide();
          
            container_->hide();
            overlay_->hide();

            fade_on_hiding_ = false;
            emit Hidden();
        }
       
    }
    dirty_ = true;
   
}
*/

}
