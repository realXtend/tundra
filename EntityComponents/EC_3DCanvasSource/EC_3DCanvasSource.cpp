// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EC_3DCanvasSource.h"

#include "EC_3DCanvas.h"
#include "EC_Mesh.h"
#include "EC_OgreCustomObject.h"
#include "IModule.h"
#include "ModuleManager.h"
#include "Entity.h"
#include "UiProxyWidget.h"
#include "UiServiceInterface.h"
#include "LoggingFunctions.h"

DEFINE_POCO_LOGGING_FUNCTIONS("EC_3DCanvasSource")

#include <QWebView>
#include <QLineEdit>
#include <QApplication>
#include <QPushButton>
#include <QUiLoader>
#include <QVBoxLayout>
#include <QGraphicsScene>
#include <QPushButton>
#include <QProgressBar>
#include <QSize>

#include "MemoryLeakCheck.h"

EC_3DCanvasSource::EC_3DCanvasSource(IModule *module) :
    IComponent(module->GetFramework()),
    source(this, "Source", ""),
    submesh(this, "Submesh", 0),
    refreshRate(this, "Refresh per sec", 0),
    show2d(this, "Show 2D on click", true),
    sync2dbrowsing(this, "Sync 2D browsing", false),
    pageWidth(this, "Page width", 800),
    pageHeight(this, "Page height", 600),
    widget_(0),
    content_widget_(0),
    placeholder_widget_(0),
    button_refreshstop_(0),
    progress_bar_(0),
    proxy_(0),
    source_edit_(0),
    canvas_started_(false)
{
    static AttributeMetadata size_metadata("", "100", "2000", "50");
    pageWidth.SetMetadata(&size_metadata);
    pageHeight.SetMetadata(&size_metadata);

    connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)), this, SLOT(UpdateWidgetAndCanvas(IAttribute*, AttributeChange::Type)));
    connect(this, SIGNAL(ParentEntitySet()), this, SLOT(RegisterActions()));
    CreateWidget();
}

EC_3DCanvasSource::~EC_3DCanvasSource()
{
    // Note: content widget is inside widget_s layout,
    // no need to do a separate deleteLater for it. This would cause problems on rundown.
    SAFE_DELETE_LATER(widget_);
}

void EC_3DCanvasSource::OnClick()
{
    if ((getshow2d() == true) && (widget_) && (proxy_))
    {
        if (!proxy_->scene())
            return;
        if (!proxy_->scene()->isActive())
            return;
        if (proxy_->isVisible())
            proxy_->AnimatedHide();
        else
            proxy_->show();
    }
}

void EC_3DCanvasSource::SourceEdited()
{
    if (!source_edit_)
        return;
    
    QString new_source = source_edit_->text();
    if (new_source != getsource())
    {
        if (getsync2dbrowsing())
            setsource(new_source);
        else
            UpdateWidget(new_source);
    }
}

void EC_3DCanvasSource::RefreshStopPressed()
{
    QWebView *webview = GetWebView();
    if (!webview || !progress_bar_)
        return;
    if (progress_bar_->isVisible())
        webview->stop();
    else
        webview->reload();
}

void EC_3DCanvasSource::BackPressed()
{
    QWebView *webview = GetWebView();
    if (!webview)
        return;
    webview->back();
}

void EC_3DCanvasSource::ForwardPressed()
{
    QWebView *webview = GetWebView();
    if (!webview)
        return;
    webview->forward();
}

void EC_3DCanvasSource::HomePressed()
{
    QWebView *webview = GetWebView();
    if (!webview || home_url_.isEmpty())
        return;
    webview->load(QUrl(home_url_));
}

void EC_3DCanvasSource::UpdateWidgetAndCanvas(IAttribute *attribute, AttributeChange::Type type)
{
    EC_3DCanvas *canvas = Get3DCanvas();
    bool update = false;
    if (attribute == &submesh)
    {
        if (canvas)
        {
            int my_submesh = getsubmesh();
            if (my_submesh < 0)
                my_submesh = 0;
            if (!canvas->GetSubMeshes().contains(my_submesh))
            {
                canvas->SetSubmesh(my_submesh);
                update = true;
            }
        }
        else
            UpdateWidget();
    }
    else if (attribute == &source)
    {
        if (last_source_ != getsource())
        {
            UpdateWidget();
            if (!canvas_started_)
                UpdateCanvas();
            else
                update = true;

            if (home_url_.isEmpty())
                home_url_ = getsource();
        }
    }
    else if (attribute == &refreshRate)
    {
        if (canvas)
        {
            int ref_rate_sec = getrefreshRate();
            if (ref_rate_sec > 0)
            {
                int ref_rate_msec = 1000 / ref_rate_sec;
                if (canvas->GetRefreshRate() != ref_rate_msec)
                {
                    canvas->SetRefreshRate(ref_rate_sec);
                    canvas_started_ = false;
                    UpdateCanvas();
                }
            }
            else
                canvas->SetRefreshRate(0);
        }
    }
    else if (attribute == &pageHeight || attribute == &pageWidth)
    {
        QWebView *webview = GetWebView();
        if (webview)
        {
            QSize new_size(getpageWidth(), getpageHeight());
            if (webview->size() != new_size)
            {
                webview->resize(new_size);
                update = true;
            }
        }
    }
    
    if (update && canvas)
        UpdateCanvas();
}

void EC_3DCanvasSource::WebViewLinkClicked(const QUrl& url)
{
    //! \todo: check here if the link is something we want to open differently (external program etc.)
    
    // If url is different than the source, update the lineedit & browser & replicate to network
    QString url_str = url.toString();
    if (url_str != getsource())
    {
        QWebView* webwidget = dynamic_cast<QWebView*>(content_widget_);
        if (!webwidget)
            return;
        webwidget->setUrl(url);
        
        if (source_edit_)
            source_edit_->setText(url_str);
        
        if (getsync2dbrowsing())
        {
            // Set last_source now so that we won't trigger reload of the page again when the source comes back from network
            last_source_ = url_str;
            setsource(url_str);
        }
    }
}

void EC_3DCanvasSource::WebViewLoadStarted()
{
    if (progress_bar_)
        progress_bar_->show();
    if (button_refreshstop_)
        button_refreshstop_->setStyleSheet("QPushButton#button_refreshstop { background-image: url('./data/ui/images/browser/stop.png'); }");
}

void EC_3DCanvasSource::WebViewLoadProgress(int progress)
{
    if (progress_bar_)
        progress_bar_->setValue(progress);
}

void EC_3DCanvasSource::WebViewLoadCompleted()
{
    // Setup ui
    if (progress_bar_)
        progress_bar_->hide();
    if (button_refreshstop_)
        button_refreshstop_->setStyleSheet("QPushButton#button_refreshstop { background-image: url('./data/ui/images/browser/refresh.png'); }");
    
    // Update the 2d ui line edit
    QWebView *webview = GetWebView();
    if (webview && source_edit_)
        source_edit_->setText(webview->url().toString());

    // Invoke a delayed repaint of the inworld texture
    QTimer::singleShot(50, this, SLOT(RepaintCanvas()));
}

void EC_3DCanvasSource::RepaintCanvas()
{
    EC_3DCanvas *canvas = Get3DCanvas();
    if (canvas)
        canvas->Update();
}

EC_3DCanvas *EC_3DCanvasSource::Get3DCanvas()
{
    Scene::Entity* entity = GetParentEntity();
    if (!entity)
        return 0;
    ComponentPtr comp = entity->GetComponent(EC_3DCanvas::TypeNameStatic());
    if (!comp)
        return 0;
    EC_3DCanvas* canvas = checked_static_cast<EC_3DCanvas*>(comp.get());
    return canvas;
}

QWebView *EC_3DCanvasSource::GetWebView()
{
    if (!content_widget_)
        return 0;
    return dynamic_cast<QWebView*>(content_widget_);
}

void EC_3DCanvasSource::UpdateWidget(QString url)
{
    // Don't load QWebViews into memory if headless
    if (GetFramework()->IsHeadless())
        return;

    // Prefer inparam over the attribute (if sync 2d browsing is disabled)
    QString source;
    if (url.isEmpty())
        source = getsource();
    else
        source = url;

    if (source.isEmpty())
    {
        QTimer::singleShot(1000, this, SLOT(FetchWebViewUrl()));
        return;
    }

    if (source != last_source_)
    {
        if (source_edit_)
            source_edit_->setText(source);
        last_source_ = source;

        if (!placeholder_widget_)
        {
            LogError("No placeholder widget, cannot create content widget");
            return;
        }
        
        // See if source looks like an url, and instantiate a QWebView then if it doesn't already exist
        if (source.indexOf("http://") != -1)
        {
            QWebView* webwidget = GetWebView();
            if (!webwidget)
            {
                // If widget exists, but is wrong type, delete and recreate
                if (content_widget_)
                {
                    content_widget_->deleteLater();
                    content_widget_ = 0;
                }
                
                webwidget = new QWebView(placeholder_widget_);
                QVBoxLayout *layout = placeholder_widget_->findChild<QVBoxLayout*>("widget_placeholder_layout");
                if (layout)
                    layout->addWidget(webwidget);

                // Load current source and resize to attribute set size
                webwidget->load(QUrl(source));
                webwidget->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
                webwidget->resize(getpageWidth(), getpageHeight());
                
                // Connect webview signals
                connect(webwidget, SIGNAL(loadStarted()), this, SLOT(WebViewLoadStarted()), Qt::UniqueConnection);
                connect(webwidget, SIGNAL(loadProgress(int)), this, SLOT(WebViewLoadProgress(int)), Qt::UniqueConnection);
                connect(webwidget, SIGNAL(loadFinished(bool)), this, SLOT(WebViewLoadCompleted()), Qt::UniqueConnection);
                connect(webwidget, SIGNAL(linkClicked(const QUrl&)), this, SLOT(WebViewLinkClicked(const QUrl &)), Qt::UniqueConnection);
                connect(webwidget->page(), SIGNAL(scrollRequested(int, int, const QRect&)), this, SLOT(RepaintCanvas()), Qt::UniqueConnection);

                // This webview is our new content_widget_
                content_widget_ = webwidget;
            }
            else
            {
                // If source changed, update the webview URL
                webwidget->load(QUrl(source));
            }
        }
    }
}

void EC_3DCanvasSource::UpdateCanvas()
{
    if (!content_widget_)
        return;
    
    Scene::Entity* entity = GetParentEntity();
    if (!entity)
    {
        LogError("No parent entity, cannot create/update 3DCanvas");
        return;
    }
    
    ComponentPtr comp = entity->GetOrCreateComponent(EC_3DCanvas::TypeNameStatic());
    if (!comp)
    {
        LogError("Could not create/get 3DCanvas component");
        return;
    }
    
    // If entity has no valid mesh or prim yet, start a retry timer and try to set the canvas later
    if ((!entity->GetComponent(EC_Mesh::TypeNameStatic())) && 
        (!entity->GetComponent(EC_OgreCustomObject::TypeNameStatic())))
    {
        //LogInfo("Mesh or prim did not exist yet, retrying");
        QTimer::singleShot(1000, this, SLOT(UpdateCanvas()));
        return;
    }
    
    // Set widget if different
    EC_3DCanvas* canvas = checked_static_cast<EC_3DCanvas*>(comp.get());
    if (canvas->GetWidget() != content_widget_)
    {
        canvas->SetWidget(content_widget_);
    }

    // Set submesh if different
    int my_submesh = getsubmesh();
    if (my_submesh < 0)
        my_submesh = 0;
    if (!canvas->GetSubMeshes().contains(my_submesh))
        canvas->SetSubmesh(my_submesh);

    // Refresh rate
    int ref_rate_sec = getrefreshRate();
    if (ref_rate_sec > 0)
    {
        int ref_rate_msec = 1000 / ref_rate_sec;
        if (canvas->GetRefreshRate() != ref_rate_msec)
        {
            canvas->SetRefreshRate(ref_rate_sec);
            canvas_started_ = false;
        }
    }
    else
        canvas->SetRefreshRate(0);

    // Start if first run
    if (!canvas_started_)
    {
        canvas->Start();
        canvas_started_ = true;
    }
    // Update otherwise
    else
    {
        canvas->Update();
    }
}

void EC_3DCanvasSource::FetchWebViewUrl()
{   
    // Workaround on the bug that ec_3canvas->webview()->url() is not valid when this component is created/updated.
    // Somehow its doesent have time to set it before we already come here. So this polls untill the url is valid.

    Scene::Entity* entity = GetParentEntity();
    if (!entity)
    {
        LogError("No parent entity, cannot create/update 3DCanvas");
        return;
    }
    
    ComponentPtr comp = entity->GetOrCreateComponent(EC_3DCanvas::TypeNameStatic());
    if (!comp)
    {
        LogError("Could not create/get 3DCanvas component");
        return;
    }

    EC_3DCanvas* canvas = checked_static_cast<EC_3DCanvas*>(comp.get());
    QWidget *canvas_widget = canvas->GetWidget();
    if (!canvas_widget)
        return;
    QWebView *canvas_webview = dynamic_cast<QWebView*>(canvas_widget);
    if (!canvas_webview)
        return;
    QString url = canvas_webview->url().toString();
    if (!url.isEmpty())
    {
        setsource(url);
        if (home_url_.isEmpty())
            home_url_ = url;
    }
    else
        QTimer::singleShot(1000, this, SLOT(FetchWebViewUrl()));
}

void EC_3DCanvasSource::ChangeLanguage()
{
    if (widget_)
    {
        QString title = tr("3DCanvas Controls");
        widget_->setWindowTitle(title);
    }
}

void EC_3DCanvasSource::CreateWidget()
{
    if (GetFramework()->IsHeadless())
        return;

    UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();
    if (!ui)
    {
        LogError("Failed to acquire UI service");
        return;
    }

    QUiLoader loader;
    loader.setLanguageChangeEnabled(true);
    QFile file("./data/ui/3dcanvassource.ui");
    file.open(QFile::ReadOnly);
    widget_ = loader.load(&file);
    file.close();
    
    if (!widget_)
    {
        LogError("Failed to create 3D canvas controls widget");
        return;
    }

    widget_->setWindowTitle(tr("Naali Web Browser"));
    proxy_ = ui->AddWidgetToScene(widget_);
    connect(qApp, SIGNAL(LanguageChanged()), this, SLOT(ChangeLanguage()));

    source_edit_ = widget_->findChild<QLineEdit*>("line_source");
    if (source_edit_)
        connect(source_edit_, SIGNAL(returnPressed()), SLOT(SourceEdited()));

    placeholder_widget_ = widget_->findChild<QWidget*>("widget_placeholder");

    button_refreshstop_ = widget_->findChild<QPushButton*>("button_refreshstop");
    if (button_refreshstop_) 
        connect(button_refreshstop_, SIGNAL(clicked()), SLOT(RefreshStopPressed()));

    QPushButton* button = widget_->findChild<QPushButton*>("button_back");
    if (button) 
        connect(button, SIGNAL(clicked()), SLOT(BackPressed()));

    button = widget_->findChild<QPushButton*>("button_forward");
    if (button) 
        connect(button, SIGNAL(clicked()), SLOT(ForwardPressed()));

    button = widget_->findChild<QPushButton*>("button_home");
    if (button) 
        connect(button, SIGNAL(clicked()), SLOT(HomePressed()));

    progress_bar_ = widget_->findChild<QProgressBar*>("progress_bar");
    if (progress_bar_)
        progress_bar_->hide();
}

void EC_3DCanvasSource::RegisterActions()
{
    Scene::Entity *entity = GetParentEntity();
    assert(entity);
    if (entity)
        entity->ConnectAction("MousePress", this, SLOT(OnClick()));
}

