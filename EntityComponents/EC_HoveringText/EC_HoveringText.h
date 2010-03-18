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

#include <QFont>
#include <QColor>

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
    void SetBackgroundColor(const QColor &color);

public slots:
    /// Shows the hovering text.
    void Show();

    /// Hides the hovering text
    void Hide();

    /// Returns if the hovering text is visible or not.
    /// @true If the hovering text is visible, false if it's hidden or not initialized properly.
    bool IsVisible() const;

    /// Sets the text to be shown.
    /// @param text Text to be shown.
    void ShowMessage(const QString &text);

private slots:
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

    /// For used for the hovering text.
    QFont font_;

    /// Color of the hovering text.
    QColor textColor_;

    /// Color of the hovering text background.
    QColor backgroundColor_;

    /// The hovering text.
    QString text_;
};

#endif
