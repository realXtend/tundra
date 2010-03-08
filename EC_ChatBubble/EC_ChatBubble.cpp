/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_ChatBubble.cpp
 *  @brief  EC_ChatBubble 
 */

#include "StableHeaders.h"
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
#include <QTimer>

EC_ChatBubble::EC_ChatBubble(Foundation::ModuleInterface *module) :
    Foundation::ComponentInterface(module->GetFramework())
{
    /*
    boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetServiceManager()->GetService
        <OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
    if (!renderer)
        return;

    Ogre::SceneManager *scene = renderer->GetSceneManager();
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

    sceneNode_ = node->GetSceneNode();
    assert(sceneNode_);

    billboardSet_ = scene->createBillboardSet(renderer->GetUniqueObjectName(), 1);
    assert(billboardSet_);

    std::string newName = std::string("material") + renderer->GetUniqueObjectName(); 
    Ogre::MaterialPtr material = OgreRenderer::CloneMaterial("UnlitTexturedSoftAlpha", newName);
    //OgreRenderer::SetTextureUnitOnMaterial(material, imageName);
    billboardSet_->setMaterialName(newName);

    Ogre::Vector3 offset(0,0,0);
    billboard_ = billboardSet_->createBillboard(Ogre::Vector3(offset.x, offset.y, offset.z));
    assert(billboard_);
    billboard_->setDimensions(1, 1);

    sceneNode_->attachObject(billboardSet);
    */
}

EC_ChatBubble::~EC_ChatBubble()
{
}

void EC_ChatBubble::Show(const Vector3df &offset, float timeToShow, const char *imageName)
{
    /*
    if (!sceneNode_)
        return;

    Ogre::TextureManager &manager = Ogre::TextureManager::getSingleton();
    Ogre::Texture *tex = dynamic_cast<Ogre::Texture *>(manager.getByName(imageName).get());
    if (!tex)
    {
        ///\bug OGRE doesn't seem to add all texture to the resource group although the texture
        ///     exists in folder spesified in the resource.cfg
        std::stringstream ss;
        ss << "Ogre Texture \"" << imageName << "\" not found!";
        std::cout << ss.str() << std::endl;

        tex = dynamic_cast<Ogre::Texture *>(manager.create(imageName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).get());
        if (tex)
        {
            std::cout << "But should be now..." << std::endl;
        }
    }

    ///\todo GetUniqueObjectName generates object names, not material or billboardset names, but anything unique goes.
    /// Perhaps would be nicer to just have a GetUniqueName(string prefix)?

    Ogre::BillboardSet *billboardSet = scene->createBillboardSet(renderer->GetUniqueObjectName(), 1);
    assert(billboardSet);

    std::string newName = std::string("material") + renderer->GetUniqueObjectName(); 
    Ogre::MaterialPtr material = OgreRenderer::CloneMaterial("UnlitTexturedSoftAlpha", newName);
    OgreRenderer::SetTextureUnitOnMaterial(material, imageName);
    billboardSet->setMaterialName(newName);

    Ogre::Billboard *billboard = billboardSet->createBillboard(Ogre::Vector3(offset.x, offset.y, offset.z));
    assert(billboard);
    billboard->setDimensions(1, 1);

    sceneNode_->attachObject(billboardSet);
    */
}

void EC_ChatBubble::ShowMessage(const QString &msg)
{
    /*
    if (!sceneNode_)
        return;

    if (msg.isNull() || msg.isEmpty())
        return;

    messages_.push_back(msg);

    // Set timer for removing the message
    int time = msg.length() * 250;
    QTimer::singleShot(time, this, SLOT(RemoveLastMessage()));

    Refresh();

    // Get pixmap with chat bubble and text rendered to it.
    QPixmap pixmap = GetPixmap(msg);
    if (pixmap.isNull())
        return;

    // Create texture
    QImage img = pixmap.toImage();
    if (img.isNull())
        return;

    Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream((void*)img.bits(), img.byteCount()));
    std::string tex_name("ChatBubbleTexture" + renderer->GetUniqueObjectName());
    Ogre::TextureManager &manager = Ogre::TextureManager::getSingleton();
    Ogre::Texture *tex2 = dynamic_cast<Ogre::Texture *>(manager.create(tex_name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).get());
    if (!tex2)
        return;

    tex2->loadRawData(stream, img.width(), img.height(), Ogre::PF_R8G8B8A8);
    //tex2->load();
    //tex2->isLoaded()

    Ogre::BillboardSet *billboardSet = scene->createBillboardSet(renderer->GetUniqueObjectName(), 1);
    assert(billboardSet);

    std::string newMatName = std::string("material") + renderer->GetUniqueObjectName();
    Ogre::MaterialPtr material = OgreRenderer::CloneMaterial("UnlitTexturedSoftAlpha", newMatName);
    OgreRenderer::SetTextureUnitOnMaterial(material, tex_name);
    billboardSet->setMaterialName(newMatName);

    Ogre::Vector3 offset(0,0,1.25f);
    Ogre::Billboard *billboard = billboardSet->createBillboard(offset);
    assert(billboard);
    billboard->setDimensions(1, 1);
    //billboardSet->setVisible();
    sceneNode_->attachObject(billboardSet);
    */
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
/*
    if (messages_.size() == 0)
        billboardSet_->setVisible(false);
    else
        billboardSet_->setVisible(true);
*/

    /*
    // Get pixmap with chat bubble and text rendered to it.
    QPixmap pixmap = GetPixmap("atssts");
    if (pixmap.isNull())
        return;

    // Create texture
    QImage img = pixmap.toImage();
    if (img.isNull())
        return;

    Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream((void*)img.bits(), img.byteCount()));
    std::string tex_name("ChatBubbleTexture" + renderer->GetUniqueObjectName());
    Ogre::TextureManager &manager = Ogre::TextureManager::getSingleton();
    Ogre::Texture *tex2 = dynamic_cast<Ogre::Texture *>(manager.create(tex_name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).get());
    assert(tex2);

    tex2->loadRawData(stream, img.width(), img.height(), Ogre::PF_R8G8B8A8);

    Ogre::BillboardSet *billboardSet = scene->createBillboardSet(renderer->GetUniqueObjectName(), 1);
    assert(billboardSet);

    std::string newMatName = std::string("material") + renderer->GetUniqueObjectName(); 
    Ogre::MaterialPtr material = OgreRenderer::CloneMaterial("UnlitTexturedSoftAlpha", newMatName);
    OgreRenderer::SetTextureUnitOnMaterial(material, tex_name);
    billboardSet->setMaterialName(newMatName);

    Ogre::Vector3 offset(0,0,1.25f);
    Ogre::Billboard *billboard = billboardSet->createBillboard(offset);
    assert(billboard);
    billboard->setDimensions(1, 1);

    sceneNode_->attachObject(billboardSet);
    */
}

QPixmap EC_ChatBubble::GetPixmap(const QString &image_name, const QString &text)
{
    int wanted_width = 400;
    int wanted_height = 200;
    QRect wanted_rect(10, 10, wanted_width, wanted_height);

    if (!QFile::exists(image_name))
        return 0;

    QPixmap pixmap;
    pixmap.load(image_name);
    pixmap = pixmap.scaled(wanted_rect.size());

    QPainter painter(&pixmap);

    // Draw rounded rect
/*
    QRectF rectangle(10.0, 20.0, 80.0, 60.0);
    painter.setBrush(Qt::SolidPattern);
    painter.drawRoundedRect(rectangle, 20.0, 15.0);
*/

    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 24));
    // draw text
    painter.drawText(wanted_rect, Qt::AlignCenter | Qt::TextWordWrap, text);

    return pixmap;
}

QPixmap EC_ChatBubble::GetPixmap(const QString &text)
{
    int max_width = 600;
    int max_height = 600;
    QRect max_rect(0, 0, max_width, max_height);

    if (!QFile::exists("./media/textures/ChatBubble.png"))
        return 0;

    // Calculate the bounding rect size.
    QStringListIterator it(messages_);
    QString fullChatLog;
    while(it.hasNext())
    {
        fullChatLog.append(it.next());
        fullChatLog.append('\n');
        /*
        QString msg = it.next();
        if (msg.length() > longestMsg.length())
            longestMsg = msg;
        */
    }

    QPixmap pixmap;
    pixmap.load("./media/textures/ChatBubble.png");
    pixmap = pixmap.scaled(max_rect.size());

    QPainter painter(&pixmap);
    painter.setPen(Qt::white);
    //painter.setFont(font_);
    painter.setFont(QFont("Arial", 24));

    QRectF rect = painter.boundingRect(max_rect, Qt::AlignLeft | Qt::TextWordWrap, fullChatLog);
//    rect.width()

    // Draw rounded rect
//    QRectF rectangle(/*10*/0, 0/*20.0*/, 80.0, 60.0);

    QBrush brush(Qt::blue, Qt::SolidPattern);
    painter.setBrush(brush);
    painter.drawRoundedRect(max_rect/*rectangle*/, 20.0, 15.0);

/*
    QString chatLog;
    it.toFront();
    while(it.hasNext())
    {
        chatLog.append(it.next());
        chatLog.append('\n');
    }
*/

    // draw text
    painter.drawText(rect/*wanted_rect*/, Qt::AlignLeft | Qt::TextWordWrap, fullChatLog);

    return pixmap;
}

