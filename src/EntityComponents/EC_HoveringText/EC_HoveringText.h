/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   EC_HoveringText.h
 *  @brief  Shows a hovering text attached to an entity.
 */

#pragma once

#include "IComponent.h"
#include "Math/float3.h"
#include "Math/float2.h"
#include "OgreModuleFwd.h"
#include "AssetFwd.h"
#include "AssetReference.h"
#include "AssetRefListener.h"
#include "Color.h"

#include <QVector3D>
#include <QFont>
#include <QColor>
#include <QLinearGradient>

class QTimeLine;
class TextureAsset;

namespace Ogre
{
    class BillboardSet;
    class Billboard;
}

/// Shows a hovering text attached to an entity.
/**
<table class="header">
<tr>
<td>
<h2>HoveringText</h2>
Shows a hovering text attached to an entity.

<b>Attributes</b>:

<ul>
<li>QString : text
<div>Text to be shown</div>
<li>QString: font
<div>Font family</div>
<li>Color: fontColor
<div>Font Color</div>
<li>int: fontSize
<div>Font Size</div>
<li>Color: backgroundColor
<div>Color of the background</div>
<li>float3: position
<div>Position of the text on entity</div>
<li>bool: usingGrad
<div>If using Gradient color </div>
<li>Color: gradStart
<div>Gradient start</div>
<li>Color: gradEnd
<div>Gradient End</div>
<li>Color: borderColor
<div>define color of the border</div>
<li>float: borderThickness
<din>define width of border</div>
<li>float2: cornerRadius
<din>The x and y radii of the ellipses defining the corners of the rounded billboard background rectangle. </div>
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>"hide": Hides the hovering text
<li>"show": Shows the hovering text.
<li>"ShowMessage": Sets the text to be shown.
    @param text Text to be shown.
<li>"IsVisible": Returns if the hovering text is visible or not.
   @true If the hovering text is visible, false if it's hidden or not initialized properly.
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>Depends on components Placeable</b>.
</table>
*/
class EC_HoveringText : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_HoveringText",29);

public:
    explicit EC_HoveringText(Scene* scene);

    ~EC_HoveringText();

    Q_PROPERTY(QString text READ gettext WRITE settext);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, text);

    Q_PROPERTY(QString font READ getfont WRITE setfont);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, font);

    Q_PROPERTY(int fontSize READ getfontSize WRITE setfontSize);
    DEFINE_QPROPERTY_ATTRIBUTE(int, fontSize);

    Q_PROPERTY(Color fontColor READ getfontColor WRITE setfontColor);
    DEFINE_QPROPERTY_ATTRIBUTE(Color, fontColor);

    Q_PROPERTY(Color backgroundColor READ getbackgroundColor WRITE setbackgroundColor);
    DEFINE_QPROPERTY_ATTRIBUTE(Color, backgroundColor);

    Q_PROPERTY(Color borderColor READ getborderColor WRITE setborderColor);
    DEFINE_QPROPERTY_ATTRIBUTE(Color, borderColor);

    Q_PROPERTY(float borderThickness READ getborderThickness WRITE setborderThickness);
    DEFINE_QPROPERTY_ATTRIBUTE(float, borderThickness);

    Q_PROPERTY(float3 position READ getposition WRITE setposition);
    DEFINE_QPROPERTY_ATTRIBUTE(float3, position);
    
    Q_PROPERTY(bool usingGrad READ getusingGrad WRITE setusingGrad);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, usingGrad);

    Q_PROPERTY(Color gradStart READ getgradStart WRITE setgradStart);
    DEFINE_QPROPERTY_ATTRIBUTE(Color, gradStart);
    
    Q_PROPERTY(Color gradEnd READ getgradEnd WRITE setgradEnd);
    DEFINE_QPROPERTY_ATTRIBUTE(Color, gradEnd);

    Q_PROPERTY(float overlayAlpha READ getoverlayAlpha WRITE setoverlayAlpha);
    DEFINE_QPROPERTY_ATTRIBUTE(float, overlayAlpha);

    Q_PROPERTY(float width READ getwidth WRITE setwidth);
    DEFINE_QPROPERTY_ATTRIBUTE(float, width);

    Q_PROPERTY(float height READ getheight WRITE setheight);
    DEFINE_QPROPERTY_ATTRIBUTE(float, height);

    Q_PROPERTY(int texWidth READ gettexWidth WRITE settexWidth);
    DEFINE_QPROPERTY_ATTRIBUTE(float, texWidth);

    Q_PROPERTY(float texHeight READ gettexHeight WRITE settexHeight);
    DEFINE_QPROPERTY_ATTRIBUTE(float, texHeight);

    Q_PROPERTY(float2 cornerRadius READ getcornerRadius WRITE setcornerRadius);
    DEFINE_QPROPERTY_ATTRIBUTE(float2, cornerRadius);

    Q_PROPERTY(bool enableMipmapping READ getenableMipmapping WRITE setenableMipmapping);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, enableMipmapping);

    Q_PROPERTY(AssetReference material READ getmaterial WRITE setmaterial);
    DEFINE_QPROPERTY_ATTRIBUTE(AssetReference, material);

    /// Clears the 3D subsystem resources for this object.
    void Destroy();

public slots:
    /// Shows the hovering text.
    void Show();

    /// Hides the hovering text
    void Hide();

    /// Returns if the hovering text is visible or not.
    /** @true If the hovering text is visible, false if it's hidden or not initialized properly. */
    bool IsVisible() const;

    /// Sets the text to be shown.
    /** @param text Text to be shown. */
    void ShowMessage(const QString &text);

    /// Sets postion for the hovering text.
    /** @param position Position as float3.
        @note The position is relative to the entity to which the hovering text is attached. */
    void SetPosition(const float3 &position);

    /// Sets the font used for the hovering text.
    /** @param font Font. */
    void SetFont(const QFont &font);

    /// Sets the color of the chat bubble text.
    /** @param color Color.*/
    void SetTextColor(const QColor &color);

    /// Sets the colors for the background gradient color.
    /** @param startColor Start color.
        @param endColor End color.
        @note Sets the using_gradient_ boolean to true. */
    void SetBackgroundGradient(const QColor &startColor, const QColor &endColor);

    /// Sets the Ogre overlay alpha value. Called in response to when the alpha value attribute changes.
    void SetOverlayAlpha(float alpha);

    /// Updates the billboard world space size.
    void SetBillboardSize(float width, float height);

    /// Gets the name of the material that this component has created for displaying the text.
    /** Useful for using this just to create the material, and using e.g. a mesh to display it. */
    QString GetMaterialName() const { return QString::fromStdString(materialName_); }

private slots:
    /// Redraws the hovering text with the current text, font and color.
    void Redraw();

    /// Called when material asset has been downloaded.
    void OnMaterialAssetLoaded(AssetPtr material);
    
    /// Called when material asset failed to load
    void OnMaterialAssetFailed(IAssetTransfer* transfer, QString reason);

private:
    void AttributesChanged();

    /// To show unique text in each hovering text object, the EC_HoveringText clones the material it has as a reference.
    /// Calling this function deletes the clone.
    void DeleteMaterial();

    /// Recreates the internal cloned material from the currently specified material asset reference (that is assumed to be loaded already).
    void RecreateMaterial();

    /// Ogre world pointer.
    OgreWorldWeakPtr world_;
    
    /// Ogre billboard set.
    Ogre::BillboardSet *billboardSet_;

    /// Ogre billboard.
    Ogre::Billboard *billboard_;

    /// Specifies the name of the private clone of the material this EC_HoveringText component has created for itself, or "" if 
    /// this component does not have a clone created.
    std::string materialName_;

    /// Name of the texture used for the billboard set.
    std::string textureName_;

    /// The font used for the hovering text.
    QFont font_;

    /// Color of the hovering text.
    QColor textColor_;

    /// Gradient background
    QLinearGradient bg_grad_;

    // Texture which contains hovering text
    boost::shared_ptr<TextureAsset> texture_;

    AssetRefListener materialAsset;
};

