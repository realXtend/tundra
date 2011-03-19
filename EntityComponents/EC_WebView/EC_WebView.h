// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_WebView_h
#define incl_EC_WebView_h

#include "IComponent.h"
#include "IAttribute.h"
#include "Declare_EC.h"

#include <QString>
#include <QSize>
#include <QPointer>
#include <QTimer>

class QWebView;

class EC_Mesh;
class EC_3DCanvas;

class EC_WebView : public IComponent
{
    DECLARE_EC(EC_WebView);
    Q_OBJECT

public:
    //! Webview URL
    Q_PROPERTY(QString webviewUrl READ getwebviewUrl WRITE setwebviewUrl);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, webviewUrl);

    //! Webview widget size
    Q_PROPERTY(QSize webviewSize READ getwebviewSize WRITE setwebviewSize);
    DEFINE_QPROPERTY_ATTRIBUTE(QSize, webviewSize);

    //! Rendering target submesh index
    Q_PROPERTY(int renderSubmeshIndex READ getrenderSubmeshIndex WRITE setrenderSubmeshIndex);
    DEFINE_QPROPERTY_ATTRIBUTE(int, renderSubmeshIndex);

    //! Rendering refresh rate
    Q_PROPERTY(int renderRefreshRate READ getrenderRefreshRate WRITE setrenderRefreshRate);
    DEFINE_QPROPERTY_ATTRIBUTE(int, renderRefreshRate);

    //! Destructor.
    ~EC_WebView();

    //! Returns if Component is serializable, always returns true.
    virtual bool IsSerializable() const;

public slots:
    void Render();

private slots:
    //! For internals to do delayed rendering due to e.g. widget resize or submesh index change.
    /// \note Depending if the 'renderRefreshRate' is a valid number (0-25) we use the timer, otherwise a QTimer::singleShot().
    void RenderDelayed();

    //! Free QWebView memory and reset internal pointer.
    void ResetWidget();

    //! Prepares everything related to the parent widget and other needed components.
    void PrepareComponent();

    //! Prepares the QWebView and connects its signals to our slots.
    void PrepareWebview();

    //! Handler for QWebView::loadStarted().
    void LoadStarted();

    //! Handler for QWebView::loadFinished(bool).
    void LoadFinished(bool success);

    //! If user select invalid submesh, this function is invoked with a delay and the value is reseted to 0.
    void ResetSubmeshIndex();

    //! Stops the current update timer
    void RenderTimerStop();

    //! Starts the update timer, or does a delayed rendering. Depending on the 'renderRefreshRate' value.
    void RenderTimerStartOrSingleShot();

    //! Monitors this entitys added components.
    void ComponentAdded(IComponent *component, AttributeChange::Type change);

    //! Monitors this entitys removed components.
    void ComponentRemoved(IComponent *component, AttributeChange::Type change);

    //! Monitors this components Attribute changes.
    void AttributeChanged(IAttribute *attribute, AttributeChange::Type changeType);

    //! Get parent entitys EC_Mesh. Return 0 if not present.
    EC_Mesh *GetMeshComponent();

    //! Get parent entitys EC_3DCanvas. Return 0 if not present.
    EC_3DCanvas *GetSceneCanvasComponent();

private:
    //! Constuctor.
    explicit EC_WebView(IModule *module);
    
    //! Boolean for tracking if this component has been prepared properly.
    /*! Guarantees: 
        - EC_Mesh is present and loaded to Ogre, ready for rendering.
        - EC_3DCanvas is present.
        - Parent Entity is valid and set.
        Does not guarantee:
        - EC_Mesh having submesh index of our 'renderSubmeshIndex' Attribute
        - EC_Mesh being visible. 
    */
    bool componentPrepared_;

    //! Boolean for tracking if the QWebView is currently in loading state.
    bool webviewLoading_;

    //! Boolean if our QWebView has loaded content for us to render.
    bool webviewHasContent_;

    //! Internal QWebView for rendering the web page.
    QPointer<QWebView> webview_;

    //! Internal timer for updating inworld EC_3DCanvas.
    QTimer *renderTimer_;
};

#endif
