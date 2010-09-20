// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_3DCanvasSource_EC_3DCanvasSource_h
#define incl_EC_3DCanvasSource_EC_3DCanvasSource_h

#include "ComponentInterface.h"
#include "AttributeInterface.h"
#include "Declare_EC.h"

class QDomDocument;
class QDomElement;

namespace Foundation
{
    class Framework;
}

class UiProxyWidget;

class QLineEdit;
class QWidget;
class QUrl;

/**

<table class="header">
<tr>
<td>
<h2>3DCanvasSource</h2>
A 3D canvas controller that initializes an EC_3DCanvas into the same entity, with source parameters (for example html page url)
Also makes it possible to display the canvas contents as a 2D widget, and edit the source

Registered by RexLogic::RexLogicModule.

<b>Attributes</b>:
<ul>

<li>QString: source
<div>Source</div> 
<li>int: position
<div>Position within source (for example scrollbar value, page number)</div> 
<li>int: submesh
<div>Submesh number into which to apply the 3DCanvas</div> 
<li>bool: show2d
<div>Show as 2D when clicked</div> 
</ul>




<b>Exposes the following scriptable functions:</b>
<ul>
<li>"Clicked": Source text editor modified.
<li>"SourceEdited": Link clicked in the 2D webview UI.
<li>"StartPressed": Todo
<li>"PrevPressed": Todo
<li>"NextPressed": Todo
<li>"EndPressed": Todo
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>



Does not emit any actions.

<b>Depends on components 3DCanvas, OgreMesh and OgreCustomObject</b>.
</table>

*/

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
    Attribute<QString> source_;

    //! Position within source (for example scrollbar value, page number)
    Attribute<int> position_;

    //! Submesh number into which to apply the 3DCanvas
    Attribute<int> submesh_;
    
    //! Show as 2D when clicked
    Attribute<bool> show2d_;

    bool manipulate_ec_3dcanvas;

public slots:
    void Clicked();
    //! Source text editor modified
    void SourceEdited();
    //! Link clicked in the 2D webview UI
    void WebViewLinkClicked(const QUrl& url);
    
    void StartPressed();
    void PrevPressed();
    void NextPressed();
    void EndPressed();
    
private slots:
    void UpdateWidgetAndCanvas();
    void UpdateWidget();
    void UpdateCanvas();
    void RepaintCanvas();
    void ChangeLanguage();
    void FetchWebViewUrl();
    
private:
    //! Constuctor.
    /*! \param module Module.
     */
    explicit EC_3DCanvasSource(Foundation::ModuleInterface *module);

    //! Create the 2D UI widget, but do not show it yet
    void CreateWidget();

    //! 2D UI widget
    QWidget* widget_;
    
    //! Content widget that is set to the 3DCanvas
    QWidget* content_widget_;
    
    //! Content placeholder widget
    QWidget* placeholder_widget_;

    //! Content proxy for ui scene
    UiProxyWidget *proxy_;

    //! Source line editor
    QLineEdit* source_edit_;
    
    //! Last set source
    QString last_source_;
};

#endif
