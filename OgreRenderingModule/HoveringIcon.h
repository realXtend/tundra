/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   HoveringIcon.h
 *  @brief  HoveringIcon shows a pic attached to an entity.
 *  @note   The entity must have EC_OgrePlaceable component available in advance.
*/

#ifndef incl_OgreRenderer_HoveringIcon_h
#define incl_OgreRenderer_HoveringIcon_h


#include "Vector3D.h"
#include "Foundation.h"
#include "OgreModuleApi.h"
#include <OgreMaterial.h>
#include <OgreBillboard.h>
#include <OgreVector3.h>
#include <QPixmap>
#include <QColor>
#include <QTimeLine>
#include <OgreBillboardSet.h>

class EC_HoveringIconGroup;
namespace Foundation
{
    class Framework;
}

namespace Scene
{
    class Entity;
}

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

class OGRE_MODULE_API HoveringIcon: public QObject
{
    Q_OBJECT

public:
    /// Constuctor.
    /// @param module Owner module.
    explicit HoveringIcon(Foundation::Framework *framework, QString& pic, Ogre::Vector3& vec, EC_HoveringIconGroup* grp);



    /// Destructor.
    ~HoveringIcon();

    /// Sets position for the icon
    /// @param position Position.
    /// @note The position is relative to the entity to which the hovering icon is attached.
    void SetPosition(const Vector3df &position);



public slots:
    /// Shows the hovering icon
    void Show();
    void AnimatedShow();
    void SetVisible(bool val);

    /// Hides the hovering icon
    void Hide();
    void AnimatedHide();

    /// Returns if the hovering icon is visible or not.
    /// @true If the hovering icon is visible, false if it's hidden or not initialized properly.
    bool IsVisible() const;



    //!Initializes the Icon internals. Has to be called before anything is actually shown. Also called in ShowPic -function
    void InitializeBillboards();

    const Ogre::Billboard& GetBillboard(){return *billboard_;} 

    const Ogre::BillboardSet& GetBillBoardSet(){return *billboardSet_;}

    void SetName(const QString &name){name_ = name;}
    const QString& GetName(){return name_;}

    Scene::Entity* GetParentEntity();




private slots:
    void UpdateAnimationStep(int step);
    void AnimationFinished();

    /// Redraws the hovering icon
    void Redraw();

private:

    
    //Location 
    Ogre::Vector3 pos_;

    /// Renderer pointer.
    boost::weak_ptr<OgreRenderer::Renderer> renderer_;

    /// Ogre billboard set.
    Ogre::BillboardSet *billboardSet_;

    /// Ogre billboard.
    Ogre::Billboard *billboard_;

    /// Ogre material for billboard set
    Ogre::MaterialPtr material_;




    QString name_;

    QPixmap pixmap_;


    // Visibility animation timeline
    QTimeLine *visibility_animation_timeline_;

    // Timed visibility timer
    QTimer *visibility_timer_;

    Foundation::Framework* framework_;

    EC_HoveringIconGroup* group_;
};

#endif
