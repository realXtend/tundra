#ifndef incl_OgreRenderer_EC_HoveringIconGroup_h
#define incl_OgreRenderer_EC_HoveringIconGroup_h

#include "ComponentInterface.h"
#include "Declare_EC.h"
#include "OgreModuleApi.h"

#include <QList>
#include <QString>
#include <OgreVector3.h>
#include <QPixmap>

class HoveringIcon;


//! This class acts as a container for icons that are shown besides the Entitys actual geometry
class OGRE_MODULE_API EC_HoveringIconGroup : public Foundation::ComponentInterface
{
    Q_OBJECT
    DECLARE_EC(EC_HoveringIconGroup);

private:
    explicit EC_HoveringIconGroup(Foundation::ModuleInterface *module);

public:
    virtual ~EC_HoveringIconGroup();

    QList<HoveringIcon*> GetIcons(){return icons_;}

    //! Invoked the entity owning this group is clicked.
    void Clicked();


    bool IsVisible(){return visible_;}

    //! Adds an icon to the group.
    // @param name name of the icon. The icons name is used when the event is dispatched to specify what icon was clicked in the event
    void AddIcon(QString &name, QString &image_path);

    //! get the width of the icon (all icons in group are of same size)
    Real GetIconWidth(){return icon_width_;}
    //! get the height of the icon (all icons in group are of same size)
    Real GetIconHeight(){return icon_height_;}
private:

    //!position from where to start placing the icons (relative to the position of the actual geometry)
    Ogre::Vector3 start_pos_;

    //! position where the next added icon will be placed
    Ogre::Vector3 next_pos_;

    //! height of the icon (all icons in group are of same size)
    Real icon_height_;

    //! width of the icon (all icons in group are of same size)
    Real icon_width_;

    //! Are the icons visible?
    bool visible_;

    //! Icons in this group
    QList<HoveringIcon*> icons_; 
};



#endif
