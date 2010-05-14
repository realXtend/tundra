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

#define LogError(msg) Poco::Logger::get("EC_3DCanvasSource").error(std::string("Error: ") + msg);
#define LogInfo(msg) Poco::Logger::get("EC_3DCanvasSource").information(msg);

EC_3DCanvasSource::EC_3DCanvasSource(Foundation::ModuleInterface *module) :
    Foundation::ComponentInterface(module->GetFramework()),
    source_(this, "source"),
    position_(this, "position", 0),
    submesh_(this, "submesh", 0),
    show2d_(this, "show 2D", false),
    widget_(0)
{
    QObject::connect(this, SIGNAL(OnChanged()), this, SLOT(UpdateWidgetAndCanvas()));
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

void EC_3DCanvasSource::UpdateWidgetAndCanvas()
{
    UpdateWidget();
    UpdateCanvas();
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
    // See if source looks like an url, and instantiate a QWebView then if it doesn't already exist
    std::string source = source_.Get();
    if (source != last_source_)
    {
        if (source.find("http://") != std::string::npos)
        {
            //CreateMainWidget();
            
            QWebView* webwidget = dynamic_cast<QWebView*>(widget_);
            if (!webwidget)
            {
                // If widget exists, but is wrong type, delete and recreate
                if (widget_)
                {
                    widget_->deleteLater();
                    widget_ = 0;
                }
                
                webwidget = new QWebView();
                webwidget->setUrl(QUrl(QString::fromStdString(source)));
                
                boost::shared_ptr<UiServices::UiModule> ui_module = 
                    framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
                if (!ui_module.get())
                {
                    LogError("Failed to acquire UiModule pointer");
                    return;
                }
                
                // Prepare proxywidget for showing in 2D
                widget_ = webwidget;
                UiServices::UiProxyWidget *proxy = ui_module->GetInworldSceneController()->AddWidgetToScene(widget_,
                    UiServices::UiWidgetProperties(widget_->windowTitle(), UiServices::SceneWidget));
                
                QObject::connect(webwidget, SIGNAL(loadFinished( bool )), this, SLOT(RepaintCanvas()));
                QObject::connect(webwidget, SIGNAL(loadProgress( int )), this, SLOT(RepaintCanvas()));
                QObject::connect(webwidget, SIGNAL(linkClicked( const QUrl & )), this, SLOT(RepaintCanvas()));
                QObject::connect(webwidget->page(), SIGNAL(scrollRequested( int, int, const QRect & )), this, SLOT(RepaintCanvas()));
            }
            else
            {
                // If source changed, update the webview URL
                webwidget->setUrl(QUrl(QString::fromStdString(source)));
            }
        }
        
        last_source_ = source;
    }
}

void EC_3DCanvasSource::UpdateCanvas()
{
    if (!widget_)
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
    canvas->SetWidget(widget_);
    int submesh = submesh_.Get();
    if (submesh < 0)
        submesh = 0;
    canvas->SetSubmesh(submesh);
    canvas->Start();
}
