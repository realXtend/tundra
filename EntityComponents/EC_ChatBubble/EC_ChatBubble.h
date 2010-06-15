/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_ChatBubble.h
 *  @brief  EC_ChatBubble Chat bubble component wich shows billboard with chat bubble and text on entity.
 *  @note   The entity must have EC_OgrePlaceable component available in advance.
*/

#ifndef incl_EC_ChatBubble_EC_ChatBubble_h
#define incl_EC_ChatBubble_EC_ChatBubble_h

#include "ComponentInterface.h"
#include "Declare_EC.h"
#include "Vector3D.h"

#include <QStringList>
#include <QFont>
#include <QColor>
#include <QRect>

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
class QTimer;
QT_END_NAMESPACE

/// Chat bubble component wich shows billboard with chat bubble and text on entity.
class EC_ChatBubble : public Foundation::ComponentInterface
{
    Q_OBJECT
    DECLARE_EC(EC_ChatBubble);

private:
    /// Constuctor.
    /// @param module Owner module.
    explicit EC_ChatBubble(Foundation::ModuleInterface *module);

public:
    /// Destructor.
    ~EC_ChatBubble();

    /// Sets postion for the chat bubble.
    /// @param position Position.
    /// @note The position is relative to the entity to which the chat bubble is attached.
    void SetPosition(const Vector3df &position);

    void SetScale(float scale);

    /// Sets the font used for the chat bubble text.
    /// @param font Font.
    void SetFont(const QFont &font) { font_ = font; }

    /// Sets the color of the chat bubble text.
    /// @param color Color.
    void SetTextColor(const QColor &color) { textColor_ = color; }

    /// Sets the color of the chat bubble.
    /// @param color Color.
    void SetBubbleColor(const QColor &color) { bubbleColor_ = color; }

    /// Returns if the chat bubble is visible or not.
    /// @true If the chat bubble text is visible, false if it's hidden or not initialized properly.
    bool IsVisible() const;

public slots:
    /// Adds new message to be shown on the chat bubble.
    /// @param msg Message to be shown.
    /// @note The time the message is shown is calculated from the message length.
    void ShowMessage(const QString &msg);

private slots:
    /// Shows next message from the stack
    void ShowNextMessage();

    /// Removes all the messages.
    void RemoveAllMessages();

    /// Check if messages fit to our rect
    bool CheckMessageSize(QString message);

    /// Create one string containing all the message in the stack.
    QString ConstructCombined() const;

    /// Redraws the chat bubble with current messages.
    void Refresh();

private:
    /// Returns pixmap with chat bubble and current messages renderer to it.
    QPixmap GetChatBubblePixmap();

    /// Renderer pointer.
    boost::weak_ptr<OgreRenderer::Renderer> renderer_;

    /// Ogre billboard set.
    Ogre::BillboardSet *billboardSet_;

    /// Ogre billboard.
    Ogre::Billboard *billboard_;

    /// Name of the material used for the billboard set.
    std::string materialName_;

    /// Name of the texture
    std::string texture_name_;

    /// For used for the chat bubble text.
    QFont font_;

    /// Color of the chat bubble text.
    QColor textColor_;

    /// Color of the chat bubble.
    QColor bubbleColor_;

    /// List of visible chat messages.
    QStringList messages_;

    /// Pop timer
    QTimer *pop_timer_;

    /// Max rext for the rendered image
    QRect bubble_max_rect_;

    /// Currently showed message
    QString current_message_;

    /// Current scale of the billboard
    float current_scale_;
    float default_z_pos_;
};

#endif
