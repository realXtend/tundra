// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_BillboardWidget_h
#define incl_EC_BillboardWidget_h

#include "IComponent.h"
#include "IAttribute.h"
#include "Declare_EC.h"
#include "Vector3D.h"
#include "OgreVector3.h"

class QWidget;
namespace Ogre
{
    class SceneNode;
    class BillboardSet;
    class Billboard;
}

namespace OgreRenderer
{
    class Renderer;
}

/// Shows given QWidget as a billboard with given size and relative position in a 3d scene.
/**
    @note Instances of this EC is not ment to be stored to server since there is no properties.
          This EC is designed to be used by client side logic such scripts to show QWidget content
          as billboards.

    @todo Transparency level support
    @todo Nonblocking texture refresh
*/
class EC_BillboardWidget : public IComponent
{
    DECLARE_EC(EC_BillboardWidget);
    Q_OBJECT
public:

    ~EC_BillboardWidget();
    virtual bool IsSerializable() const { return false; }

public slots:
    /// Set widget to be showed
    /// @note The widget object is not destroyed when billboard is deleted.
    void SetWidget(QWidget* widget);

    /// @return current widget. Return 0 if current widget is not set
    QWidget* GetWidget();

    /// Updated the billboard content using current widget
    void Refresh();

    /// position of the center of the billboard
    void SetPosition(Vector3df& pos);

    /// position of the center of the billboard
    void SetPosition(float x, float y, float z);

    /// position of the center of the billboard
    Vector3df GetPosition() { return Vector3df(position_.x, position_.y, position_.z); }

    /// Set width and height in world units
    void SetSize(float width, float height);

    /// Set width of the billboard
    /// @param width Height in world units
    void SetWidth(float width);

    /// Set height of the billboard
    /// @param height Height in world units
    void SetHeight(float height);

    /// @return widh of the billboard in the scene
    float GetWidth() { return width_; }

    /// @return height if the billboard in the scene
    float GetHeight() { return height_; }

private slots:
    void InitializeOgreBillboard();
    void UninitializeOgreBillboard();

private:
    explicit EC_BillboardWidget(IModule *module);

    QWidget* widget_;

    /// Renderer pointer.
    boost::weak_ptr<OgreRenderer::Renderer> renderer_;

    /// Ogre billboard set.
    Ogre::BillboardSet* billboard_set_;

    /// Ogre billboard.
    Ogre::Billboard* billboard_;

    std::string material_name_;
    std::string texture_name_;

    Ogre::Vector3 position_;
    float width_;
    float height_;
};

#endif // incl_EC_BillboardWidget_h
