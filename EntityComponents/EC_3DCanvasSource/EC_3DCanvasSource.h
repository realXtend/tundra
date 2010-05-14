// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_3DCanvasSource_EC_3DCanvasSource_h
#define incl_EC_3DCanvasSource_EC_3DCanvasSource_h

#include "ComponentInterface.h"
#include "AttributeInterface.h"
#include "Declare_EC.h"

class QDomDocument;
class QDomElement;

#include <QTimer>

namespace Ogre
{
    class Light;
}

//! A 3D canvas controller that initializes an EC_3DCanvas into the same entity, with source parameters (for example html page url)
/*! Also makes it possible to display the canvas contents as a 2D widget, and edit the source
 */
class EC_3DCanvasSource : public Foundation::ComponentInterface
{
    DECLARE_EC(EC_3DCanvasSource);

    Q_OBJECT

public:
    //! Destructor.
    ~EC_3DCanvasSource();

    virtual bool IsSerializable() const { return true; }

    //! Source
    Foundation::Attribute<std::string> source_;

    //! Position within source (for example scrollbar value, page number)
    Foundation::Attribute<int> position_;

    //! Submesh number into which to apply the 3DCanvas
    Foundation::Attribute<int> submesh_;
    
    //! Show as 2D when clicked
    Foundation::Attribute<bool> show2d_;

public slots:
    void Clicked();
    
private slots:
    void UpdateWidgetAndCanvas();
    void UpdateWidget();
    void UpdateCanvas();
    void RepaintCanvas();
    
private:
    //! Constuctor.
    /*! \param module Module.
     */
    explicit EC_3DCanvasSource(Foundation::ModuleInterface *module);

    QWidget* widget_;

    //! Last set source
    std::string last_source_;
};

#endif
