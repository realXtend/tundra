// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_WebView_h
#define incl_EC_WebView_h

#include "IComponent.h"
#include "IAttribute.h"
#include "Declare_EC.h"

#include "SceneFwd.h"

#include <QString>
#include <QSize>
#include <QPointer>
#include <QPoint>
#include <QTimer>
#include <QMenu>

class QWebView;

class EC_Mesh;
class EC_3DCanvas;

class RaycastResult;
class UserConnection;

namespace TundraLogic { class Server; }

//! A web browser on who's content can be rendered into a 3D scene object.
/**
<table class="header"><tr><td>
<h2>WebView</h2>

A web browser on who's content can be rendered into a 3D scene object. The component also support sharing your browsing with across all clients in the world.
You can set the 'interactive' attribute boolean to true. After this clients will get a context menu when clicking mouse on the 3D object the web browser is rendered to.
This menu will provide you to get a 2D UI of the browser and let you take control of shared browsing.

<b>Shared browsing functionality:</b> Any client can request to control the shared browsing, the request will be accepted if no other client has the control at that time.
After a client has control, their 2D browsers page changes and any scroll movement will be synchronized to all clients. This will update the 3D scene object rendering and 2D browser with your current browser state.
In addition when a client has control of the browsing all other clients 2D browser UI scroll bars are hidden and mouse scroll events are denied. The components attributes are hidden from the entity component editor for
everyone but the controller himself. When you want to release your control, click the 3D object again to get the menu you can from there release the control of shared browsing. Releasing control will re-enable everyones
2D browser UIs scroll bars and mouse wheel scroll and show all the attributes in their entity component editors.

<b>Important notes:</b> Recommended that you don't take control of shared browsing on a non headless server instance. This may in certain situation (crash or rundown of server while server has control)
leave your components locked down so that no one can take control back. For clients leaving or crashing during control there are safe guards to never leave the control in a bad state.

Registered by RexLogic::RexLogicModule.

<b>Attributes</b>:
<ul>
<li>QString: webviewUrl
<div>Sets the url that the web browser shows.</div>
<li>QSize: webviewSize
<div>Sets the web browsers size. This attribute is the only way to resize the browser for the syncing feature to always show same content on all clients.</div>
<li>int: renderSubmeshIndex
<div>Sets the submesh index of the entitys EC_Mesh where the browser content will be rendered in the 3D scene object.</div>
<li>int: renderRefreshRate
<div>Sets how many times in a second the browser should be rendered (updated) in the 3D scene object. 0 is no automatic updates, then rendering will be done only when browser content is scrolled.</div>
<li>bool: interactive
<div>Sets if this web browser is interactive. This means when you click it you will get a context menu to show 2D browser and to start/stop shared browsing.</div>
<li>int: controllerId
<div>Defines the client id of the current controller of shared browsing. Note: This attribute is hidden from the UI layer. I suggest you do not set new values to this from code either! Purely for internal c++ use of this component.</div>
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>"Render": Renders the current browser content in the 3D scene object. Use with caution, the component handles automatic updating on relevant events quite well.
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>There are entity actions that this component reacts to, but they are not intended to be executed outside the c++ code of this component. They are related to browser syncing across clients.
</ul>

</td></tr>

Does not emit any actions.

<b>Depends on components EC_3DCanvas and EC_Mesh. Depends on modules TundraLogicModule for Client and Server classes and KristalliProtocolModule for UserConnection class.</b>.

</table>
*/
class EC_WebView : public IComponent
{
    DECLARE_EC(EC_WebView);
    Q_OBJECT

public:
    //! Webview URL.
    Q_PROPERTY(QString webviewUrl READ getwebviewUrl WRITE setwebviewUrl);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, webviewUrl);

    //! Webview widget size.
    Q_PROPERTY(QSize webviewSize READ getwebviewSize WRITE setwebviewSize);
    DEFINE_QPROPERTY_ATTRIBUTE(QSize, webviewSize);

    //! Rendering target submesh index.
    Q_PROPERTY(int renderSubmeshIndex READ getrenderSubmeshIndex WRITE setrenderSubmeshIndex);
    DEFINE_QPROPERTY_ATTRIBUTE(int, renderSubmeshIndex);

    //! Rendering refresh rate.
    Q_PROPERTY(int renderRefreshRate READ getrenderRefreshRate WRITE setrenderRefreshRate);
    DEFINE_QPROPERTY_ATTRIBUTE(int, renderRefreshRate);

    //! Boolean for interactive mode, if true it will show context menus on mouse click events.
    Q_PROPERTY(bool interactive READ getinteractive WRITE setinteractive);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, interactive);

    //! Interaction ID for components to exchange messages.
    /// \note this attribute is hidden from the UI layer and is not meant to be modified by normal users.
    Q_PROPERTY(int controllerId READ getcontrollerId WRITE setcontrollerId);
    DEFINE_QPROPERTY_ATTRIBUTE(int, controllerId);
    
    //! Destructor.
    ~EC_WebView();

    //! Returns if Component is serializable, always returns true.
    virtual bool IsSerializable() const;

    //! Event filter for this QObject
    virtual bool eventFilter(QObject *obj, QEvent *e);

    //! Server side initialize.
    void ServerInitialize(TundraLogic::Server *server);

public slots:
    //! Render our QWebViews current state with current Attributes to our EC_3DCanvas.
    void Render();

    //! If you want to show the interaction menu/actions in your own place, use this to retrieve the menu.
    //! It is the callers responsibility to destroy the returned QMenu ptr. For example use menu->deleteLater() once you are done with it.
    /// \param bool createSubmenu Defines if all the actions are in the QMenu or as subitems in it. If created the submenu will be called "Browser".
    /// \return QMenu* A menu with actions connected to the correct handlers inside this component.
    /// \note If you do custom menu handling set the 'interactive' boolean to false so we wont try to double popup QMenus.
    QMenu *GetInteractionMenu(bool createSubmenu = true);

private slots:
    //! Server side handler for user disconnects.
    void ServerHandleDisconnect(int connectionID, UserConnection *connection);

    //! Server side handler for attribute changes. Only interested in 'controllerId'.
    void ServerHandleAttributeChange(IAttribute *attribute, AttributeChange::Type changeType);

    /*! Server side handler for checking controller id against connected clients. 
        The id must never be set in any component to a connection id that is not connected.
        Fixes situations some rarer situations e.g. server is shut down when someone has control and something auto stores the conmponent data,
        leaving a connection id to the scene description. When server loads this then clients cannot take control before the controller id is assigned to new client.
    */
    void ServerCheckControllerValidity(int connectionID);

    //! For internals to do delayed rendering due to e.g. widget resize or submesh index change.
    /// \note Depending if the 'renderRefreshRate' is a valid number (0-25) we use the timer, otherwise a QTimer::singleShot().
    void RenderDelayed();

    //! Free QWebView memory and reset internal pointer.
    void ResetWidget();

    //! Prepares everything related to the parent widget and other needed components.
    void PrepareComponent();

    //! Prepares the QWebView and connects its signals to our slots.
    void PrepareWebview();

    //! Handler for QWebView::linkClicked(const QUrl&)
    void LoadRequested(const QUrl &url);

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

    //! Handler when EC_Mesh emits that the mesh is ready.
    void TargetMeshReady();

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

    //! Monitors entity mouse clicks.
    void EntityClicked(Scene::Entity *entity, Qt::MouseButton button, RaycastResult *raycastResult);

    //! Handles request to show the QWebView
    void InteractShowRequest();

    //! Handles requests to take control of sharing
    void InteractControlRequest();

    //! Handles requests to release control of sharing
    void InteractControlReleaseRequest();

    //! Enables/disables the QWebView scrollbars.
    void InteractEnableScrollbars(bool enabled);

    //! Handles entity action WebViewControllerChanged
    /// \note The action signature is (string)"WebViewControllerChanged", (int)"id", (string)"name"
    void ActionControllerChanged(QString id, QString newController);

    //! Handles entity action WebViewScroll
    /// \note The action signature is (string)"WebViewScroll", (int)"x", (int)"y"
    void ActionScroll(QString x, QString y);

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

    //! Metadata for toggling UI visibility when interaction mode changes.
    AttributeMetadata *interactionMetaData_;

    //! Our own connection id.
    int myControllerId_;

    //! Name of the client that has current control
    QString currentControllerName_;

    //! Tracking the scroll position when we are in control.
    QPoint controlledScrollPos_;
};

#endif
