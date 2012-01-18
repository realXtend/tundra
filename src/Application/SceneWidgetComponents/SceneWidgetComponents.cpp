// For conditions of distribution and use, see copyright notice in LICENSE

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
    IModule("SceneWidgetComponents"),
    networkManager_(0)
{
    Reset();
}

SceneWidgetComponents::~SceneWidgetComponents()
{
}

void SceneWidgetComponents::Initialize()
{
    if (framework_->IsHeadless())
        return;

    processingDelay_.setSingleShot(true);
    connect(&processingDelay_, SIGNAL(timeout()), this, SLOT(ProcessNextRenderingRequestImpl()));
    connect(framework_->Input()->TopLevelInputContext(), SIGNAL(MouseEventReceived(MouseEvent*)), SLOT(OnMouseEvent(MouseEvent*)));

    QObject *client = qvariant_cast<QObject*>(framework_->property("client"));
    if (client)
        connect(client, SIGNAL(Disconnected()), this, SLOT(Reset()));
}

void SceneWidgetComponents::Uninitialize()
{
    Reset();

    if (networkManager_)
    {
        // Reset parent of cache and cookie jar.
        if (networkManager_->cache())
            networkManager_->cache()->setParent(0);
        if (networkManager_->cookieJar())
            networkManager_->cookieJar()->setParent(0);
        SAFE_DELETE(networkManager_)
    }
}

QImage SceneWidgetComponents::DrawMessageTexture(QString message, bool error)
{
    QImage img(QSize(500,500), QImage::Format_ARGB32);

    QPainter p(&img);
    p.fillRect(img.rect(), QColor(240,240,240));

    QFont f = p.font();
    f.setPointSize(20);
    p.setFont(f);
    if (error)
        p.setPen(Qt::red);

    p.drawText(img.rect(), Qt::AlignCenter|Qt::TextWordWrap, message);
    p.end();
    return img;
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
    if (request.IsIdentical(currentRequest_))
        return;

    // Check if we have pending request with identical data.
    // Only keep one request per client in the pending list, remove older ones.
    QList<WebRenderRequest> removeList;
    foreach(WebRenderRequest existingRequest, webRenderRequests_)
    {
        if (request.IsIdentical(existingRequest))
            return;
        if (request.client.data() == existingRequest.client.data())
            removeList << existingRequest;
    }
    foreach(WebRenderRequest removeRequest, removeList)
    {
        LogWarning("WebRenderingRequest: Removing same client request with id " + QString::number(removeRequest.id));
        webRenderRequests_.removeAll(removeRequest);
    }

    // Assign ID for the request
    request.id = idGenerator_.AllocateReplicated();
    webRenderRequests_ << request;

    ProcessNextRenderingRequest();
}

void SceneWidgetComponents::ProcessNextRenderingRequest()
{
    if (!processingDelay_.isActive())
        processingDelay_.start(100);
}

void SceneWidgetComponents::ProcessNextRenderingRequestImpl()
{
    // Nothing is pending
    if (webRenderRequests_.isEmpty() && currentRequest_.id == 0)
    {
        idGenerator_.Reset();
        ResetWebWidget();
        return;
    }
    // Already processing a request
    if (currentRequest_.id != 0)
        return;

    currentRequest_ = webRenderRequests_.takeFirst();
    
    // Check if client component has been destroyed
    if (!currentRequest_.client)
    {
        currentRequest_.Reset();
        ProcessNextRenderingRequest();
        return;
    }

    if (!webview_)
        CreateWebWidget();

    webview_->setFixedSize(currentRequest_.resolution);
    webview_->load(currentRequest_.url);
}

void SceneWidgetComponents::CreateWebWidget()
{
    if (framework_->IsHeadless())
        return;
    if (webview_)
        return;

    // This is only created once and only deleted on exit.
    // However we want to delay it here until we have a need for it.
    if (!networkManager_)
    {
        networkManager_ = new QNetworkAccessManager();
        connect(networkManager_, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)), 
                this, SLOT(OnWebViewSslErrors(QNetworkReply*, const QList<QSslError>&)), Qt::UniqueConnection);

#ifdef SCENEWIDGET_BROWSER_SHARED_DATA
        // Shared disk cache and cookies for all browsers
        BrowserUiPlugin *browserPlugin = framework_->GetModule<BrowserUiPlugin>();
        if (browserPlugin)
        {
            networkManager_->setCache(browserPlugin->MainDiskCache());
            if (networkManager_->cache()) networkManager_->cache()->setParent(0);
            networkManager_->setCookieJar(browserPlugin->MainCookieJar());
            if (networkManager_->cookieJar()) networkManager_->cookieJar()->setParent(0);
        }
#endif
    }

    webview_ = new QWebView();
    webview_->page()->setNetworkAccessManager(networkManager_);
    webview_->setAttribute(Qt::WA_DontShowOnScreen, true);
    webview_->setUpdatesEnabled(false);
    webview_->hide();

    connect(webview_, SIGNAL(loadFinished(bool)), this, SLOT(OnWebViewReady(bool)), Qt::UniqueConnection);
}

void SceneWidgetComponents::Reset()
{
    buffer_ = QImage();
    webRenderRequests_.clear();
    idGenerator_.Reset();
    currentRequest_.Reset();
    ResetWebWidget();
}

void SceneWidgetComponents::ResetWebWidget()
{
    if (webview_)
    {
        webview_->disconnect();
        webview_->stop();
        SAFE_DELETE(webview_)
    }
}

void SceneWidgetComponents::OnWebViewReady(bool succesfull)
{
    bool skipRendering = false;
    if (currentRequest_.id == 0)
        skipRendering = true;
    if (!currentRequest_.client)
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
        currentRequest_.client->Render(DrawMessageTexture("Failed to load page\n\n" + currentRequest_.url.toString(), true));
        skipRendering = true;
    }

    if (!skipRendering)
    {
        if (buffer_.size() != webview_->size())
            buffer_ = QImage(webview_->size(), QImage::Format_ARGB32_Premultiplied);
        
        if (buffer_.width() > 0 && buffer_.height() > 0)
        {
            QPainter painter(&buffer_);
            webview_->render(&painter);
            painter.end();

            currentRequest_.client->Render(buffer_);
        }
        else
            currentRequest_.client->Render(DrawMessageTexture("Widget height invalid after page load\n\n" + currentRequest_.url.toString(), true));
    }

    currentRequest_.Reset();
    ProcessNextRenderingRequest();
}

void SceneWidgetComponents::OnWebViewSslErrors(QNetworkReply *reply, const QList<QSslError>& errors)
{
    LogWarning("WebRenderingRequest: Could not load page, ssl errors occurred in url '" + reply->url().toString() + "'");
    if (errors.isEmpty())
        LogWarning("- Unknown SSL error");
    else
    {
        foreach(QSslError err, errors)
            LogWarning("- " + err.errorString());
    }
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
