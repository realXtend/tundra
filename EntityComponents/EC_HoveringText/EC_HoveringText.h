/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_HoveringText.h
 *  @brief  EC_HoveringText shows a hovering text attached to an entity.
 *  @note   The entity must have EC_Placeable component available in advance.
*/

#ifndef incl_EC_HoveringText_EC_HoveringText_h
#define incl_EC_HoveringText_EC_HoveringText_h

#include "IComponent.h"
#include "Declare_EC.h"
#include "Vector3D.h"

#include <QFont>
#include <QColor>
#include <QLinearGradient>

#include "Color.h"

namespace OgreRenderer
{
    class Renderer;
}

namespace Ogre
{
    class SceneNode;
    class BillboardSet;
    class Billboard;
}

QT_BEGIN_NAMESPACE
class QTimeLine;
QT_END_NAMESPACE

/**
<table class="header">
<tr>
<td>
<h2>HoveringText</h2>
HoveringText shows a hovering text attached to an entity.

Registered by RexLogic::RexLogicModule.

<b>Attributes</b>:

<ul>
<li>QString : textAttr
<div>Text to be shown</div>
<li>QString: fontAttr
<div>Font family</div>
<li>Color: fontColorAttr
<div>Font Color</div>
<li>int: fontSizeAttr
<div>Font Size</div>
<li>Color: backgroundColorAttr
<div>Color of the background</div>
<li>Vector3df: positionAttr
<div>Position of the text on entity</div>
<li>bool: usingGradAttr
<div>If using Gradient color </div>
<li>Color: gradStartAttr
<div>Gradient start</div>
<li>Color: gradEndAttr
<div>Gradient End</div>
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>"hide": Hides the hovering text
<li>"show": Shows the hovering text.
<li>"AnimatedShow": Shows the hovering text with animation.
<li>"Clicked": Hovering text is clicked. Toggles the visibility.
	@param msec_to_show Time to show in milliseconds.
<li>"AnimatedHide": Hides the hovering text with animation.
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



/// Shows a hovering text attached to an entity.
class EC_HoveringText : public IComponent
{
    Q_OBJECT
    DECLARE_EC(EC_HoveringText);

private:
    /// Constuctor.
    /// @param module Owner module.
    explicit EC_HoveringText(IModule *module);

public:
    /// Destructor.
    ~EC_HoveringText();

	virtual bool IsSerializable() const { return true; }

    /// Sets postion for the chat bubble.
    /// @param position Position.
    /// @note The position is relative to the entity to which the hovering text is attached.
    void SetPosition(const Vector3df &position);

    /// Sets the font used for the hovering text.
    /// @param font Font.
    void SetFont(const QFont &font);

    /// Sets the color of the chat bubble text.
    /// @param color Color.
    void SetTextColor(const QColor &color);

    /// Sets the background color for the hovering text.
    /// @param color Color.
    /// @note If EC_HoveringText's color is Qt::transparent (default behavior), background is not drawn.
    /// @note Sets the using_gradient_ boolean to false.
    void SetBackgroundColor(const QColor &color);

    /// Sets the colors for the background gradient color.
    /// @param start_color Start color.
    /// @param end_color End color.
    /// @note Sets the using_gradient_ boolean to true.
    void SetBackgroundGradient(const QColor &start_color, const QColor &end_color);


	Q_PROPERTY(QString textAttr READ gettextAttr WRITE settextAttr);
	DEFINE_QPROPERTY_ATTRIBUTE(QString, textAttr);

	Q_PROPERTY(QString fontAttr READ getfontAttr WRITE setfontAttr);
	DEFINE_QPROPERTY_ATTRIBUTE(QString, fontAttr);

	Q_PROPERTY(int fontSizeAttr READ getfontSizeAttr WRITE setfontSizeAttr);
	DEFINE_QPROPERTY_ATTRIBUTE(int, fontSizeAttr);

	Q_PROPERTY(Color fontColorAttr READ getfontColorAttr WRITE setfontColorAttr);
    DEFINE_QPROPERTY_ATTRIBUTE(Color, fontColorAttr);

	Q_PROPERTY(Color backgroundColorAttr READ getbackgroundColorAttr WRITE setbackgroundColorAttr);
	DEFINE_QPROPERTY_ATTRIBUTE(Color, backgroundColorAttr);

	Q_PROPERTY(Color borderColorAttr READ getborderColorAttr WRITE setborderColorAttr);
	DEFINE_QPROPERTY_ATTRIBUTE(Color, borderColorAttr);

    Q_PROPERTY(float borderThicknessAttr READ getborderThicknessAttr WRITE setborderThicknessAttr);
	DEFINE_QPROPERTY_ATTRIBUTE(float, borderThicknessAttr);

	Q_PROPERTY(Vector3df positionAttr READ getpositionAttr WRITE setpositionAttr);
	DEFINE_QPROPERTY_ATTRIBUTE(Vector3df, positionAttr);

	Q_PROPERTY(bool usingGradAttr READ getusingGradAttr WRITE setusingGradAttr);
	DEFINE_QPROPERTY_ATTRIBUTE(bool, usingGradAttr);

	Q_PROPERTY(Color gradStartAttr READ getgradStartAttr WRITE setgradStartAttr);
	DEFINE_QPROPERTY_ATTRIBUTE(Color, gradStartAttr);
	
	Q_PROPERTY(Color gradEndAttr READ getgradEndAttr WRITE setgradEndAttr);
	DEFINE_QPROPERTY_ATTRIBUTE(Color, gradEndAttr);


    /// Clears the 3D subsystem resources for this object.
    void Destroy();

public slots:
    /// Shows the hovering text.
    void Show();

    /// Shows the hovering text with animation.
    void AnimatedShow();

    /// Hovering text is clicked. Toggles the visibility.
    /// @param msec_to_show Time to show in milliseconds.
    void Clicked(int msec_to_show = 5000);

    /// Hides the hovering text
    void Hide();

    /// Hides the hovering text with animation.
    void AnimatedHide();

    /// Returns if the hovering text is visible or not.
    /// @true If the hovering text is visible, false if it's hidden or not initialized properly.
    bool IsVisible() const;

    /// Sets the text to be shown.
    /// @param text Text to be shown.
    void ShowMessage(const QString &text);

private slots:
    /// Updates the animation
    /// @param step Alpha animation step.
    void UpdateAnimationStep(int step);

    /// Finishes the animation.
    void AnimationFinished();

    /// Redraws the hovering text with the current text, font and color.
    void Redraw();
	void UpdateSignals();

    //! Emitted when some of the attributes has been changed.
    void AttributeUpdated(IComponent *component, IAttribute *attribute);

private:
    /// Returns pixmap with chat bubble and current messages renderer to it.
    QPixmap GetTextPixmap();

    /// Renderer pointer.
    boost::weak_ptr<OgreRenderer::Renderer> renderer_;

    /// Ogre billboard set.
    Ogre::BillboardSet *billboardSet_;

    /// Ogre billboard.
    Ogre::Billboard *billboard_;

    /// Name of the material used for the billboard set.
    std::string materialName_;

    /// Name of the texture used for the billboard set.
    std::string textureName_;

    /// The font used for the hovering text.
    QFont font_;

    /// Color of the hovering text.
    QColor textColor_; 

    /// Color of the hovering text background.
    QColor backgroundColor_; 

    /// Gradient background
    QLinearGradient bg_grad_;

    // Visibility animation timeline.
    QTimeLine *visibility_animation_timeline_;

    // Timed visibility timer
    QTimer *visibility_timer_;
};

#endif
