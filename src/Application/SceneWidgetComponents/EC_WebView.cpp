// For conditions of distribution and use, see copyright notice in LICENSE

#include "Math/MathNamespace.h"
#include "DebugOperatorNew.h"

#include "EC_WebView.h"
#include "SceneWidgetComponents.h"
#include "EC_WidgetCanvas.h"

#include "Framework.h"
#include "IModule.h"
#include "SceneAPI.h"
#include "SceneInteract.h"
#include "IRenderer.h"
#include "Entity.h"
#include "AttributeMetadata.h"
#include "Application.h"
#include "UiAPI.h"
#include "UiMainWindow.h"
#include "TundraLogicModule.h"
#include "Client.h"
#include "Server.h"
#include "UserConnection.h"
#include "EC_Mesh.h"
#include "LoggingFunctions.h"

#include <QWebView>
#include <QWebFrame>
#include <QUrl>
#include <QCursor>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QWheelEvent>
#include <QApplication>
#include <QUuid>
#include <QNetworkReply>

#ifdef SCENEWIDGET_BROWSER_SHARED_DATA
#include "BrowserUiPlugin.h"
#include "CookieJar.h"
#include <QNetworkDiskCache>
#endif

#include "MemoryLeakCheck.h"

static int NoneControlID = -1;

EC_WebView::EC_WebView(Scene *scene) :
    IComponent(scene),
    webview_(0),
    renderTimer_(0),
    webviewLoading_(false),
    webviewHasContent_(false),
    componentPrepared_(false),
    myControllerId_(0),
    currentControllerName_(""),
    webviewUrl(this, "View URL", QString()),
    webviewSize(this, "View Size", QPoint(800,600)),
    renderSubmeshIndex(this, "Render Submesh", 0),
    renderRefreshRate(this, "Render FPS", 0),
    interactive(this, "Interactive", false),
    controllerId(this, "ControllerId", NoneControlID),
    illuminating(this, "Illuminating", true),
    enabled(this, "Enabled", true)
{
    interactionMetaData_ = new AttributeMetadata(); /**< @todo memory leak */

    static AttributeMetadata submeshMetaData;
    static AttributeMetadata refreshRateMetadata;
    static AttributeMetadata nonDesignableMetadata;
    static bool metadataInitialized = false;
    if (!metadataInitialized)
    {
        submeshMetaData.minimum = "0";
        submeshMetaData.step = "1";
        refreshRateMetadata.minimum = "0";
        refreshRateMetadata.maximum = "25";
        refreshRateMetadata.step = "1";
        nonDesignableMetadata.designable = false;
        metadataInitialized = true;
    }
    renderSubmeshIndex.SetMetadata(&submeshMetaData);
    renderRefreshRate.SetMetadata(&refreshRateMetadata);
    controllerId.SetMetadata(&nonDesignableMetadata);

    // Initializations depending if we are on the server or client
    TundraLogic::TundraLogicModule *tundraLogic = GetFramework()->GetModule<TundraLogic::TundraLogicModule>();
    if (tundraLogic)
    {
        if (tundraLogic->IsServer())
            ServerInitialize(tundraLogic->GetServer().get());
        else
            myControllerId_ = tundraLogic->GetClient()->GetConnectionID();
    }

    // Don't do anything beyond if rendering is not enabled
    // aka headless server. UI enabled server may continue, but they are not going
    // to have perfect browsing sync as the entity actions are sent to "peers" as in other clients.
    if (!ViewEnabled() || GetFramework()->IsHeadless())
        return;

    // Connect window size changes to update rendering as the ogre textures go black.
    if (GetFramework()->Ui()->MainWindow())
        connect(GetFramework()->Ui()->MainWindow(), SIGNAL(WindowResizeEvent(int,int)), SLOT(RenderWindowResized()), Qt::UniqueConnection);

    // Connect signals from IComponent
    connect(this, SIGNAL(ParentEntitySet()), SLOT(PrepareComponent()), Qt::UniqueConnection);
    
    // Prepare render timer
    renderTimer_ = new QTimer(this);
    connect(renderTimer_, SIGNAL(timeout()), SLOT(Render()), Qt::UniqueConnection);
    resizeRenderTimer_ = new QTimer(this);
    resizeRenderTimer_->setSingleShot(true);
    connect(resizeRenderTimer_, SIGNAL(timeout()), SLOT(RenderDelayed()), Qt::UniqueConnection);

    // Prepare scene interactions
    SceneInteract *sceneInteract = GetFramework()->GetModule<SceneInteract>();
    if (sceneInteract)
    {
        connect(sceneInteract, SIGNAL(EntityClicked(Entity*, Qt::MouseButton, RaycastResult*)), 
                SLOT(EntityClicked(Entity*, Qt::MouseButton, RaycastResult*)));
    }
}

EC_WebView::~EC_WebView()
{
    disconnect();
    ResetWebView(true);
}

bool EC_WebView::eventFilter(QObject *obj, QEvent *e)
{
    if (webview_)
    {
        if (obj == webview_ || obj == webview_->page())
        {
            switch (e->type())
            {
                case QEvent::UpdateRequest:
                case QEvent::Paint:
                {
                    QPoint posNow = webview_->page()->mainFrame()->scrollPosition();
                    if (controlledScrollPos_ != posNow)
                    {
                        controlledScrollPos_ = posNow;
                        // Send scroll position update to peers when we are in control.
                        if (getcontrollerId() == myControllerId_)
                        {
                            ParentEntity()->Exec(4, "WebViewScroll", 
                                QString::number(controlledScrollPos_.x()), QString::number(controlledScrollPos_.y()));
                        }
                        RenderDelayed();
                    }
                    break;
                }
                case QEvent::Wheel:
                {
                    QWheelEvent *wheelEvent = dynamic_cast<QWheelEvent*>(e);
                    if (wheelEvent)
                    {
                        // Deny wheel scroll if webview is being controlled externally.
                        int currentControllerId = getcontrollerId();
                        if (currentControllerId != NoneControlID &&
                            currentControllerId != myControllerId_)
                            return true;
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
    return false;
}

void EC_WebView::ServerInitialize(TundraLogic::Server *server)
{
    if (!server || !server->IsRunning())
        return;
    connect(server, SIGNAL(UserDisconnected(int, UserConnection*)), SLOT(ServerHandleDisconnect(int, UserConnection*)), Qt::UniqueConnection);
}

void EC_WebView::ServerHandleDisconnect(int connectionID, UserConnection* connection)
{
    // Server will release the control of a EC_Webview if the controlling user disconnects.
    // This will ensure that a webview wont be left in a state that no one can take control of it.
    if (getcontrollerId() != NoneControlID)
    {
        if (getcontrollerId() == connectionID)
        {
            setcontrollerId(NoneControlID);
            ParentEntity()->Exec(4, "WebViewControllerChanged", QString::number(NoneControlID), "");
        }
    }
}

void EC_WebView::ServerCheckControllerValidity(int connectionID)
{
    TundraLogic::TundraLogicModule *tundraLogic = GetFramework()->GetModule<TundraLogic::TundraLogicModule>();
    if (tundraLogic)
    {
        if (connectionID != NoneControlID)
        {
            /// Special case: a non-headless server takes control. We have a reserved id of -2 for this.
            /// \note You should not control browsers from a non headless server, this will get you in trouble
            /// when your connection id -2 gets into any .txml/.tbin files!
            if (connectionID == -2)
                return;

            UserConnection *user = tundraLogic->GetServer()->GetUserConnection(connectionID).get();
            if (!user)
            {
                // The ID is not a valid active connection, reset the control id to all clients.
                setcontrollerId(NoneControlID);
                ParentEntity()->Exec(4, "WebViewControllerChanged", QString::number(NoneControlID), "");
            }
        }
    }
}

// Protected

void EC_WebView::Render(QImage image)
{
    // Don't do anything if rendering is not enabled
    if (!ViewEnabled() || GetFramework()->IsHeadless())
        return;

    // If not enabled don't render
    if (!getenabled())
        return;
    if (!componentPrepared_)
        return;

    // We may have switched to the local mode in the meanwhile
    if (getcontrollerId() != NoneControlID || getrenderRefreshRate() > 0)
        return;
    if (webview_ && webview_->isVisible())
    {
        RenderDelayed();
        return;
    }

    // Get needed components, something is fatally wrong if these are not present but componentPrepared_ is true.
    EC_Mesh *mesh = GetMeshComponent();
    EC_WidgetCanvas *sceneCanvas = GetSceneCanvasComponent();
    if (!mesh || !sceneCanvas)
    {
        // In the case someone destroyed EC_WidgetCanvas or EC_Mesh from our entity
        // lets stop our running timer (if its running), so we don't unnecessarily poll here.
        RenderTimerStop();
        componentPrepared_ = false;
        return;
    }

    // Validate submesh index from EC_Mesh
    uint submeshIndex = (uint)getrenderSubmeshIndex();
    if (submeshIndex >= mesh->GetNumSubMeshes())
    {
        /// \note ResetSubmeshIndex() is called with a small delay here, or the ec editor UI wont react to it. Resetting the index back to 0 will call Render() again.
        LogWarning("Render submesh index " + QString::number(submeshIndex).toStdString() + " is illegal, restoring default value.");
        QTimer::singleShot(1, this, SLOT(ResetSubmeshIndex()));
        return;
    }

    // Set submesh to EC_WidgetCanvas if different from current
    if (!sceneCanvas->GetSubMeshes().contains(submeshIndex))
        sceneCanvas->SetSubmesh(submeshIndex);

    // Reset widget ptr as we are now using direct image updates
    if (sceneCanvas->GetWidget() != 0)
        sceneCanvas->SetWidget(0);

    sceneCanvas->Update(image);   
}

// Public slots

void EC_WebView::Render()
{
    // Don't do anything if rendering is not enabled
    if (!ViewEnabled() || GetFramework()->IsHeadless())
        return;

    // If not enabled don't render
    if (!getenabled())
        return;

    if (!webview_)
    {
        // Redirect to the rendering queue
        if (getrenderRefreshRate() == 0 && getcontrollerId() == NoneControlID)
            LoadUrl(getwebviewUrl().simplified());
        else
            LogWarning("EC_WebView::Render: Webview is null, aborting!");
        return;
    }

    if (!componentPrepared_)
        return;
    if (webviewLoading_)
        return;
    if (!webviewHasContent_)
        return;

    // Get needed components, something is fatally wrong if these are not present but componentPrepared_ is true.
    EC_Mesh *mesh = GetMeshComponent();
    EC_WidgetCanvas *sceneCanvas = GetSceneCanvasComponent();
    if (!mesh || !sceneCanvas)
    {
        // In the case someone destroyed EC_WidgetCanvas or EC_Mesh from our entity
        // lets stop our running timer (if its running), so we don't unnecessarily poll here.
        RenderTimerStop();
        componentPrepared_ = false;
        return;
    }

    // Validate submesh index from EC_Mesh
    uint submeshIndex = (uint)getrenderSubmeshIndex();
    if (submeshIndex >= mesh->GetNumSubMeshes())
    {
        /// \note ResetSubmeshIndex() is called with a small delay here, or the ec editor UI wont react to it. Resetting the index back to 0 will call Render() again.
        LogWarning("Render submesh index " + QString::number(submeshIndex).toStdString() + " is illegal, restoring default value.");
        QTimer::singleShot(1, this, SLOT(ResetSubmeshIndex()));
        return;
    }
    
    // Set submesh to EC_WidgetCanvas if different from current
    if (!sceneCanvas->GetSubMeshes().contains(submeshIndex))
        sceneCanvas->SetSubmesh(submeshIndex);
    
    // Set widget to EC_WidgetCanvas if different from current
    if (sceneCanvas->GetWidget() != webview_)
        sceneCanvas->SetWidget(webview_);

    sceneCanvas->Update();
}

QMenu *EC_WebView::GetInteractionMenu(bool createSubmenu)
{
    int currentControlId = getcontrollerId();
    
    QString installDir = Application::InstallationDirectory();
    if (installDir.isEmpty())
        installDir = "./";
    
    QMenu *actionMenu = new QMenu(0);
    actionMenu->addAction(QIcon(installDir + "data/ui/images/icon/browser.ico"), "Show", this, SLOT(InteractShowRequest()));
    if (currentControlId == NoneControlID && !webviewLoading_)
        actionMenu->addAction("Share Browsing", this, SLOT(InteractControlRequest()));
    else if (currentControlId == NoneControlID && webviewLoading_)
        actionMenu->addAction("Loading page, please wait...")->setEnabled(false);
    else
    {
        // We have the control
        if (currentControlId == myControllerId_)
            actionMenu->addAction("Release Shared Browsing", this, SLOT(InteractControlReleaseRequest()));
        // Another client has control
        else
        {
            actionMenu->addSeparator();
            if (currentControllerName_.trimmed().isEmpty())
                actionMenu->addAction("Controlled by another client")->setEnabled(false);
            else
                actionMenu->addAction("Controlled by " + currentControllerName_.trimmed())->setEnabled(false);
        }
    }

    // Create root menu or return the action list
    if (createSubmenu)
    {
        QMenu *rootMenu = new QMenu(0);
        
        QMenu *subMenu = rootMenu->addMenu(QIcon(installDir + "data/ui/images/icon/browser.ico"), "Browser");
        subMenu->addActions(actionMenu->actions());
        return rootMenu;
    }
    else
        return actionMenu;
}

// Private slots

void EC_WebView::RenderDelayed()
{
    if (!componentPrepared_)
        return;
    if (!getenabled())
        return;

    // If timer does not exist or is not active, 
    // invoke a single shot to our Render() function.
    // If the timer is active, it meas we dont want to double call Render()
    // the timer will invoke it once the timeout() is emitted next time.
    if (renderTimer_)
    {
        if (!renderTimer_->isActive())
            QTimer::singleShot(10, this, SLOT(Render()));
    }
    else
        QTimer::singleShot(10, this, SLOT(Render()));
}

void EC_WebView::RenderWindowResized()
{
    if (!resizeRenderTimer_)
        return;

#if defined(DIRECTX_ENABLED) && defined(WIN32)
    // Rendering goes black on the texture when 
    // windows is resized only on directx
    if (!resizeRenderTimer_->isActive())
        resizeRenderTimer_->start(500);
#endif
}

void EC_WebView::PrepareComponent()
{
    // Don't do anything if rendering is not enabled
    if (!ViewEnabled() || GetFramework()->IsHeadless())
        return;
    if (componentPrepared_)
        LogWarning("PrepareComponent: Preparations seem to be done already, you might not want to do this multiple times.");

    // Get parent and connect to the component removed signal.
    Entity *parent = ParentEntity();
    if (parent)
    {
        connect(parent, SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)), SLOT(ComponentRemoved(IComponent*, AttributeChange::Type)), Qt::UniqueConnection);
        parent->ConnectAction("WebViewControllerChanged", this, SLOT(ActionControllerChanged(QString, QString)));
        parent->ConnectAction("WebViewScroll", this, SLOT(ActionScroll(QString, QString)));
    }
    else
    {
        LogError("PrepareComponent: Could not get parent entity pointer!");
        return;
    }

    // Get EC_Mesh component
    EC_Mesh *mesh = GetMeshComponent();
    if (!mesh)
    {
        // Wait for EC_Mesh to be added.
        connect(parent, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), SLOT(ComponentAdded(IComponent*, AttributeChange::Type)), Qt::UniqueConnection);
        return;
    }
    else
    {
        // Inspect if this mesh is ready for rendering. EC_Mesh being present != being loaded into Ogre and ready for rendering.
        if (!mesh->GetEntity())
        {
            connect(mesh, SIGNAL(MeshChanged()), SLOT(TargetMeshReady()), Qt::UniqueConnection);
            return;
        }
        else
            connect(mesh, SIGNAL(MaterialChanged(uint, const QString&)), SLOT(TargetMeshMaterialChanged(uint, const QString&)), Qt::UniqueConnection);
    }
    
    if (sceneCanvasName_.isEmpty())
        sceneCanvasName_ = "WebViewCanvas-" + QUuid::createUuid().toString().replace("{", "").replace("}", "");

    // Get or create local EC_WidgetCanvas component
    ComponentPtr iComponent = parent->GetOrCreateComponent(EC_WidgetCanvas::TypeNameStatic(), sceneCanvasName_, AttributeChange::LocalOnly, false);
    EC_WidgetCanvas *sceneCanvas = dynamic_cast<EC_WidgetCanvas*>(iComponent.get());
    if (!sceneCanvas)
    {
        LogError("PrepareComponent: Could not get or create EC_WidgetCanvas component!");
        return;
    }
    sceneCanvas->SetTemporary(true);
    sceneCanvas->SetSelfIllumination(getilluminating());
    
    // All the needed components are present, mark prepared as true.
    componentPrepared_ = true;

    // We are now prepared, check enabled state and restore possible materials now
    if (!getenabled())
        sceneCanvas->RestoreOriginalMeshMaterials();

    if (webview_)
    {
        // Resize the widget if needed before loading the page.
        QSize targetSize = QSize(getwebviewSize().x(), getwebviewSize().y());
        if (webview_->size() != targetSize)
            webview_->setFixedSize(targetSize);
    }

    LoadUrl(getwebviewUrl().simplified());
}

void EC_WebView::PrepareWebview()
{
    // Don't do anything if rendering is not enabled
    if (!ViewEnabled() || GetFramework()->IsHeadless())
        return;
    if (webview_)
        return;

    webviewLoading_ = false;
    webviewHasContent_ = false;

    // Do not set our main window as the parent so we can our selves delete
    // the widget on dtor. This will result in the Qt::Tool window going behind the main win
    // when you set focus on it, but this is something we have to accept on the ui side of things.
    // Otherwise we cant delete the webview on exit, or if we do crash on UiAPI::~UiAPI()!
    webview_ = new QWebView();
    webview_->setWindowFlags(Qt::Tool);
    webview_->setFixedSize(QSize(getwebviewSize().x(), getwebviewSize().y()));
    webview_->installEventFilter(this);
    webview_->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

    connect(webview_, SIGNAL(linkClicked(const QUrl&)), this, SLOT(LoadRequested(const QUrl&)), Qt::UniqueConnection);
    connect(webview_, SIGNAL(loadStarted()), this, SLOT(LoadStarted()), Qt::UniqueConnection);
    connect(webview_, SIGNAL(loadFinished(bool)), this, SLOT(LoadFinished(bool)), Qt::UniqueConnection);

    /// @todo Use shared access manager from SceneWidgetComponent module?
    QNetworkAccessManager *networkAccess = webview_->page()->networkAccessManager();
    if (networkAccess)
    {
#ifndef QT_NO_OPENSSL
        connect(networkAccess, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)), 
            this, SLOT(OnSslErrors(QNetworkReply*, const QList<QSslError>&)), Qt::UniqueConnection);
#endif

#ifdef SCENEWIDGET_BROWSER_SHARED_DATA
        BrowserUiPlugin *browserPlugin = framework->GetModule<BrowserUiPlugin>();
        if (browserPlugin)
        {
            // Shared disk cache and cookies for all browsers
            networkAccess->setCache(browserPlugin->MainDiskCache());
            if (networkAccess->cache()) networkAccess->cache()->setParent(0);
            networkAccess->setCookieJar(browserPlugin->MainCookieJar());
            if (networkAccess->cookieJar()) networkAccess->cookieJar()->setParent(0);
        }
#endif
    }

    // Load current url
    LoadUrl(getwebviewUrl().simplified());
}

void EC_WebView::ResetWebView(bool ignoreVisibility)
{
    if (webview_)
    {
        if (!ignoreVisibility && webview_->isVisible())
            return;

        // Reset the EC_WidgetCanvas data for added safety. Restore original materials.
        // If we come here from dtor this wont happen as parent entity has been reseted.
        EC_WidgetCanvas *sceneCanvas = GetSceneCanvasComponent();
        if (sceneCanvas)
        {
            if (sceneCanvas->GetWidget() == webview_)
            {
                sceneCanvas->RestoreOriginalMeshMaterials();
                sceneCanvas->SetWidget(0);
            }
        }

        // Reset parent of shard cache and cookie jar.
        QNetworkAccessManager *networkManager =  webview_->page() != 0 ? webview_->page()->networkAccessManager() : 0;
        if (networkManager)
        {
            if (networkManager->cache()) networkManager->cache()->setParent(0);
            if (networkManager->cookieJar()) networkManager->cookieJar()->setParent(0);
        }

        // Disconnect existing widgets signal connections, 
        // stop its networking, 
        // mark it for Qt cleanup and
        // reset our internal ptr.
        webview_->disconnect();
        if (webviewLoading_)
        {
            webview_->stop();
            webviewLoading_ = false;
        }
        SAFE_DELETE(webview_)
    }

    RenderTimerStop();
    webviewHasContent_ = false;
}

#ifndef QT_NO_OPENSSL
void EC_WebView::OnSslErrors(QNetworkReply *reply, const QList<QSslError>& errors)
{
    LogWarning("EC_WebView: Could not load page, ssl errors occurred in url '" + getwebviewUrl() + "'");
    if (errors.isEmpty())
        LogWarning("- Unknown SSL error");
    else
    {
        foreach(QSslError err, errors)
            LogWarning("- " + err.errorString());
    }
    StopBrowser();
}
#endif

void EC_WebView::LoadRequested(const QUrl &url)
{
    // If we are sharing browsing, share the url to everyone.
    // Otherwise load locally. If we are clicking links and someone
    // else has control, do nothing.
    int currentControllerId = getcontrollerId();
    if (currentControllerId == myControllerId_)
        setwebviewUrl(url.toEncoded());
    else if (currentControllerId == NoneControlID)
        LoadUrl(url.toString());
}

void EC_WebView::LoadUrl(QString urlString)
{
    if (!componentPrepared_)
        return;

    if (urlString.isEmpty())
    {
        RenderTimerStop();

        // Restore the original materials from the mesh if user sets url to empty string.
        EC_WidgetCanvas *sceneCanvas = GetSceneCanvasComponent();
        if (sceneCanvas)
            sceneCanvas->RestoreOriginalMeshMaterials();
        return;
    }

    if (!getenabled())
        return;

    // Add http in front or strict mode parsing will fail.
    if (!urlString.startsWith("http://") && !urlString.startsWith("https://"))
        urlString = "http://" + urlString;
    
    // Don't automatically do percent encoding etc with strict mode.
    QUrl url = QUrl::fromPercentEncoding(urlString.toAscii());
    if (url.isValid())
    {
        // Someone is controlling the browsing or frequent 
        // updates are needed, instantiate and use a local QWebView.
        if (getrenderRefreshRate() > 0 || getcontrollerId() != NoneControlID)
        {
            if (!webview_)
                PrepareWebview();
        }
        else
        {
            ResetWebView();

            // Above won't reset widget if its visible, then we use local instance
            if (!webview_)
            {
                SceneWidgetComponents *sceneComponentsPlugin = GetFramework()->GetModule<SceneWidgetComponents>();
                if (sceneComponentsPlugin)
                    sceneComponentsPlugin->WebRenderingRequest(this, url, QSize(getwebviewSize().x(), getwebviewSize().y()));
                else
                    LogError("EC_WebView: Failed to get SceneWidgetComponents module!");
            }
        }

        if (webview_)
        {
            if (webview_->url() != url)
                webview_->load(url);
            else
                RenderDelayed();
        }
    }
    else
        LogError("EC_WebView: Invalid url '" + url.toString() + "'. Did you remember to input the percent encoded form of the url?");
}

void EC_WebView::LoadStarted()
{
    RenderTimerStop();
    if (!webview_)
        return;

    webviewLoading_ = true;

    QString loadedUrl = getwebviewUrl().simplified();
    QString title;
    if (getcontrollerId() != NoneControlID)
    {
        if (getcontrollerId() != myControllerId_) 
            title = "Controller by " + currentControllerName_ + " - " + loadedUrl;
        else
            title = "Controller by you - " + loadedUrl;
    }
    else
        title = loadedUrl;
    webview_->setWindowTitle(title);
}

void EC_WebView::LoadFinished(bool success)
{
    if (!webview_)
        return;

    QString loadedUrl = webview_->url().toString().trimmed();
    webviewLoading_ = false;
    if (success)
    {
        webviewHasContent_ = true;

        // Construct the ui title to tell user who is in control of the browsing.
        QString title;
        if (getcontrollerId() != NoneControlID)
        {
            if (getcontrollerId() != myControllerId_) 
               title = "Controller by " + currentControllerName_ + " - " + loadedUrl;
            else
               title = "Controller by you - " + loadedUrl;
        }
        else
            title = loadedUrl;
        webview_->setWindowTitle(title);
    }
    else
    {
        if (!loadedUrl.isEmpty())
            webview_->setWindowTitle("Error while loading " + loadedUrl);
    }

    RenderTimerStartOrSingleShot();
}

void EC_WebView::StopBrowser()
{
    if (!webview_)
        return;

    webview_->stop();
    webviewLoading_ = false;
    webviewHasContent_ = false;
}

void EC_WebView::ResetSubmeshIndex()
{
    setrenderSubmeshIndex(0);
}

void EC_WebView::RenderTimerStop()
{
    if (renderTimer_ && renderTimer_->isActive())
        renderTimer_->stop();
}

void EC_WebView::RenderTimerStartOrSingleShot()
{
    if (renderTimer_)
    {
        if (getrenderRefreshRate() > 0)
        {
            PrepareWebview();

            // Clamp FPS to 0-25, the EC editor UI does this for us with AttributeMetaData,
            // but someone can inject crazy stuff here directly from code
            int rateNow = 1000 / getrenderRefreshRate();
            if (rateNow < 40)
                rateNow = 40; // Max allowed FPS 25 == 40ms timer timeout
            renderTimer_->start(rateNow);
        }
        else
            QTimer::singleShot(10, this, SLOT(Render()));
    }
    else
        QTimer::singleShot(10, this, SLOT(Render()));

    // If we get here we can safely reset the widget
    // if no one has browser control
    if (getrenderRefreshRate() == 0 && getcontrollerId() == NoneControlID)
        ResetWebView();
}

void EC_WebView::TargetMeshReady()
{
    if (!componentPrepared_)
        PrepareComponent();
}

void EC_WebView::TargetMeshMaterialChanged(uint index, const QString &material)
{
    if (!componentPrepared_)
        return;
    if (sceneCanvasName_.isEmpty())
        return;
    if (!ParentEntity())
        return;
    if (!getenabled())
        return;

    if (index == (uint)getrenderSubmeshIndex())
    {
        // Don't create the canvas, if its not there yet there is nothing to re-apply
        IComponent *comp = ParentEntity()->GetComponent(EC_WidgetCanvas::TypeNameStatic(), sceneCanvasName_).get();
        EC_WidgetCanvas *sceneCanvas = dynamic_cast<EC_WidgetCanvas*>(comp);
        if (sceneCanvas)
        {
            if (material != sceneCanvas->GetMaterialName())
            {
                // This will make 3DCanvas to update its internals, which means
                // our material is re-applied to the submesh.
                sceneCanvas->SetSubmesh(getrenderSubmeshIndex());
                RenderDelayed();
            }
        }
    }
}

void EC_WebView::ComponentAdded(IComponent *component, AttributeChange::Type change)
{
    if (component->TypeName() == EC_Mesh::TypeNameStatic())
    {
        if (!componentPrepared_)
            PrepareComponent();
    }
}

void EC_WebView::ComponentRemoved(IComponent *component, AttributeChange::Type change)
{
    /** If this component is being removed we need to reset to the target meshes original materials.
        EC_WidgetCanvas is too stupid to do this (should be improved!) At this stage our parent
        entity is still valid. This will cease to exist if this behavior is changed in SceneManager and/or
        Entity classes. Parent entity is not valid in the dtor so this has to be done here.
        \todo Improve EC_WidgetCanvas to always know when the widget that its rendering is destroyed and reset the original materials.
        \note This implementation relies on parent entity being valid here. If this is changed in the future the above todo must be implemented.
        \note We are going to the dtor after this, it will free our objects, this is why we dont do it here.
    */
    if (component == this)
    {
        // Reset EC_WidgetCanvas
        EC_WidgetCanvas *canvasSource = GetSceneCanvasComponent();
        if (canvasSource && webview_)
        {
            // If the widget that its rendering is ours, restore original materials.
            if (canvasSource->GetWidget() == webview_)
            {
                canvasSource->Stop();
                canvasSource->RestoreOriginalMeshMaterials();
                canvasSource->SetWidget(0);
            }

            // Clean up our EC_WidgetCanvas component from the entity
            if (ParentEntity() && !sceneCanvasName_.isEmpty())
                ParentEntity()->RemoveComponent(EC_WidgetCanvas::TypeNameStatic(), sceneCanvasName_, AttributeChange::LocalOnly);
        }
    }
    /// \todo Add check if this component has another EC_Mesh then init EC_WidgetCanvas again for that! Now we just blindly stop this EC.
    else if (component->TypeName() == EC_Mesh::TypeNameStatic())
    {
        RenderTimerStop();
        componentPrepared_ = false;
    }
}

void EC_WebView::AttributesChanged()
{
    TundraLogic::TundraLogicModule *tundraLogic = GetFramework()->GetModule<TundraLogic::TundraLogicModule>();
    if (tundraLogic)
    {
        if (tundraLogic->IsServer())
        {
            if (controllerId.ValueChanged())
                ServerCheckControllerValidity(getcontrollerId());
            return;
        }
    }

    if (webviewUrl.ValueChanged())
    {
        webviewUrl.ClearChangedFlag();
        LoadUrl(getwebviewUrl().simplified());
    }
    if (webviewSize.ValueChanged())
    {
        webviewSize.ClearChangedFlag();

        // Always keep a fixed size. If user wants to resize the
        // 2D widget, he needs to use the components attribute.
        // This is done to ensure shared browsing will always be same size on all clients.
        // The scroll event x,y positions will always show same content on both controller and slaves.
        QSize targetSize = QSize(getwebviewSize().x(), getwebviewSize().y());
        if (webview_ && webview_->size() != targetSize)
        {
            webview_->setFixedSize(targetSize);
            RenderDelayed();
        }

        // If local webview is null we need to 
        // update the static view with the new resolution
        if (!webview_)
            RenderDelayed();
    }
    if (renderSubmeshIndex.ValueChanged())
    {
        renderSubmeshIndex.ClearChangedFlag();

        // Rendering will make sure this submesh index is not out of range.
        // If it is it will reset the submesh index to 0. This is very nice for the user experience.
        RenderDelayed();
    }
    if (renderRefreshRate.ValueChanged())
    {
        renderRefreshRate.ClearChangedFlag();

        if (!componentPrepared_)
            return;

        RenderTimerStop();
        RenderTimerStartOrSingleShot();
    }
    if (controllerId.ValueChanged())
    {
        controllerId.ClearChangedFlag();

        // If we have control, leave ui so that we can modify the attributes
        int currentControllerId = getcontrollerId();
        if (currentControllerId == myControllerId_)
        {
            EnableScrollbars(true);
            return;
        }

        // If no one has control show scroll bars and reload the current page.
        // Disable scroll bars from local UI if someone else is controlling.
        if (currentControllerId == NoneControlID)
        {
            EnableScrollbars(true);
            LoadUrl(getwebviewUrl().simplified());
        }
        else
            EnableScrollbars(false);

        // If control is not on us
        // 1. No one is controlling = show ui attributes editable
        // 2. Someone else is controlling = hide all of this components attributes, so slaves cannot modify them.
        interactionMetaData_->designable = currentControllerId == NoneControlID ? true : false;
        AttributeVector::iterator iter = attributes.begin();
        AttributeVector::iterator end = attributes.end();
        while(iter != end)
        {
            IAttribute *attr = (*iter);
            // Always keep 'controlId' hidden from users.
            if (attr != &controllerId)
            {
                attr->SetMetadata(interactionMetaData_);
                attr->Changed(AttributeChange::LocalOnly);
            }
            ++iter;
        }
    }
    if (illuminating.ValueChanged())
    {
        illuminating.ClearChangedFlag();

        EC_WidgetCanvas *canvas = GetSceneCanvasComponent();
        if (canvas)
            canvas->SetSelfIllumination(getilluminating());
    }
    if (enabled.ValueChanged())
    {
        enabled.ClearChangedFlag();

        EC_WidgetCanvas *sceneCanvas = GetSceneCanvasComponent();
        if (componentPrepared_ && sceneCanvas)
        {
            RenderTimerStop();
            if (!getenabled())
            {
                sceneCanvas->RestoreOriginalMeshMaterials();
                ResetWebView();
            }
            else
            {
                sceneCanvas->SetSubmesh(getrenderSubmeshIndex());
                RenderTimerStartOrSingleShot();
            }
        }
    }
}

EC_Mesh *EC_WebView::GetMeshComponent()
{
    if (!ParentEntity())
        return 0;
    EC_Mesh *mesh = ParentEntity()->GetComponent<EC_Mesh>().get();
    return mesh;
}

EC_WidgetCanvas *EC_WebView::GetSceneCanvasComponent()
{
    if (!ParentEntity())
        return 0;
    if (sceneCanvasName_.isEmpty())
        return 0;
    IComponent *comp = ParentEntity()->GetComponent(EC_WidgetCanvas::TypeNameStatic(), sceneCanvasName_).get();
    EC_WidgetCanvas *sceneCanvas = dynamic_cast<EC_WidgetCanvas*>(comp);
    return sceneCanvas;
}

void EC_WebView::EntityClicked(Entity *entity, Qt::MouseButton button, RaycastResult *raycastResult)
{
    if (!getinteractive() || !ParentEntity())
        return;
    
    // We are only interested in left clicks on our entity.
    if (!raycastResult)
        return;
    if (button != Qt::LeftButton)
        return;
    
    if (entity == ParentEntity())
    {
        // We are only interested in clicks to our target submesh index.
        if (raycastResult->submesh != (unsigned)getrenderSubmeshIndex())
            return;

        // Entities have EC_Highlight if it is being manipulated.
        // At this situation we don't want to show any ui.
        if (entity->GetComponent("EC_Highlight"))
            return;

        QMenu *popupInteract = GetInteractionMenu(false);
        if (!popupInteract->actions().empty())
            popupInteract->exec(QCursor::pos());
        popupInteract->deleteLater();
    }
}

void EC_WebView::InteractShowRequest()
{    
    /// @todo Rewrite this control hacking stuff so something more sensible.
    /// This is required atm due to the web page rendering queue for non-controlled,
    /// 0-fps EC_WebViews, so we don't have to reserve QWebView in mem.

    // Automatically get control so the 2D widget is created.
    bool autoControlled = false;
    if (getcontrollerId() == NoneControlID && getrenderRefreshRate() <= 0)
    {
        autoControlled = true;
        InteractControlRequest();
    }
        
    if (getcontrollerId() != NoneControlID || getrenderRefreshRate() > 0)
        PrepareWebview();
        
    if (!webview_)
        return;

    if (!webview_->isVisible())
    {
        // Center the webview on the click position if possible.
        QPoint globalMousePos = QCursor::pos();
        QPoint showPos = globalMousePos;
        if (globalMousePos.x() > (webview_->width() / 2))
            showPos.setX(globalMousePos.x() - (webview_->width() / 2));
        if (globalMousePos.y() > (webview_->height() / 2))
            showPos.setY(globalMousePos.y() - (webview_->height() / 2));

        webview_->move(showPos);
    }
    webview_->show();
    webview_->activateWindow();
    QApplication::setActiveWindow(webview_);
    
    // Auto release control if it was only for creating the temporary 2D widget.
    if (autoControlled && getcontrollerId() == myControllerId_ && getrenderRefreshRate() <= 0)
        InteractControlReleaseRequest();
}

void EC_WebView::InteractControlRequest()
{
    if (getcontrollerId() == NoneControlID)
    {
        // Mark us as the controller, this gets synced to all clients
        setcontrollerId(myControllerId_);

        // Send your local webview up to date url to others
        if (webview_)
            setwebviewUrl(webview_->url().toEncoded());
        
        // Resolve our user name, use connection id if not available
        QString myControllerName;
        int myId = -1;
        TundraLogic::TundraLogicModule *tundraLogic = GetFramework()->GetModule<TundraLogic::TundraLogicModule>();
        if (tundraLogic)
        {
            if (!tundraLogic->IsServer())
            {
                myId = myControllerId_;
                myControllerName = tundraLogic->GetClient()->GetLoginProperty("username");
                if (myControllerName.trimmed().isEmpty())
                    myControllerName = "client #" + QString::number(myId);
            }
            else
            {
                // Not sure if 0 is reserved for the server. Lets stick with -2 being server.
                // Note that this quite a rare case that server wants to control outside of dev testing.
                // Also note that servers wont get sync browsing as the entity actions are sent to peers.
                myControllerName = "server";
                myId = -2;
            }
        }

        // Execute entity action to peers aka other clients.
        ParentEntity()->Exec(4, "WebViewControllerChanged", QString::number(myId), myControllerName);
        EnableScrollbars(true);
    }
    else
        QMessageBox::information(GetFramework()->Ui()->MainWindow(),
            "Control Request Denied", "Another client already has control of this web browser.");
}

void EC_WebView::InteractControlReleaseRequest()
{
    if (getcontrollerId() == NoneControlID)
        LogWarning("Seems like anyone does not have control? Making sure by releasing again.");
    setcontrollerId(NoneControlID);
    ParentEntity()->Exec(4, "WebViewControllerChanged", QString::number(NoneControlID), "");
}

void EC_WebView::EnableScrollbars(bool enabled)
{
    if (!webview_)
        return;

    Qt::ScrollBarPolicy policy = enabled ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff;
    webview_->page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, policy);
    webview_->page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, policy);
    RenderDelayed();
}

void EC_WebView::ActionControllerChanged(QString id, QString newController)
{
    currentControllerName_ = newController.trimmed();
}

void EC_WebView::ActionScroll(QString x, QString y)
{
    int currentControlId = getcontrollerId();
    if (currentControlId != NoneControlID && currentControlId != myControllerId_)
    {
        if (!webview_)
            PrepareWebview();

        QPoint scrollPos(x.toInt(), y.toInt());
        webview_->page()->mainFrame()->setScrollPosition(scrollPos);
        Render();
    }
}
