// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <QObject>
#include "AssetFwd.h"

class Framework;
class TextureAsset;
class OgreMaterialAsset;
class RenderWindow;

namespace Ogre
{
    class Overlay;
    class PanelOverlayElement;
}

/// A UiPlane is a 2D surface composited on the main window using the GPU.
class UiPlane : public QObject
{
    Q_OBJECT

public:
    UiPlane(Framework *fw, RenderWindow *renderWindow);
    ~UiPlane();

public slots:
    void Show();

    void Hide();

    /// Specifies the compositing order of this UiPlane.
    /** Higher values are in the front, smaller values go back.
        This value must be in the range [0, 650]. Passing a value outside that range gets clamped to the range.
        The main 2D Qt UI has a z order of 500, so setting a value larger than 500
        shows this plane on top of the 2D UI.
        The default-initialized Z order value is 250. */
    void SetZOrder(int zOrder);

    int ZOrder() const;

    /// Sets the opacity of this UiPlane. 1.0f: Fully opaque, 0.0f: Not visible.
    /** The default-initialized value is 1.0f. */
    void SetAlpha(float alpha);

    float Alpha() const;

    /// Sets the X coordinate of this UiPlane.
    /** @param x The position. If absolutePixels == false, x is interpreted relative to the main window width, and must be in the range [0,1].
                 If absolutePixels == true, this value is interpreted in absolute pixel coordinates.
        @param anchorX One of {-1, 0, 1}. If -1 is passed, the x coordinate is interpreted relative to the left side of the screen.
                 If 0 is passed, the x coordinate will be relative to the center of the screen.
                 If 1 is passed, the x coordinate will be relative to the right edge of the screen.
        @note The default value for x coordinate of newly created UiPlanes is x == 0, absolutePixels == false, anchorX == -1. */
    void SetX(float x, bool absolutePixels = false, int anchorX = -1);

    /// Sets the Y coordinate of this UiPlane.
    /** @param y The position. If absolutePixels == false, y is interpreted relative to the main window height, and must be in the range [0,1].
                 If absolutePixels == true, this value is interpreted in absolute pixel coordinates.
        @param anchorY One of {-1, 0, 1}. If -1 is passed, the y coordinate is interpreted relative to the top side of the screen.
                 If 0 is passed, the y coordinate will be relative to the center of the screen.
                 If 1 is passed, the y coordinate will be relative to the bottom edge of the screen.
        @note The default value for y coordinate of newly created UiPlanes is y == 0, absolutePixels == false, anchorY == -1. */
    void SetY(float y, bool absolutePixels = false, int anchorY = -1);

    /// Sets the width of this UiPlane.
    /** @param width The new width. If absolutePixels == false, width is interpreted relative to the main window width, and must be in the range [0,1].
                 If absolutePixels == true, this value is interpreted in absolute pixel coordinates.
        @note The default value for the width of newly created UiPlanes is width == 1, absolutePixels == false. */
    void SetWidth(float width, bool absolutePixels = false);

    /// Sets the height of this UiPlane.
    /** @param height The new height. If absolutePixels == false, height is interpreted relative to the main window height, and must be in the range [0,1].
                 If absolutePixels == true, this value is interpreted in absolute pixel coordinates.
        @note The default value for the height of newly created UiPlanes is height == 1, absolutePixels == false. */
    void SetHeight(float height, bool absolutePixels = false);

    float X() const { return x.value; }
    bool XIsAbsolute() const { return x.absolute; }

    float Y() const { return y.value; }
    bool YIsAbsolute() const { return y.absolute; }

    float Width() const { return width.value; }
    bool WidthIsAbsolute() const { return width.absolute; }

    float Height() const { return height.value; }
    bool HeightIsAbsolute() const { return height.absolute; }

    int AnchorX() const { return anchorX; }
    int AnchorY() const { return anchorY; }

//    void SetMaterial(OgreMaterialAsset *material);
    void SetTexture(TextureAsset *texture);

    /// Refreshes all internal parameters of the Ogre overlay material to match the currently set state.
    /// One shouldn't ever need to call this function manually (but exposed still to allow debugging potential problems).
    void UpdateOgreOverlay();

    /// Refreshes the position and size of the overlay rectangle into Ogre3D.
    /// One shouldn't ever need to call this function manually (but exposed still to allow debugging potential problems).
    void UpdateOgreOverlayGeometry();

private:
    /// The ogre overlay. This pointer is never null, and is always valid for the lifetime of this UiPlane.
    Ogre::Overlay *overlay;

    /// The ogre overlay element. This pointer is never null, and is always valid for the lifetime of this UiPlane.
    Ogre::PanelOverlayElement *overlayElement;

    struct AbsOrRelValue
    {
        float value;
        bool absolute; // If true, this value is in absolute units (pixels). If false, then relative (screen size).

        AbsOrRelValue()
        :value(0.f), absolute(false)
        { }

        float ToRelative(int pixelUnits) const { return absolute ? value / pixelUnits : value; }
    };

    AbsOrRelValue x;
    AbsOrRelValue y;
    AbsOrRelValue width;
    AbsOrRelValue height;
    int anchorX;
    int anchorY;

    weak_ptr<OgreMaterialAsset> materialAsset;
    weak_ptr<TextureAsset> textureAsset;

    ///\note Ideally, would not want to cache this, but directly query the material.
    float alpha;

    Framework *fw;
    RenderWindow *rw;
};
