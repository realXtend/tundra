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

#include "MemoryLeakCheck.h"

EC_3DCanvasSource::EC_3DCanvasSource(IModule *module) :
    IComponent(module->GetFramework()),
    source(this, "source"),
    position(this, "position", 0),
    submesh(this, "submesh", 0),
    show2d(this, "show 2D", true),
    widget_(0),
    content_widget_(0),
    placeholder_widget_(0),
    proxy_(0),
    source_edit_(0),
    manipulate_ec_3dcanvas(true)
{
    connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)), this, SLOT(UpdateWidgetAndCanvas()));
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
        if (proxy_->isVisible())
            proxy_->AnimatedHide();
        else
            proxy_->show();
    }
}

void EC_3DCanvasSource::SourceEdited()
{
    QString new_source = source_edit_->text();
    if (new_source != getsource())
    {
        // Replicate changed source to network
        // std::cout << "Changed source to " << new_source << std::endl;
        setsource(new_source);
        ComponentChanged(AttributeChange::Default);
    }
}

void EC_3DCanvasSource::StartPressed()
{
}

void EC_3DCanvasSource::PrevPressed()
{
}

void EC_3DCanvasSource::NextPressed()
{
}

void EC_3DCanvasSource::EndPressed()
{
}

void EC_3DCanvasSource::UpdateWidgetAndCanvas()
{
    UpdateWidget();
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
        
        // Set last_source now so that we won't trigger reload of the page again when the source comes back from network
        last_source_ = url_str;
        
        // std::cout << "Changed source by click to " << url_str << std::endl;
        setsource(url_str);
        ComponentChanged(AttributeChange::Default);
    }
}

void EC_3DCanvasSource::RepaintCanvas()
{
    if (!manipulate_ec_3dcanvas)
        return;

    Scene::Entity* entity = GetParentEntity();
    if (!entity)
        return;

    ComponentPtr comp = entity->GetComponent(EC_3DCanvas::TypeNameStatic());
    if (!comp)
        return;

    EC_3DCanvas* canvas = checked_static_cast<EC_3DCanvas*>(comp.get());
    canvas->Start();
}

void EC_3DCanvasSource::UpdateWidget()
{
    QString source = getsource();
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
            QWebView* webwidget = dynamic_cast<QWebView*>(content_widget_);
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

                webwidget->setUrl(QUrl(source));
                webwidget->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
                content_widget_ = webwidget;
                
                connect(webwidget, SIGNAL(loadFinished( bool )), this, SLOT(RepaintCanvas()));
                connect(webwidget, SIGNAL(loadProgress( int )), this, SLOT(RepaintCanvas()));
                connect(webwidget, SIGNAL(linkClicked( const QUrl & )), this, SLOT(WebViewLinkClicked( const QUrl & )));
                connect(webwidget->page(), SIGNAL(scrollRequested( int, int, const QRect & )), this, SLOT(RepaintCanvas()));
            }
            else
            {
                // If source changed, update the webview URL
                webwidget->setUrl(QUrl(source));
            }
        }
    }
}

void EC_3DCanvasSource::UpdateCanvas()
{
    if (!content_widget_ || !manipulate_ec_3dcanvas)
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
    
    EC_3DCanvas* canvas = checked_static_cast<EC_3DCanvas*>(comp.get());
    canvas->SetWidget(content_widget_);
    int submesh = getsubmesh();
    if (submesh < 0)
        submesh = 0;
    canvas->SetSubmesh(submesh);
    canvas->Start();
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
        ComponentChanged(AttributeChange::LocalOnly);
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

    widget_->setWindowTitle(tr("3DCanvas Controls"));
    proxy_ = ui->AddWidgetToScene(widget_);

    connect(qApp, SIGNAL(LanguageChanged()), this, SLOT(ChangeLanguage()));

    source_edit_ = widget_->findChild<QLineEdit*>("line_source");
    if (source_edit_)
    {
        connect(source_edit_, SIGNAL( editingFinished() ), this, SLOT( SourceEdited() ));
    }
    
    placeholder_widget_ = widget_->findChild<QWidget*>("widget_placeholder");
    
    QPushButton* button = widget_->findChild<QPushButton*>("but_start");
    if (button) connect(button, SIGNAL( clicked() ), this, SLOT( StartPressed() ));
    button = widget_->findChild<QPushButton*>("but_prev");
    if (button) connect(button, SIGNAL( clicked() ), this, SLOT( PrevPressed() ));
    button = widget_->findChild<QPushButton*>("but_next");
    if (button) connect(button, SIGNAL( clicked() ), this, SLOT( NextPressed() ));
    button = widget_->findChild<QPushButton*>("but_end");
    if (button) connect(button, SIGNAL( clicked() ), this, SLOT( EndPressed() ));
}

void EC_3DCanvasSource::RegisterActions()
{
    Scene::Entity *entity = GetParentEntity();
    assert(entity);
    if (entity)
        entity->ConnectAction("MousePress", this, SLOT(OnClick()));
}

