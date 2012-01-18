// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "SceneWidgetComponentsApi.h"

#include "FrameworkFwd.h"
#include "InputFwd.h"
#include "SceneFwd.h"

#include "IModule.h"
#include "UniqueIdGenerator.h"
#include "EC_WebView.h"

#include <QUrl>
#include <QSize>
#include <QPointer>
#include <QWebView>
#include <QSslError>
#include <QTimer>
#include <QImage>

class QNetworkAccessManager;
class QNetworkReply;

struct WebRenderRequest
{
    WebRenderRequest(EC_WebView *c = 0, QUrl u = QUrl(), QSize r = QSize());
    bool operator==(const WebRenderRequest &other) const;
    bool IsIdentical(WebRenderRequest request);
    void Reset();

    QPointer<EC_WebView> client;
    QUrl url;
    QSize resolution;
    unsigned int id;
};

class SCENEWIDGET_MODULE_API SceneWidgetComponents : public IModule
{

Q_OBJECT

public:
    /// Constructor
    SceneWidgetComponents();

    /// Deconstructor. 
    virtual ~SceneWidgetComponents();

    /// IModule override.
    virtual void Initialize();

    /// IModule override.
    virtual void Uninitialize();

public slots:
    void WebRenderingRequest(EC_WebView *client, QUrl url, QSize resolution);
    
    QImage DrawMessageTexture(QString message, bool error = false);

private slots:
    void OnMouseEvent(MouseEvent *mEvent);

    void ProcessNextRenderingRequest();
    void ProcessNextRenderingRequestImpl();

    void CreateWebWidget();
    void ResetWebWidget();
    void Reset();

    void OnWebViewReady(bool succesfull);
    void OnWebViewSslErrors(QNetworkReply *reply, const QList<QSslError>& errors);    

private:
    QNetworkAccessManager *networkManager_;
    QPointer<QWebView> webview_;

    QList<WebRenderRequest> webRenderRequests_;
    WebRenderRequest currentRequest_;
    UniqueIdGenerator idGenerator_;
    QImage buffer_;
    QTimer processingDelay_;
};
