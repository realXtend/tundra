#include "StableHeaders.h"
#include "EC_HoveringIconGroup.h"
#include "HoveringIcon.h"
#include "OgreRenderingModule.h"


EC_HoveringIconGroup::EC_HoveringIconGroup(Foundation::ModuleInterface *module)
:Foundation::ComponentInterface(module->GetFramework()),
visible_(false),
icon_height_(0.25f),
icon_width_(0.25f)
    {
        start_pos_ = Ogre::Vector3(0.0f, 0.7f, 0.9f);
        next_pos_ = start_pos_;

        
    }

void EC_HoveringIconGroup::AddIcon(QString &name, QString &image_path)
{
    HoveringIcon* icon = new HoveringIcon(framework_, image_path, next_pos_,this);
    next_pos_.z -= icon_height_ + 0.05f;
    icon->SetName(name);
    icons_.append(icon);
}
void EC_HoveringIconGroup::Clicked()
{
    visible_ = !visible_;
    for(int i=0;i<icons_.size();i++)
    {
        icons_.at(i)->SetVisible(visible_);
    }

}
