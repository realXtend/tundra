/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_ChatBubble.h
 *  @brief  EC_ChatBubble Chat bubble component wich shows billboard with chat bubble and text on entity.
*/

#ifndef incl_EC_ChatBubble_EC_ChatBubble_h
#define incl_EC_ChatBubble_EC_ChatBubble_h

#include "ComponentInterface.h"
#include "Declare_EC.h"
#include "Vector3D.h"

#include <QStringList>
#include <QFont>

namespace Ogre
{
    class SceneNode;
    class BillboardSet;
    class Billboard;
}

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

    /// Sets the font used for the chat bubble text.
    /// @param font Font.
    void SetFont(const QFont &font) { font_ = font; }

public slots:
    /// Adds new message to be shown on the chat bubble.
    /// @param msg Message to be shown.
    void ShowMessage(const QString &msg);

private slots:
    /// Removes the last message.
    void RemoveLastMessage();

    /// Removes all the messages.
    void RemoveAllMessages();

    /// Redraws the chat bubble with current messages.
    void Refresh();

private:
    /// Returns pixmap with chat bubble and current messages renderer to it.
    QPixmap GetChatBubblePixmap();

    /// Ogre scene node for this chat bubble billboard.
    Ogre::SceneNode *sceneNode_;

    /// Ogre billboard set.
    Ogre::BillboardSet *billboardSet_;

    /// Ogre billboard.
    Ogre::Billboard *billboard_;

    /// For used for the chat bubble text.
    QFont font_;

    /// List of visible chat messages.
    QStringList messages_;
};

#endif
