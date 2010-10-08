// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_3DCanvasSource_EC_3DCanvasSource_h
#define incl_EC_3DCanvasSource_EC_3DCanvasSource_h

#include "IComponent.h"
#include "IAttribute.h"
#include "Declare_EC.h"

class UiProxyWidget;

class QLineEdit;
class QWidget;
class QUrl;

/// A 3D canvas controller that initializes an EC_3DCanvas into the same entity, with source parameters (for example html page url)
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
<li>"MousePressed" : Opens the web page in 2D web browser widget if show2d attribute is true.
</ul>
</td>
</tr>

Does not emit any actions.

<b>Depends on components 3DCanvas, OgreMesh and OgreCustomObject</b>.
</table>
*/
class EC_3DCanvasSource : public IComponent
{
    DECLARE_EC(EC_3DCanvasSource);

    Q_OBJECT

public:
    //! Destructor.
    ~EC_3DCanvasSource();

    virtual bool IsSerializable() const { return true; }

    //! Source
    Q_PROPERTY(QString source READ getsource WRITE setsource);
	DEFINE_QPROPERTY_ATTRIBUTE(QString, source);

    //! Position within source (for example scrollbar value, page number)
    Q_PROPERTY(int position READ getposition WRITE setposition);
	DEFINE_QPROPERTY_ATTRIBUTE(int, position);

    //! Submesh number into which to apply the 3DCanvas
    Q_PROPERTY(int submesh READ getsubmesh WRITE setsubmesh);
	DEFINE_QPROPERTY_ATTRIBUTE(int, submesh);

    //! Show as 2D when clicked
    Q_PROPERTY(bool show2d READ getshow2d WRITE setshow2d);
	DEFINE_QPROPERTY_ATTRIBUTE(bool, show2d);

    bool manipulate_ec_3dcanvas;

public slots:
    void OnClick();
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

    /// Registers the action this EC provides to the parent entity, when it's set.
    void RegisterActions();

private:
    //! Constuctor.
    /*! \param module Module.
     */
    explicit EC_3DCanvasSource(IModule *module);

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
