// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_3DCanvas.h"
#include "EC_3DCanvasSource.h"
#include "EC_OgreMesh.h"
#include "EC_OgreCustomObject.h"
#include "ModuleInterface.h"
#include "ModuleManager.h"
#include "Entity.h"

#include "UiModule.h"
#include "Inworld/View/UiProxyWidget.h"
#include "Inworld/InworldSceneController.h"

#include <Ogre.h>

#include <QWebView>
#include <QLineEdit>
#include <QApplication>
#include <QPushButton>
#include <QUiLoader>

#define LogError(msg) Poco::Logger::get("EC_3DCanvasSource").error(std::string("Error: ") + msg);
#define LogInfo(msg) Poco::Logger::get("EC_3DCanvasSource").information(msg);

EC_3DCanvasSource::EC_3DCanvasSource(Foundation::ModuleInterface *module) :
    Foundation::ComponentInterface(module->GetFramework()),
    source_(this, "source"),
    position_(this, "position", 0),
    submesh_(this, "submesh", 0),
    show2d_(this, "show 2D", false),
    widget_(0),
    content_widget_(0),
    placeholder_widget_(0),
    source_edit_(0)
{
    QObject::connect(this, SIGNAL(OnChanged()), this, SLOT(UpdateWidgetAndCanvas()));
    CreateWidget();
}

EC_3DCanvasSource::~EC_3DCanvasSource()
{
    Scene::Entity* entity = GetParentEntity();
    if (entity)
    {
        Foundation::ComponentInterfacePtr comp = entity->GetComponent(EC_3DCanvas::TypeNameStatic());
        if (comp)
        {
            EC_3DCanvas* canvas = checked_static_cast<EC_3DCanvas*>(comp.get());
            canvas->SetWidget(0);
        }
    }
    
    if (content_widget_)
    {
        content_widget_->deleteLater();
        content_widget_ = 0;
    }
    if (widget_)
    {
        widget_->deleteLater();
        widget_ = 0;
    }
}

void EC_3DCanvasSource::Clicked()
{
    if ((show2d_.Get() == true) && (widget_))
        widget_->show();
}

void EC_3DCanvasSource::SourceEdited()
{
    std::string new_source = source_edit_->text().toStdString();
    if (new_source != source_.Get())
    {
        // Replicate changed source to network
        // std::cout << "Changed source to " << new_source << std::endl;
        source_.Set(new_source, Foundation::Local);
        ComponentChanged(Foundation::Local);
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
    std::string url_str = url.toString().toStdString();
    if (url_str != source_.Get())
    {
        QWebView* webwidget = dynamic_cast<QWebView*>(content_widget_);
        if (!webwidget)
            return;
        webwidget->setUrl(url);
        
        if (source_edit_)
            source_edit_->setText(QString::fromStdString(url_str));
        
        // Set last_source now so that we won't trigger reload of the page again when the source comes back from network
        last_source_ = url_str;
        
        // std::cout << "Changed source by click to " << url_str << std::endl;
        source_.Set(url_str, Foundation::Local);
        ComponentChanged(Foundation::Local);
    }
}

void EC_3DCanvasSource::RepaintCanvas()
{
    Scene::Entity* entity = GetParentEntity();
    if (!entity)
        return;
    
    Foundation::ComponentInterfacePtr comp = entity->GetComponent(EC_3DCanvas::TypeNameStatic());
    if (!comp)
        return;
    
    EC_3DCanvas* canvas = checked_static_cast<EC_3DCanvas*>(comp.get());
    canvas->Start();
}

void EC_3DCanvasSource::UpdateWidget()
{
    std::string source = source_.Get();
    if (source != last_source_)
    {
        if (source_edit_)
            source_edit_->setText(QString::fromStdString(source));
        
        last_source_ = source;
        if (!placeholder_widget_)
        {
            LogError("No placeholder widget, cannot create content widget");
            return;
        }
        
        // See if source looks like an url, and instantiate a QWebView then if it doesn't already exist
        if (source.find("http://") != std::string::npos)
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
                webwidget->setUrl(QUrl(QString::fromStdString(source)));
                webwidget->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
                
                content_widget_ = webwidget;
                content_widget_->show();
                
                QObject::connect(webwidget, SIGNAL(loadFinished( bool )), this, SLOT(RepaintCanvas()));
                QObject::connect(webwidget, SIGNAL(loadProgress( int )), this, SLOT(RepaintCanvas()));
                QObject::connect(webwidget, SIGNAL(linkClicked( const QUrl & )), this, SLOT(WebViewLinkClicked( const QUrl & )));
                QObject::connect(webwidget->page(), SIGNAL(scrollRequested( int, int, const QRect & )), this, SLOT(RepaintCanvas()));
            }
            else
            {
                // If source changed, update the webview URL
                webwidget->setUrl(QUrl(QString::fromStdString(source)));
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
    
    Foundation::ComponentInterfacePtr comp = entity->GetOrCreateComponent(EC_3DCanvas::TypeNameStatic());
    if (!comp)
    {
        LogError("Could not create/get 3DCanvas component");
        return;
    }
    
    // If entity has no valid mesh or prim yet, start a retry timer and try to set the canvas later
    if ((!entity->GetComponent(OgreRenderer::EC_OgreMesh::TypeNameStatic())) && 
        (!entity->GetComponent(OgreRenderer::EC_OgreCustomObject::TypeNameStatic())))
    {
        LogInfo("Mesh or prim did not exist yet, retrying");
        QTimer::singleShot(1000, this, SLOT(UpdateCanvas()));
        return;
    }
    
    EC_3DCanvas* canvas = checked_static_cast<EC_3DCanvas*>(comp.get());
    canvas->SetWidget(content_widget_);
    int submesh = submesh_.Get();
    if (submesh < 0)
        submesh = 0;
    canvas->SetSubmesh(submesh);
    canvas->Start();
}

void EC_3DCanvasSource::ChangeLanguage()
{
    if (widget_)
    {
        QString translation = QApplication::translate("3DCanvasSource", "3DCanvas Controls");
        widget_->graphicsProxyWidget()->setWindowTitle(translation);
    }
}

void EC_3DCanvasSource::CreateWidget()
{
    boost::shared_ptr<UiServices::UiModule> ui_module = 
        framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
    if (!ui_module.get())
    {
        LogError("Failed to acquire UiModule pointer");
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
    
    UiServices::UiProxyWidget *proxy = ui_module->GetInworldSceneController()->AddWidgetToScene(widget_,
        UiServices::UiWidgetProperties(widget_->windowTitle(), UiServices::SceneWidget));
    
    QObject::connect(qApp, SIGNAL(LanguageChanged()), this, SLOT(ChangeLanguage()));
    
    source_edit_ = widget_->findChild<QLineEdit*>("line_source");
    if (source_edit_)
    {
        QObject::connect(source_edit_, SIGNAL( editingFinished() ), this, SLOT( SourceEdited() ));
    }
    
    placeholder_widget_ = widget_->findChild<QWidget*>("widget_placeholder");
    
    QPushButton* button = widget_->findChild<QPushButton*>("but_start");
    if (button) QObject::connect(button, SIGNAL( clicked() ), this, SLOT( StartPressed() ));
    button = widget_->findChild<QPushButton*>("but_prev");
    if (button) QObject::connect(button, SIGNAL( clicked() ), this, SLOT( PrevPressed() ));
    button = widget_->findChild<QPushButton*>("but_next");
    if (button) QObject::connect(button, SIGNAL( clicked() ), this, SLOT( NextPressed() ));
    button = widget_->findChild<QPushButton*>("but_end");
    if (button) QObject::connect(button, SIGNAL( clicked() ), this, SLOT( EndPressed() ));
}

