/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_HoveringText.cpp
 *  @brief  EC_HoveringText shows a hovering text attached to an entity.
 *  @note   The entity must EC_OgrePlaceable available in advance.
 */

#include "StableHeaders.h"
//#include "DebugOperatorNew.h"
#include "EC_HoveringText.h"
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

//#include "MemoryLeakCheck.h"

EC_HoveringText::EC_HoveringText(Foundation::ModuleInterface *module) :
    Foundation::ComponentInterface(module->GetFramework()),
    font_(QFont("Arial", 100)),
    backgroundColor_(Qt::transparent),
    textColor_(Qt::black),
    billboardSet_(0),
    billboard_(0),
    text_("")
{
    renderer_ = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);
}

EC_HoveringText::~EC_HoveringText()
{
}

void EC_HoveringText::SetPosition(const Vector3df& position)
{
    if (billboard_)
        billboard_->setPosition(Ogre::Vector3(position.x, position.y, position.z));
}

void EC_HoveringText::SetFont(const QFont &font)
{
    font_ = font;
    Redraw();
}

void EC_HoveringText::SetTextColor(const QColor &color)
{
    textColor_ = color;
    Redraw();
}

void EC_HoveringText::SetBackgroundColor(const QColor &color)
{
    backgroundColor_ = color;
    Redraw();
}

void EC_HoveringText::Show()
{
    if (billboardSet_)
        billboardSet_->setVisible(true);
}

void EC_HoveringText::Hide()
{
    if (billboardSet_)
        billboardSet_->setVisible(false);
}

bool EC_HoveringText::IsVisible() const
{
    if (billboardSet_)
        return billboardSet_->isVisible();
    else
        return false;
}

void EC_HoveringText::ShowMessage(const QString &text)
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

        std::string newName = std::string("material") + renderer_.lock()->GetUniqueObjectName(); 
        Ogre::MaterialPtr material = OgreRenderer::CloneMaterial("UnlitTexturedSoftAlpha", newName);
        billboardSet_->setMaterialName(newName);

        billboard_ = billboardSet_->createBillboard(Ogre::Vector3(0, 0, 1.f));
        assert(billboard_);
        billboard_->setDimensions(2, 1);

        sceneNode->attachObject(billboardSet_);
    }

    if (text.isNull() || text.isEmpty())
        return;

    text_ = text;

    Redraw();
}

void EC_HoveringText::Redraw()
{
    if (renderer_.expired() ||!billboardSet_ || !billboard_)
        return;

    // Get pixmap with text rendered to it.
    QPixmap pixmap = GetTextPixmap();
    if (pixmap.isNull())
        return;

    // Create texture
    QImage img = pixmap.toImage();
    Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream((void*)img.bits(), img.byteCount()));
    std::string tex_name("HoveringTextTexture" + renderer_.lock()->GetUniqueObjectName());
    Ogre::TextureManager &manager = Ogre::TextureManager::getSingleton();
    Ogre::Texture *tex = dynamic_cast<Ogre::Texture *>(manager.create(tex_name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).get());
    assert(tex);

    tex->loadRawData(stream, img.width(), img.height(), Ogre::PF_A8R8G8B8);

    // Set new material with the new texture name in it.
    std::string newMatName = std::string("material") + renderer_.lock()->GetUniqueObjectName(); 
    Ogre::MaterialPtr material = OgreRenderer::CloneMaterial("UnlitTexturedSoftAlpha", newMatName);
    OgreRenderer::SetTextureUnitOnMaterial(material, tex_name);
    billboardSet_->setMaterialName(newMatName);
}

QPixmap EC_HoveringText::GetTextPixmap()
{
    if (renderer_.expired())
        return 0;

///\todo    Resize the font size according to the render window size and distance
///         avatar's distance from the camera.
//    const int minWidth =
//    const int minHeight =
//    Ogre::Viewport* viewport = renderer_.lock()->GetViewport();
//    const int max_width = viewport->getActualWidth()/4;
//    int max_height = viewport->getActualHeight()/10;

    const int max_width = 1500;
    int max_height = 800;
    QRect max_rect(0, 0, max_width, max_height);

    const QString &filename("./media/textures/Transparent.png");
    assert(QFile::exists(filename));
    if (!QFile::exists(filename))
        return 0;

    // Create pixmap
    QPixmap pixmap;
    pixmap.load(filename);
    pixmap = pixmap.scaled(max_rect.size());

    QPainter painter(&pixmap);
    painter.setFont(font_);

    // Do some splitting for rex account names
    // Remove the server tag for now
    ///\todo Should not be done here. Do this in EC_OpenSimPresence.
    if (text_.indexOf("@") != -1)
    {
        QStringList words = text_.split(" ");
        foreach (QString word, words)
            if (word.indexOf("@") != -1)
                words.removeOne(word);
        text_ = words.join(" ");
    }

    // Set padding for text.
    // Make the font size temporarily bigger when calculating bounding rect
    // so we get padding without need to modify the rect itself.
    QFont origFont = painter.font();
    painter.setFont(QFont(origFont.family(), origFont.pointSize()+12));
    QRect rect = painter.boundingRect(max_rect, Qt::AlignCenter | Qt::TextWordWrap, text_);
    painter.setFont(origFont);
    // could also try this:
    // QFontMetrics metric(any_qfont); int width = metric.width(mytext) + padding;

    rect.setHeight(rect.height() - 10);
/*
     QRadialGradient gradient(rect.center(), 300);
     gradient.setColorAt(0, QColor::fromRgbF(0, 0, 0, 0.9));
     gradient.setColorAt(1, QColor::fromRgbF(0.75f, 0.75f, 0.75f, 0.9));
     QBrush brush(gradient);
*/
    // Draw rounded rect.
    QBrush brush(backgroundColor_, Qt::SolidPattern);
    painter.setBrush(brush);
    if (backgroundColor_ != Qt::transparent)
        painter.drawRoundedRect(rect, 20.0, 20.0);

    // Draw text
    painter.setPen(textColor_);
    painter.drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, text_);

    return pixmap;
}

