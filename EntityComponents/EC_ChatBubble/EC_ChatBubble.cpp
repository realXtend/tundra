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
    font_(QFont("Arial", 100)),
    bubbleColor_(QColor(48, 113, 255, 255)),
    textColor_(Qt::white),
    billboardSet_(0),
    billboard_(0)
{
    renderer_ = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);
}

EC_ChatBubble::~EC_ChatBubble()
{
    RemoveAllMessages();
}

void EC_ChatBubble::SetPosition(const Vector3df& position)
{
    if (billboard_)
        billboard_->setPosition(Ogre::Vector3(position.x, position.y, position.z));
}

void EC_ChatBubble::ShowMessage(const QString &msg)
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
        billboardSet_ = scene->createBillboardSet(renderer_.lock()->GetUniqueObjectName(), 1);
        assert(billboardSet_);

        materialName_ = std::string("material") + renderer_.lock()->GetUniqueObjectName(); 
        Ogre::MaterialPtr material = OgreRenderer::CloneMaterial("UnlitTexturedSoftAlpha", materialName_);
        billboardSet_->setMaterialName(materialName_);

        billboard_ = billboardSet_->createBillboard(Ogre::Vector3(0, 0, 1.5f));
        assert(billboard_);
        billboardSet_->setDefaultDimensions(2, 1);

        sceneNode->attachObject(billboardSet_);
    }

    if (msg.isNull() || msg.isEmpty())
        return;

    messages_.push_back(msg);

    // Set timer for removing the message
    int timeToShow = 0;
    const int minTime = 4000;
    if (msg.length() * 400 < minTime)
        timeToShow = minTime;
    else
        timeToShow = msg.length() * 400;

    QTimer::singleShot(timeToShow, this, SLOT(RemoveLastMessage()));

    Refresh();
}

void EC_ChatBubble::RemoveLastMessage()
{
    messages_.pop_front();
    Refresh();
}

void EC_ChatBubble::RemoveAllMessages()
{
    messages_.clear();
    Refresh();
}

void EC_ChatBubble::Refresh()
{
    if (renderer_.expired() ||!billboardSet_ || !billboard_)
        return;

    // If no messages in the log, hide the chat bubble.
    if (messages_.size() == 0)
    {
        billboardSet_->setVisible(false);
        return;
    }
    else
        billboardSet_->setVisible(true);

    // Get pixmap with chat bubble and text rendered to it.
    QPixmap pixmap = GetChatBubblePixmap();
    if (pixmap.isNull())
        return;

    // Create texture
    QImage img = pixmap.toImage();
#include "DisableMemoryLeakCheck.h"
    Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream((void*)img.bits(), img.byteCount()));
#include "EnableMemoryLeakCheck.h"
    std::string tex_name("ChatBubbleTexture" + renderer_.lock()->GetUniqueObjectName());
    Ogre::TextureManager &manager = Ogre::TextureManager::getSingleton();
    Ogre::Texture *tex = checked_static_cast<Ogre::Texture *>(manager.create(
        tex_name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).get());
    assert(tex);

    tex->loadRawData(stream, img.width(), img.height(), Ogre::PF_A8R8G8B8);

    // Set new texture for the material
    assert(!materialName_.empty());
    if (!materialName_.empty())
    {
        Ogre::MaterialManager &mgr = Ogre::MaterialManager::getSingleton();
        Ogre::MaterialPtr material = mgr.getByName(materialName_);
        assert(material.get());
        OgreRenderer::SetTextureUnitOnMaterial(material, tex_name);
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

    const int max_width = 1600;
    int max_height = 800;
    QRect max_rect(0, 0, max_width, max_height);

    // Create transparent pixmap
    QPixmap pixmap(max_rect.size());
    pixmap.fill(Qt::transparent);

    // Gather chat log and calculate the bounding rect size.
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
    }

    // Create painter
    QPainter painter(&pixmap);
    painter.setFont(font_);

    // Set padding for text.
    // Make the font size temporarily bigger when calculating bounding rect
    // so we get padding without need to modify the rect itself.
    QFont origFont = painter.font();
    painter.setFont(QFont(origFont.family(), origFont.pointSize()+12));
    QRect rect = painter.boundingRect(max_rect, Qt::AlignCenter | Qt::TextWordWrap, fullChatLog);
    painter.setFont(origFont);
    rect.setHeight(rect.height() - 10);
    // could also try this:
    // QFontMetrics metric(any_qfont); int width = metric.width(mytext) + padding;

    // Color setup
    QLinearGradient grad(rect.topLeft(), rect.bottomLeft());
    grad.setColorAt(0, QColor(39, 92, 206, 255));
    grad.setColorAt(1, bubbleColor_);
    QBrush brush(grad);

    // Draw rounded rect.
    painter.setBrush(brush);
    painter.drawRoundedRect(rect, 20.0, 20.0);

    // Draw text
    painter.setPen(textColor_);
    painter.drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, fullChatLog);

    return pixmap;
}

