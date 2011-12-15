// For conditions of distribution and use, see copyright notice in license.txt

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
#include <QImage>

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

private slots:
    void OnMouseEvent(MouseEvent *mEvent);

    void ProcessNextRenderingRequest();
    void CreateWebWidget();
    void ResetWebWidget();
    void OnWebViewReady(bool succesfull);
    void OnWebViewSslErrors(QNetworkReply *reply, const QList<QSslError>& errors);

private:
    QPointer<QWebView> webview_;
    QList<WebRenderRequest> webRenderRequests_;
    WebRenderRequest processingRequest_;
    UniqueIdGenerator idGenerator_;
    QImage buffer_;
};
