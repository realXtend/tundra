/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_ChatBubble.cpp
 *  @brief  EC_ChatBubble Chat bubble component wich shows billboard with chat bubble and text on entity.
 *  @note   The entity must have EC_OgrePlaceable component available in advance.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EC_ChatBubble.h"
#include "ModuleInterface.h"
#include "Renderer.h"
#include "EC_OgrePlaceable.h"
#include "Entity.h"
#include "OgreMaterialUtils.h"
#include "LoggingFunctions.h"

DEFINE_POCO_LOGGING_FUNCTIONS("EC_ChatBubble");

#include <Ogre.h>
#include <OgreBillboardSet.h>
#include <OgreTextureManager.h>
#include <OgreResource.h>

#include <QFile>
#include <QPainter>
#include <QLinearGradient>
#include <QTimer>

#include "MemoryLeakCheck.h"

EC_ChatBubble::EC_ChatBubble(Foundation::ModuleInterface *module) :
    Foundation::ComponentInterface(module->GetFramework()),
    font_(QFont("Arial", 50)),
    bubbleColor_(QColor(48, 113, 255, 255)),
    textColor_(Qt::white),
    billboardSet_(0),
    billboard_(0),
    pop_timer_(new QTimer(this)),
    bubble_max_rect_(0,0,1000,500),
    current_scale_(1.0f),
    default_z_pos_(1.9f)
{
    // Get renderer service
    renderer_ = module->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>();

    // Pop timer init
    pop_timer_->setSingleShot(true);
    connect(pop_timer_, SIGNAL(timeout()), SLOT(ShowNextMessage()));

    connect(this, SIGNAL(ParentEntitySet()), this, SLOT(Update()));
}

EC_ChatBubble::~EC_ChatBubble()
{
    if (!renderer_.expired())
        Ogre::TextureManager::getSingleton().remove(texture_name_);
}

void EC_ChatBubble::SetPosition(const Vector3df& position)
{
    if (billboard_)
        billboard_->setPosition(Ogre::Vector3(position.x, position.y, position.z));
}

void EC_ChatBubble::SetScale(float scale)
{
    // Round the scale consist only to two decimals, other we get unwanted jittering
    scale *= 100;
    scale = floorf(scale);
    scale /=100;

    if (!billboardSet_ || !billboard_ || (current_scale_ == scale))
        return;

    // Make scale go inside our range
    clamp(scale, 0.5f, 2.5f);

    // Update dimension
    billboardSet_->setDefaultDimensions(2*scale, 1*scale);

    // Update position
    Ogre::Vector3 position = billboard_->getPosition();
    if (scale <= 1.0)
        position.z = default_z_pos_ - (1.0-scale);
    else if (scale > 1.0 && scale <= 1.7)
        position.z = default_z_pos_;
    else if (scale > 1.7 && scale <= 2.5)
        position.z = default_z_pos_ + (scale - 1.7);
    else if (scale > 2.5)
        position.z = default_z_pos_ + (2.5 - 1.7);
    billboard_->setPosition(position);

    current_scale_ = scale;
}

bool EC_ChatBubble::IsVisible() const
{
    if (billboardSet_)
        return billboardSet_->isVisible();
    else
        return false;
}

void EC_ChatBubble::ShowMessage(const QString &msg)
{
    if (msg.isNull() || msg.isEmpty())
        return;

    if (!billboardSet_ && !billboard_)
        Update();
    if (!billboardSet_ && !billboard_)
        return;

    // Push message to queue and update rendering
    messages_.push_back(msg);
    ShowNextMessage();
}

void EC_ChatBubble::ShowNextMessage()
{
    int count_popped = 0;

    // Return if nothing available and hide bubble
    if (messages_.isEmpty())
    {
        billboardSet_->setVisible(false);
        current_message_ = "";
        return;
    }

    // If no timer are running, current text can be reseted
    // and we pick up new messages from queue
    if (!pop_timer_->isActive())
        current_message_ = "";
    
    // Pop as many as we can get
    QString all_messages = ConstructCombined();
    if (CheckMessageSize(all_messages))
    {
        // If the text fits, lets show it
        current_message_ = all_messages;
        count_popped = messages_.count();

        // Update rendering with current_message_
        Refresh();
        pop_timer_->start(count_popped * 4000);

        // Remove popped ones from queue
        for (int index=0; index<count_popped; index++)
            messages_.pop_front();
    }
    else
    {
        // If combined did not fit, lets try again on next update
        if (!pop_timer_->isActive())
        {
            // If timer is not active and all messages in queue did not fit
            // lets take one at a time.
            current_message_ = messages_.front();
            Refresh();
            messages_.pop_front();
            pop_timer_->start(4000);
        }
    }
}

void EC_ChatBubble::RemoveAllMessages()
{
    messages_.clear();
    Refresh();
}

bool EC_ChatBubble::CheckMessageSize(const QString &message)
{
    // Get padding from font metrics
    QFontMetrics metric(font_);
    int padding = metric.averageCharWidth();

    // Text rect
    QRect text_boundaries(padding, padding, bubble_max_rect_.width()-(2*padding), bubble_max_rect_.height()-(2*padding));
    QRect text_rect = metric.boundingRect(text_boundaries, Qt::AlignCenter | Qt::TextWordWrap, message);
    if (text_rect.width() > bubble_max_rect_.width() || text_rect.height() > bubble_max_rect_.height())
        return false;
    else
        return true;
}

QString EC_ChatBubble::ConstructCombined() const
{
    // Format queued strings with line ending
    QStringList queued = messages_;
    QString queued_formatted;
    foreach(QString line, queued)
        queued_formatted += line.append('\n');
    queued_formatted = queued_formatted.left(queued_formatted.lastIndexOf('\n'));

    // Join current and upcoming
    QString all_messages;
    if (!current_message_.isEmpty())
    {
        all_messages = current_message_;
        all_messages.append('\n');
        all_messages.append(queued_formatted);
    }
    else
        all_messages = queued_formatted;

    return all_messages;
}

void EC_ChatBubble::Refresh()
{
    if (renderer_.expired() || !billboardSet_ || !billboard_)
        return;

    // If no messages in the log, hide the chat bubble.
    if (messages_.isEmpty())
    {
        billboardSet_->setVisible(false);
        return;
    }
    else
        billboardSet_->setVisible(true);

    // Get image buffer and texture
    QImage buffer = GetChatBubblePixmap().toImage();
    Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().getByName(texture_name_);
    if (buffer.isNull() || texture.isNull())
        return;

    // Check texture size
    if (texture->getWidth() != buffer.width() || texture->getHeight() != buffer.height())
    {
        texture->freeInternalResources();
        texture->setWidth(buffer.width());
        texture->setHeight(buffer.height());
        texture->createInternalResources();
    }

    // Update texture buffer
    Ogre::Box update_box(0,0, buffer.width(), buffer.height());
    Ogre::PixelBox pixel_box(update_box, Ogre::PF_A8R8G8B8, (void*)buffer.bits());
    texture->getBuffer()->blitFromMemory(pixel_box, update_box);
}

void EC_ChatBubble::Update()
{
    if (renderer_.expired())
        return;

    Ogre::SceneManager *scene = renderer_.lock()->GetSceneManager();
    assert(scene);
    if (!scene)
        return;

    Scene::Entity *entity = GetParentEntity();
    assert(entity);
    if (!entity)
        return;

    OgreRenderer::EC_OgrePlaceable *node = entity->GetComponent<OgreRenderer::EC_OgrePlaceable>().get();
    if (!node)
        return;

    Ogre::SceneNode *sceneNode = node->GetSceneNode();
    assert(sceneNode);
    if (!sceneNode)
        return;

    // Create billboard if it doesn't exist.
    if (!billboardSet_ && !billboard_)
    {
        // Create billboardset and billboard
        billboardSet_ = scene->createBillboardSet(renderer_.lock()->GetUniqueObjectName(), 1);
        assert(billboardSet_);

        billboard_ = billboardSet_->createBillboard(Ogre::Vector3(0, 0, default_z_pos_));
        assert(billboard_);

        billboardSet_->setDefaultDimensions(2, 1);
        sceneNode->attachObject(billboardSet_);

        // Create material
        materialName_ = std::string("material") + renderer_.lock()->GetUniqueObjectName(); 
        Ogre::MaterialPtr material = OgreRenderer::CloneMaterial("UnlitTexturedSoftAlpha", materialName_);
        billboardSet_->setMaterialName(materialName_);

        // Create texture
        texture_name_ = "ChatBubbleTexture" + renderer_.lock()->GetUniqueObjectName();
        Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().createManual(
            texture_name_, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            Ogre::TEX_TYPE_2D, 1, 1, 0, Ogre::PF_A8R8G8B8, 
            Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

        // Set texture to material
        if (texture.isNull())
            texture_name_ = "";
        else
            OgreRenderer::SetTextureUnitOnMaterial(material, texture_name_);
    }
    else
    {
        // Billboard already exists, remove it from the old and attach it to a new scene node.
        LogInfo("Trying to detach chat bubble billboard from its old node and attach to a new node. This feature is not tested.");
        Ogre::SceneNode *oldNode = billboardSet_->getParentSceneNode();
        oldNode->detachObject(billboardSet_);
        sceneNode->attachObject(billboardSet_);
    }
}

QPixmap EC_ChatBubble::GetChatBubblePixmap()
{
    if (renderer_.expired())
        return 0;

///\todo    Resize the chat bubble and font size according to the render window size and distance
///         avatar's distance from the camera.
//    const int minWidth =
//    const int minHeight =
//    Ogre::Viewport* viewport = renderer_.lock()->GetViewport();
//    const int max_width = viewport->getActualWidth()/4;
//    int max_height = viewport->getActualHeight()/10;

    // Create transparent pixmap
    QPixmap pixmap(bubble_max_rect_.size());
    pixmap.fill(Qt::transparent);

    // Gather chat log and calculate the bounding rect size.
    /*
    QStringListIterator it(messages_);
    QString fullChatLog;
    while(it.hasNext())
    {
        // Add spaces to too long words (usually just spam) so that wordwrap done later works better.
        ///\todo    Make the logic more intelligent and better. Not critical as normal words aren't
        ///         that often longer than ~20 characters.
        QString rawChatEntry = it.next();
        if (rawChatEntry.length() > 21 && !rawChatEntry.contains(' '))
        {
            int split_idx = rawChatEntry.length()/2;
            QString substr1 = rawChatEntry.mid(0, split_idx+1);
            QString substr2 = rawChatEntry.mid(split_idx, rawChatEntry.length()-substr1.length());
            fullChatLog.append(substr1);
            fullChatLog.append(" ");
            fullChatLog.append(substr2);
        }
        else
        {
            fullChatLog.append(rawChatEntry);
        }

        if (it.hasNext())
            fullChatLog.append('\n');

        // Set padding for text.
        // Make the font size temporarily bigger when calculating bounding rect
        // so we get padding without need to modify the rect itself.
        //QFont origFont = painter.font();
        //painter.setFont(QFont(origFont.family(), origFont.pointSize()+12));
    }
    */

    // Create painter
    QPainter painter(&pixmap);
    painter.setFont(font_);

    // Get padding from font metrics
    QFontMetrics metric(painter.font());
    int padding = metric.averageCharWidth();

    // Text rect
    QRect text_boundaries(padding, padding, bubble_max_rect_.width()-(2*padding), bubble_max_rect_.height()-(2*padding));
    QRect text_rect = painter.boundingRect(text_boundaries, Qt::AlignCenter | Qt::TextWordWrap, current_message_ /*fullChatLog*/);
    
    // Background rect
    int bg_left = text_rect.x() - (padding/2);
    if (bg_left < 0)
        bg_left = 0;
    int bg_top = text_rect.y() - (padding/2);
    if (bg_top < 0)
        bg_top = 0;
    int bg_width = text_rect.width() + (padding);
    if (bg_width > bubble_max_rect_.width())
        bg_width = bubble_max_rect_.width();
    int bg_height = text_rect.height() + (padding);
    if (bg_height > bubble_max_rect_.height())
        bg_height = bubble_max_rect_.height();

    QRect bg_rect(bg_left, bg_top, bg_width, bg_height); 

    // Background color
    QLinearGradient grad(bg_rect.topLeft(), bg_rect.bottomLeft());
    grad.setColorAt(0, QColor(39, 92, 206, 255));
    grad.setColorAt(1, bubbleColor_);
    QBrush brush(grad);

    // Draw rounded rect
    painter.setBrush(brush);
    painter.setPen(QColor(39, 92, 206, 255));
    painter.drawRoundedRect(bg_rect, 20.0, 20.0);

    // Draw text
    painter.setPen(textColor_);
    painter.drawText(text_rect, Qt::AlignCenter | Qt::TextWordWrap, current_message_);

    return pixmap;
}
