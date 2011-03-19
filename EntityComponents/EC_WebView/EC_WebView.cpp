// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EC_WebView.h"
#include "IModule.h"
#include "Entity.h"

#include "EC_3DCanvas.h"
#include "EC_Mesh.h"

#include <QWebView>
#include <QDebug>

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("EC_WebView")

#include "MemoryLeakCheck.h"

EC_WebView::EC_WebView(IModule *module) :
    IComponent(module->GetFramework()),
    webview_(0),
    renderTimer_(0),
    webviewLoading_(false),
    webviewHasContent_(false),
    componentPrepared_(false),
    webviewUrl(this, "View URL", QString()),
    webviewSize(this, "View Size", QSize(800,600)),
    renderSubmeshIndex(this, "Render Submesh", 0),
    renderRefreshRate(this, "Render FPS", 0)
{
    static AttributeMetadata submeshMetaData;
    static AttributeMetadata refreshRateMetadata;
    static bool metadataInitialized = false;
    if (!metadataInitialized)
    {
        submeshMetaData.minimum = "0";
        submeshMetaData.step = "1";
        refreshRateMetadata.minimum = "0";
        refreshRateMetadata.maximum = "25";
        refreshRateMetadata.step = "1";
        metadataInitialized = true;
    }
    renderSubmeshIndex.SetMetadata(&submeshMetaData);
    renderRefreshRate.SetMetadata(&refreshRateMetadata);

    // Don't do anything if rendering is not enabled
    if (!ViewEnabled() || GetFramework()->IsHeadless())
        return;

    connect(this, SIGNAL(ParentEntitySet()), SLOT(PrepareComponent()), Qt::UniqueConnection);
    connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(AttributeChanged(IAttribute*, AttributeChange::Type)), Qt::UniqueConnection);
    
    renderTimer_ = new QTimer(this);
    connect(renderTimer_, SIGNAL(timeout()), SLOT(Render()), Qt::UniqueConnection);

    PrepareWebview();
}

EC_WebView::~EC_WebView()
{
    ResetWidget();
}

bool EC_WebView::IsSerializable() const
{
    return true;
}

// Public slots

void EC_WebView::Render()
{
    // Don't do anything if rendering is not enabled
    if (!ViewEnabled() || GetFramework()->IsHeadless())
        return;

    // Comprehensive checks that everything is ok before rendering anything.
    if (!webview_)
    {
        LogError("Render: Webview object is null, aborting render call.");
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
    EC_3DCanvas *sceneCanvas = GetSceneCanvasComponent();
    if (!mesh || !sceneCanvas)
    {
        // In the case someone destroyed EC_3DCanvas or EC_Mesh from our entity
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
    
    // Set submesh to EC_3DCanvas if different from current
    if (!sceneCanvas->GetSubMeshes().contains(submeshIndex))
        sceneCanvas->SetSubmesh(submeshIndex);
    
    // Set widget to EC_3DCanvas if different from current
    if (sceneCanvas->GetWidget() != webview_)
        sceneCanvas->SetWidget(webview_);

    sceneCanvas->Update();
}

// Private slots

void EC_WebView::RenderDelayed()
{
    if (!componentPrepared_)
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

void EC_WebView::ResetWidget()
{
    RenderTimerStop();

    if (webview_)
    {
        // Reset the EC_3DCanvas data for added safety. Restore original materials.
        // If we come here from dtor this wont happen as parent entity has been reseted.
        EC_3DCanvas *sceneCanvas = GetSceneCanvasComponent();
        if (sceneCanvas)
        {
            if (sceneCanvas->GetWidget() == webview_)
            {
                sceneCanvas->RestoreOriginalMeshMaterials();
                sceneCanvas->SetWidget(0);
            }
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
        webview_->deleteLater();
        webview_ = 0;
    }

    webviewHasContent_ = false;
}

void EC_WebView::PrepareComponent()
{
    // Don't do anything if rendering is not enabled
    if (!ViewEnabled() || GetFramework()->IsHeadless())
        return;

    // Some security checks
    if (componentPrepared_)
    {
        LogWarning("PrepareComponent: Preparations seem to be done already, you might not want to do this multiple times.");
    }
    if (!webview_)
    {
        LogError("PrepareComponent: Cannot start preparing, webview object is null. This should never happen!");
        return;
    }

    // Get parent and connect to the component removed signal.
    Scene::Entity *parent = GetParentEntity();
    if (parent)
    {
        connect(parent, SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)), SLOT(ComponentRemoved(IComponent*, AttributeChange::Type)), Qt::UniqueConnection);
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
            connect(mesh, SIGNAL(OnMeshChanged()), SLOT(PrepareComponent()), Qt::UniqueConnection);
            return;
        }
    }

    // Get or create local EC_3DCanvas component
    IComponent *iComponent = parent->GetOrCreateComponentRaw(EC_3DCanvas::TypeNameStatic(), AttributeChange::LocalOnly, false);
    EC_3DCanvas *sceneCanvas = dynamic_cast<EC_3DCanvas*>(iComponent);
    if (!sceneCanvas)
    {
        LogError("PrepareComponent: Could not get or create EC_3DCanvas component!");
        return;
    }
    
    // All the needed components are present, mark prepared as true.
    componentPrepared_ = true;
    
    // Resize the widget if needed before loading the page.
    QSize targetSize = getwebviewSize();
    if (webview_->size() != targetSize)
        webview_->resize(targetSize);

    // Validate and load the 'getwebviewUrl' page to our QWebView if it's not empty.
    QString urlString = getwebviewUrl().simplified();
    if (urlString.isEmpty())
    {
        // Stop timers if running
        RenderTimerStop();
        return;
    }

    if (!urlString.startsWith("http://") && !urlString.startsWith("https://"))
        urlString = "http://" + urlString;

    //! \note loading the url will invoke Render() once QWebView signals the page has been loaded.
    webview_->load(QUrl(urlString, QUrl::TolerantMode));
}

void EC_WebView::PrepareWebview()
{
    // Don't do anything if rendering is not enabled
    if (!ViewEnabled() || GetFramework()->IsHeadless())
        return;

    ResetWidget();

    webview_ = new QWebView();
    webview_->resize(getwebviewSize());

    connect(webview_, SIGNAL(loadStarted()), this, SLOT(LoadStarted()), Qt::UniqueConnection);
    connect(webview_, SIGNAL(loadFinished(bool)), this, SLOT(LoadFinished(bool)), Qt::UniqueConnection);
}

void EC_WebView::LoadStarted()
{
    RenderTimerStop();
    webviewLoading_ = true;
}

void EC_WebView::LoadFinished(bool success)
{
    webviewLoading_ = false;
    if (success)
    {
        webviewHasContent_ = true;
        RenderTimerStartOrSingleShot();
    }
    else
        LogWarning("Please check your URL, failed to load " + webview_->url().toString().toStdString());
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
        if (renderTimer_->isActive())
            LogWarning("RenderTimerStartOrSingleShot: Did you forget to stop timer before you called me?");

        if (getrenderRefreshRate() > 0)
        {
            // Clamp FPS to 0-25, the EC editor UI does this for us with AttributeMetaData,
            // but someone can inject crazy stuff here directly from code
            int renderFPS = 1000 / getrenderRefreshRate();
            if (renderFPS < 40)
                renderFPS = 40;
            if (renderFPS <= 0)
                return;
            renderTimer_->start(renderFPS);
        }
        else
            QTimer::singleShot(10, this, SLOT(Render()));
    }
    else
        QTimer::singleShot(10, this, SLOT(Render()));
}

void EC_WebView::ComponentAdded(IComponent *component, AttributeChange::Type change)
{
    if (component->TypeName() == EC_Mesh::TypeNameStatic())
        PrepareComponent();
}

void EC_WebView::ComponentRemoved(IComponent *component, AttributeChange::Type change)
{
    /*! If this component is being removed we need to reset to the target meshes original materials.
        EC_3DCanvas is too stupid to do this (should be improved!) At this stage our parent
        entity is still valid. This will cease to exist if this behavior is changed in SceneManager and/or
        Entity classes. Parent entity is not valid in the dtor so this has to be done here.
        \todo Improve EC_3DCanvas to always know when the widget that its rendering is destroyed and reset the original materials.
        \note This implementation relies on parent entity being valid here. If this is changed in the future the above todo must be implemented.
        \note We are going to the dtor after this, it will free our objects, this is why we dont do it here.
    */
    if (component == this)
    {
        EC_3DCanvas *canvasSource = GetSceneCanvasComponent();
        if (canvasSource && webview_)
        {
            // If the widget that its rendering is ours, restore original materials.
            if (canvasSource->GetWidget() == webview_)
            {
                canvasSource->Stop();
                canvasSource->RestoreOriginalMeshMaterials();
                canvasSource->SetWidget(0);
            }
        }
    }
    ///\todo Add check if this component has another EC_Mesh then init EC_3DCanvas again for that! Now we just blindly stop this EC.
    else if (component->TypeName() == EC_Mesh::TypeNameStatic())
    {
        RenderTimerStop();
        componentPrepared_ = false;
    }
}

void EC_WebView::AttributeChanged(IAttribute *attribute, AttributeChange::Type changeType)
{
    if (attribute == &webviewUrl)
    {
        if (!componentPrepared_ || !webview_)
            return;

        // Load the 'getwebviewUrl' page to our QWebView if it's not empty.
        QString urlString = getwebviewUrl().simplified();
        if (urlString.isEmpty())
        {
            RenderTimerStop();

            // Restore the original materials from the mesh if user sets url to empty string.
            EC_3DCanvas *sceneCanvas = GetSceneCanvasComponent();
            if (sceneCanvas)
                sceneCanvas->RestoreOriginalMeshMaterials();
            return;
        }

        if (!urlString.startsWith("http://") && !urlString.startsWith("https://"))
            urlString = "http://" + urlString;
        webview_->load(QUrl(urlString, QUrl::TolerantMode));
    }
    else if (attribute == &webviewSize)
    {
        if (!webview_)
            return;

        QSize targetSize = getwebviewSize();
        if (webview_->size() != targetSize)
        {
            webview_->resize(targetSize);
            RenderDelayed();
        }
    }
    else if (attribute == &renderSubmeshIndex)
    {
        if (!componentPrepared_ || !webview_)
            return;
        RenderDelayed();
    }
    else if (attribute == &renderRefreshRate)
    {
        if (!componentPrepared_ || !webview_)
            return;
        
        RenderTimerStop();
        RenderTimerStartOrSingleShot();
    }
}

EC_Mesh *EC_WebView::GetMeshComponent()
{
    if (!GetParentEntity())
        return 0;
    EC_Mesh *mesh = GetParentEntity()->GetComponent<EC_Mesh>().get();
    return mesh;
}

EC_3DCanvas *EC_WebView::GetSceneCanvasComponent()
{
    if (!GetParentEntity())
        return 0;
    EC_3DCanvas *canvas = GetParentEntity()->GetComponent<EC_3DCanvas>().get();
    return canvas;
}
