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



class OGRE_MODULE_API EC_HoveringIconGroup : public Foundation::ComponentInterface
{
    Q_OBJECT
    DECLARE_EC(EC_HoveringIconGroup);

private:
    explicit EC_HoveringIconGroup(Foundation::ModuleInterface *module);

public:
    virtual ~EC_HoveringIconGroup(){}

    QList<HoveringIcon*> GetIcons(){return icons_;}

    void Clicked();
    bool IsVisible(){return visible_;}

    void AddIcon(QString &name, QString &image_path);

    Real GetIconWidth(){return icon_width_;}
    Real GetIconHeight(){return icon_height_;}
private:
    Ogre::Vector3 start_pos_;
    Ogre::Vector3 next_pos_;

    Real icon_height_;
    Real icon_width_;

    bool visible_;

    QList<HoveringIcon*> icons_; 
};



#endif
