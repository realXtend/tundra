/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_HoveringText.h
 *  @brief  EC_HoveringText shows a hovering text attached to an entity.
 *  @note   The entity must have EC_OgrePlaceable component available in advance.
*/

#ifndef incl_EC_HoveringText_h
#define incl_EC_HoveringText_h

#include "ComponentInterface.h"
#include "Declare_EC.h"
#include "Vector3D.h"

#include <OgreMaterial.h>

#include <QFont>
#include <QColor>
#include <QBrush>
#include <QTimeLine>

namespace OgreRenderer
{
    class Renderer;
}

namespace Ogre
{
    class SceneNode;
    class BillboardSet;
    class Billboard;
    class MaterialPtr;
}

class EC_HoveringText : public Foundation::ComponentInterface
{
    Q_OBJECT
    DECLARE_EC(EC_HoveringText);

private:
    /// Constuctor.
    /// @param module Owner module.
    explicit EC_HoveringText(Foundation::ModuleInterface *module);

public:
    /// Destructor.
    ~EC_HoveringText();

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
    void SetBackgroundColor(const QColor &color);

    void SetBackgroundGradient(const QColor &color1, const QColor &color2);

public slots:
    /// Shows the hovering text.
    void Show();
    void AnimatedShow();

    /// Hides the hovering text
    void Hide();
    void AnimatedHide();

    /// Returns if the hovering text is visible or not.
    /// @true If the hovering text is visible, false if it's hidden or not initialized properly.
    bool IsVisible() const;

    /// Sets the text to be shown.
    /// @param text Text to be shown.
    void ShowMessage(const QString &text);

private slots:
    void UpdateAnimationStep(int step);
    void AnimationFinished();

    /// Redraws the hovering text with the current text, font and color.
    void Redraw();

private:
    /// Returns pixmap with chat bubble and current messages renderer to it.
    QPixmap GetTextPixmap();

    /// Renderer pointer.
    boost::weak_ptr<OgreRenderer::Renderer> renderer_;

    /// Ogre billboard set.
    Ogre::BillboardSet *billboardSet_;

    /// Ogre billboard.
    Ogre::Billboard *billboard_;

    /// Ogre material for billboard set
    Ogre::MaterialPtr material_;

    /// For used for the hovering text.
    QFont font_;

    /// Color of the hovering text.
    QColor textColor_;

    /// Color of the hovering text background.
    QColor backgroundColor_;

    /// Gradien background
    QLinearGradient bg_grad_;

    /// The hovering text.
    QString text_;

    bool using_gradient_;

    // Visibility animation timeline
    QTimeLine *visibility_animation_timeline_;
};

#endif
