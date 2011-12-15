// For conditions of distribution and use, see copyright notice in license.txt

#include "SceneWidgetComponents.h"

#include "Framework.h"
#include "Math/float2.h"
#include "LoggingFunctions.h"

#include "InputAPI.h"
#include "MouseEvent.h"
#include "InputContext.h"

#include "SceneAPI.h"
#include "Scene.h"
#include "Entity.h"

#include "UiAPI.h"
#include "UiGraphicsView.h"

#include "EC_WebView.h"
#include "EC_WidgetBillboard.h"

#include <QPainter>
#include <QNetworkReply>
#include <QDebug>

#ifdef SCENEWIDGET_BROWSER_SHARED_DATA
#include "BrowserUiPlugin.h"
#include "CookieJar.h"
#include <QNetworkDiskCache>
#endif

SceneWidgetComponents::SceneWidgetComponents() :
    IModule("SceneWidgetComponents")
{
}

SceneWidgetComponents::~SceneWidgetComponents()
{
}

void SceneWidgetComponents::Initialize()
{
    if (!framework_->IsHeadless())
        connect(framework_->Input()->TopLevelInputContext(), SIGNAL(MouseEventReceived(MouseEvent*)), SLOT(OnMouseEvent(MouseEvent*)));
}

void SceneWidgetComponents::Uninitialize()
{
    webRenderRequests_.clear();
    ResetWebWidget();
}

void SceneWidgetComponents::OnMouseEvent(MouseEvent *mEvent)
{
    MouseEvent::EventType et = mEvent->eventType;

    if (framework_->IsHeadless())
        return;
    if (!framework_->Scene()->MainCameraScene())
        return;
    if (framework_->Ui()->GraphicsView()->GetVisibleItemAtCoords(mEvent->x, mEvent->y) != 0)
        return;

    // Filter out not wanted events here so we don't 
    // do the potentially costly raycast unnecessarily.
    if (mEvent->handled || mEvent->IsRightButtonDown())
        return;
    else if (et == MouseEvent::MouseScroll)
        return;

    float closestDistance = 100000.0;
    EC_WidgetBillboard *closestComponent = 0;

    EntityList ents = framework_->Scene()->MainCameraScene()->GetEntitiesWithComponent(EC_WidgetBillboard::TypeNameStatic());
    EntityList::const_iterator iter = ents.begin();

    // Find the closest hit EC_WidgetBillboard
    while (iter != ents.end())
    {
        EntityPtr ent = *iter;
        ++iter;

        if (!ent.get())
            continue;

        EC_WidgetBillboard *widgetBillboard = dynamic_cast<EC_WidgetBillboard*>(ent->GetComponent(EC_WidgetBillboard::TypeNameStatic()).get());
        if (!widgetBillboard)
            continue;

        bool hit = false; float2 uv; float distance;
        widgetBillboard->RaycastBillboard(mEvent->x, mEvent->y, hit, uv, distance);

        if (hit)
        {
            if (distance < closestDistance)
            {
                closestDistance = distance;
                closestComponent = widgetBillboard;
            }
        }
        else
            widgetBillboard->CheckMouseState();
    }

    // Redirect mouse event for the closest EC_WidgetBillboard
    if (closestComponent)
    {
        closestComponent->OnMouseEvent(mEvent);
        mEvent->handled = true;
    }
}

void SceneWidgetComponents::WebRenderingRequest(EC_WebView *client, QUrl url, QSize resolution)
{
    if (framework_->IsHeadless())
        return;

    if (resolution.width() <= 0 || resolution.height() <= 0)
    {
        LogError("WebRenderingRequest: Resolution height or width cannot be <= 0");
        return;
    }

    WebRenderRequest request(client, url, resolution);

    // Check if we are already processing identical request.
    if (request.IsIdentical(processingRequest_))
    {
        LogWarning("WebRenderingRequest: Current request already identical, returning!");
        return;
    }

    // Check if we have pending request with identical data.
    // Only keep one request per client in the pending list, remove older ones.
    QList<WebRenderRequest> removeList;
    foreach(WebRenderRequest existingRequest, webRenderRequests_)
    {
        if (request.IsIdentical(existingRequest))
        {
            LogWarning("WebRenderingRequest: Pending request already identical, returning!");
            return;
        }
        if (request.client == existingRequest.client)
            removeList << existingRequest;
    }
    foreach(WebRenderRequest removeRequest, removeList)
    {
        LogInfo("WebRenderingRequest: Removing same client request with id " + QString::number(removeRequest.id));
        webRenderRequests_.removeAll(removeRequest);
    }

    // Assign ID for the request
    request.id = idGenerator_.AllocateReplicated();
    webRenderRequests_ << request;

    qDebug() << "New render request" << endl 
        << "client :" << request.client << endl
        << "id     :" << request.id << endl
        << "url    :" << request.url.toString() << endl
        << "size   :" << request.resolution;

    ProcessNextRenderingRequest();
}

void SceneWidgetComponents::ProcessNextRenderingRequest()
{
    // Nothing is pending
    if (webRenderRequests_.isEmpty() && processingRequest_.id == 0)
    {
        ResetWebWidget();
        return;
    }
    // Already processing a request
    if (processingRequest_.id != 0)
        return;

    processingRequest_ = webRenderRequests_.takeFirst();
    
    // Check if client component has been destroyed
    if (!processingRequest_.client)
    {
        processingRequest_.Reset();
        ProcessNextRenderingRequest();
        return;
    }

    if (!webview_)
        CreateWebWidget();

    webview_->setFixedSize(processingRequest_.resolution);
    webview_->load(processingRequest_.url);
}

void SceneWidgetComponents::CreateWebWidget()
{
    if (framework_->IsHeadless())
        return;
    if (webview_)
        return;

    webview_ = new QWebView();
    webview_->setAttribute(Qt::WA_DontShowOnScreen, true);
    webview_->setUpdatesEnabled(false);
    webview_->hide();

    connect(webview_, SIGNAL(loadFinished(bool)), this, SLOT(OnWebViewReady(bool)), Qt::UniqueConnection);

    QNetworkAccessManager *networkAccess = webview_->page()->networkAccessManager();
    if (networkAccess)
    {
        connect(networkAccess, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)), 
            this, SLOT(OnWebViewSslErrors(QNetworkReply*, const QList<QSslError>&)), Qt::UniqueConnection);

#ifdef SCENEWIDGET_BROWSER_SHARED_DATA
        BrowserUiPlugin *browserPlugin = framework_->GetModule<BrowserUiPlugin>();
        if (browserPlugin)
        {
            // Shared disk cache and cookies for all browsers
            if (networkAccess->cache() != browserPlugin->MainDiskCache())
                networkAccess->setCache(browserPlugin->MainDiskCache());
            if (networkAccess->cookieJar() != browserPlugin->MainCookieJar())
                networkAccess->setCookieJar(browserPlugin->MainCookieJar());
        }
#endif
    }
}

void SceneWidgetComponents::ResetWebWidget()
{
    if (webview_)
    {
        LogInfo("SceneWidgetComponents: Resetting render widget");
        webview_->disconnect();
        webview_->stop();
        SAFE_DELETE(webview_)
    }
    if (!webview_.isNull())
        LogError("SceneWidgetComponents: Failed to reset web widget!");
}

void SceneWidgetComponents::OnWebViewReady(bool succesfull)
{
    bool skipRendering = false;
    if (processingRequest_.id == 0)
        skipRendering = true;
    if (!processingRequest_.client)
    {
        LogWarning("WebRenderingRequest: Client EC_WebView is null after page load.");
        skipRendering = true;
    }
    if (!webview_)
    {
        /// @todo handle with error rendering
        LogError("WebRenderingRequest: Webview is null after page load.");
        skipRendering = true;
    }
    // Only report load errors if the intent was to render
    if (!skipRendering && !succesfull) 
    {
        /// @todo handle with error rendering
        LogError("WebRenderingRequest: Failed to load target page: " + processingRequest_.url.toString());
        skipRendering = true;
    }
    if (skipRendering)
    {
        processingRequest_.Reset();
        ProcessNextRenderingRequest();
        return;
    }

    if (buffer_.size() != webview_->size())
        buffer_ = QImage(webview_->size(), QImage::Format_ARGB32_Premultiplied);
    
    /// @todo handle with error rendering
    if (buffer_.width() <= 0 || buffer_.height() <= 0)
    {
        LogError("WebRenderingRequest: Widget height invalid after page load.");
        processingRequest_.Reset();
        ProcessNextRenderingRequest();
        return;
    }

    qDebug() << "Completed rendering:" << processingRequest_.url.toString();

    QPainter painter(&buffer_);
    webview_->render(&painter);
    painter.end();

    processingRequest_.client->Render(buffer_);
    processingRequest_.Reset();
}

void SceneWidgetComponents::OnWebViewSslErrors(QNetworkReply *reply, const QList<QSslError>& errors)
{
    LogError("WebRenderingRequest: SSL errors detected while loading page");

    /// @todo handle with error rendering
    processingRequest_.Reset();
    ProcessNextRenderingRequest();
}

// WebRenderRequest

WebRenderRequest::WebRenderRequest(EC_WebView *c, QUrl u, QSize r) : 
    client(c), url(u), resolution(r), id(0)
{
}

void WebRenderRequest::Reset()
{
    id = 0;
    client = 0;
    url = QUrl();
    resolution = QSize();
}

bool WebRenderRequest::IsIdentical(WebRenderRequest request)
{
    if (client.data() == request.client.data() &&
        url == request.url &&
        resolution == request.resolution)
        return true;
    return false;
}

bool WebRenderRequest::operator==(const WebRenderRequest &other) const
{
    return (id == other.id);
}
