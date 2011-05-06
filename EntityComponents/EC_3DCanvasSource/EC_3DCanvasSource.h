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
class QProgressBar;
class QPushButton;
class QWebView;

class EC_3DCanvas;

/// A 3D canvas controller that initializes an EC_3DCanvas into the same entity, with source parameters (for example html page url)
/**
<table class="header">
<tr>
<td>
<h2>3DCanvasSource</h2>

\note THIS COMPONENT IS DEPRECATED. Use the new EC_WebView instead when possible.

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

    //! Submesh number into which to apply the 3DCanvas
    Q_PROPERTY(int submesh READ getsubmesh WRITE setsubmesh);
    DEFINE_QPROPERTY_ATTRIBUTE(int, submesh);

    //! Show as 2D when clicked
    Q_PROPERTY(bool show2d READ getshow2d WRITE setshow2d);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, show2d);

    //! Sync url changes from the ui
    Q_PROPERTY(bool sync2dbrowsing READ getsync2dbrowsing WRITE setsync2dbrowsing);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, sync2dbrowsing);

    //! Refresh rate
    Q_PROPERTY(int refreshRate READ getrefreshRate WRITE setrefreshRate);
    DEFINE_QPROPERTY_ATTRIBUTE(int, refreshRate);

    //! Width of the rendered widget
    Q_PROPERTY(int pageWidth READ getpageWidth WRITE setpageWidth);
    DEFINE_QPROPERTY_ATTRIBUTE(int, pageWidth);

    //! Height of the rendered widget
    Q_PROPERTY(int pageHeight READ getpageHeight WRITE setpageHeight);
    DEFINE_QPROPERTY_ATTRIBUTE(int, pageHeight);

public slots:
    void OnClick();
    //! Source text editor modified
    void SourceEdited();

    //! Link clicked in the 2D web view UI
    void WebViewLinkClicked(const QUrl& url);
    //! Loading starts, show progress bar and update ui
    void WebViewLoadStarted();
    //! Loading progress, update progress bar
    void WebViewLoadProgress(int progress);
    //! Loading completed, hide progress bad and update ui + repaint canvas
    void WebViewLoadCompleted();

    //! Buttons handler for stop/refresh
    void RefreshStopPressed();
    //! Buttons handler for back
    void BackPressed();
    //! Buttons handler for forward
    void ForwardPressed();
    //! Buttons handler for home (initial component url)
    void HomePressed();
    
private slots:
    void UpdateWidgetAndCanvas(IAttribute *attribute, AttributeChange::Type type);
    void UpdateWidget(QString url = QString());
    void UpdateCanvas();
    void RepaintCanvas();
    void ChangeLanguage();
    void FetchWebViewUrl();

    /// Registers the action this EC provides to the parent entity, when it's set.
    void RegisterActions();

    EC_3DCanvas *Get3DCanvas();
    QWebView *GetWebView();

private:
    //! Constructor.
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

    //! Progress bar ptr picked from the ui file
    QProgressBar *progress_bar_;

    //! refresh/stop button ptr picked from the ui file
    QPushButton *button_refreshstop_;
    
    //! Initial component url is marked as the home page
    QString home_url_;
    
    //! Last set source
    QString last_source_;

    bool canvas_started_;
};

#endif
