// For conditions of distribution and use, see copyright notice in license.txt

#include "DebugOperatorNew.h"

#include "UiAPI.h"
#include "UiMainWindow.h"
#include "UiGraphicsView.h"
#include "QtUiAsset.h"
#include "UiProxyWidget.h"

#include "Framework.h"
#include "AssetAPI.h"
#include "GenericAssetFactory.h"
#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("UiAPI")

#include <QEvent>
#include <QLayout>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QUiLoader>
#include <QFile>
#include <QAction>

#include "MemoryLeakCheck.h"

/// The SuppressedPaintWidget is used as a viewport for the main Naali QGraphicsView.
/** Its purpose is to disable all automatic drawing of the QGraphicsView to screen so that
    we can composite an Ogre 3D render with the Qt widgets added to a QGraphicsScene. */
class SuppressedPaintWidget : public QWidget {
public:
    SuppressedPaintWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~SuppressedPaintWidget() {}

protected:
    virtual bool event(QEvent *event);
    virtual void paintEvent(QPaintEvent *event);
};

SuppressedPaintWidget::SuppressedPaintWidget(QWidget *parent, Qt::WindowFlags f)
:QWidget(parent, f)
{
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
}

bool SuppressedPaintWidget::event(QEvent *event)
{
    switch(event->type())
    {
    case QEvent::UpdateRequest:
    case QEvent::Paint:
    case QEvent::Wheel:
    case QEvent::Resize:
        return true;
    default:
        return QWidget::event(event);
    }
}

void SuppressedPaintWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
}

UiAPI::UiAPI(Foundation::Framework *owner_) :
    owner(owner_),
    mainWindow(0),
    graphicsView(0),
    graphicsScene(0)
{
    if (owner->IsHeadless())
        return;
    
    mainWindow = new UiMainWindow(owner);
    mainWindow->centralWidget()->setAutoFillBackground(false);
    //mainWindow->setUpdatesEnabled(false);

    // Apply the Naali main window icon. 
    // Note: this will only affect to a icon at main window left top corner.
    //       The application thumbnail icon must be set by adding icon resource
    //       to viewer project
    QIcon icon("./data/ui/images/icon/naali_logo_32px_RC1.ico");
    mainWindow->setWindowIcon(icon);

    graphicsView = new UiGraphicsView(mainWindow->centralWidget());

    ///\todo Memory leak below, see very end of ~Renderer() for comments.

    // QMainWindow has a layout by default. It will not let you set another.
    // Leave this check here if the window type changes to for example QWidget so we dont crash then.
    if (!mainWindow->centralWidget()->layout())
        mainWindow->centralWidget()->setLayout(new QVBoxLayout());
    mainWindow->centralWidget()->layout()->setMargin(0);
    mainWindow->centralWidget()->layout()->setContentsMargins(0,0,0,0);
    mainWindow->centralWidget()->layout()->addWidget(graphicsView);

    viewportWidget = new SuppressedPaintWidget();
    graphicsView->setViewport(viewportWidget);
    viewportWidget->setAttribute(Qt::WA_DontShowOnScreen, true);
    viewportWidget->setGeometry(0, 0, graphicsView->width(), graphicsView->height());
    viewportWidget->setContentsMargins(0,0,0,0);

    mainWindow->centralWidget()->setContentsMargins(0,0,0,0);
    graphicsView->setContentsMargins(0,0,0,0);
    
    graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    graphicsView->horizontalScrollBar()->setValue(0);
    graphicsView->horizontalScrollBar()->setRange(0, 0);

    graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    graphicsView->verticalScrollBar()->setValue(0);
    graphicsView->verticalScrollBar()->setRange(0, 0);

    graphicsScene = new QGraphicsScene(this);

    graphicsView->setScene(graphicsScene);
    graphicsView->scene()->setSceneRect(graphicsView->rect());
    connect(graphicsScene, SIGNAL(changed(const QList<QRectF> &)), graphicsView, SLOT(HandleSceneChanged(const QList<QRectF> &))); 
    connect(graphicsScene, SIGNAL(sceneRectChanged(const QRectF &)), SLOT(OnSceneRectChanged(const QRectF &)));

    connect(mainWindow, SIGNAL(WindowResizeEvent(int,int)), graphicsView, SLOT(Resize(int,int))); 

    mainWindow->LoadWindowSettingsFromFile();
    graphicsView->Resize(mainWindow->width(), mainWindow->height());

    // Unfortunate hack we have to do, will mess up the splash screen for a moment.
    // Reason why we need to show/hide hack is qt makes the QWidget::winId() valid on first show event.
    // This means we need to have a valid win id before ogre is loaded and the winid is passet to its renderer to paint into.
    mainWindow->show();
    mainWindow->hide();

    /// Do a full repaint of the view now that we've shown it.
    graphicsView->MarkViewUndirty();

    owner_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<QtUiAsset>("QtUiFile")));

    connect(mainWindow, SIGNAL(WindowCloseEvent()), owner, SLOT(Exit()));
}

UiAPI::~UiAPI()
{
    delete mainWindow;
    delete viewportWidget;
}

UiMainWindow *UiAPI::MainWindow() const
{
    return mainWindow;
}

UiGraphicsView *UiAPI::GraphicsView() const
{
    return graphicsView;
}

QGraphicsScene *UiAPI::GraphicsScene() const
{
    return graphicsScene;
}

UiProxyWidget *UiAPI::AddWidgetToScene(QWidget *widget, Qt::WindowFlags flags)
{
    if (!widget)
    {
        LogError("AddWidgetToScene called with a null proxy widget!");
        return 0;
    }

    // QGraphicsProxyWidget maintains symmetry for the following states:
    // state, enabled, visible, geometry, layoutDirection, style, palette,
    // font, cursor, sizeHint, getContentsMargins and windowTitle

    UiProxyWidget *proxy = new UiProxyWidget(widget, flags);
    assert(proxy->widget() == widget);
    
    // Synchronize windowState flags
    proxy->widget()->setWindowState(widget->windowState());

    AddProxyWidgetToScene(proxy);

    // If the widget has WA_DeleteOnClose on, connect its proxy's visibleChanged()
    // signal to a slot which handles the deletion. This must be done because closing
    // proxy window in our system doesn't yield closeEvent, but hideEvent instead.
    if (widget->testAttribute(Qt::WA_DeleteOnClose))
        connect(proxy, SIGNAL(visibleChanged()), SLOT(DeleteCallingWidgetOnClose()));

    return proxy;
}

bool UiAPI::AddProxyWidgetToScene(UiProxyWidget *widget)
{
    if (!widget)
    {
        LogError("AddWidgetToScene called with a null proxy widget!");
        return false;
    }

    if (!widget->widget())
    {
        LogError("AddWidgetToScene called for proxy widget that does not embed a widget!");
        return false;
    }

    if (widgets.contains(widget))
    {
        LogWarning("AddWidgetToScene: Scene already contains the given widget!");
        return false;
    }

    connect(widget, SIGNAL(destroyed(QObject *)), this, SLOT(OnProxyDestroyed(QObject *)));

    widgets.append(widget);

    if (widget->isVisible())
        widget->hide();

    // If no position has been set for Qt::Dialog widget, use default one so that the window's title
    // bar - or any other critical part, doesn't go outside the view.
    if ((widget->windowFlags() & Qt::Dialog) && widget->pos() == QPointF() && !(widget->widget()->windowState() & Qt::WindowFullScreen))
        widget->setPos(10.0, 200.0);

    // Resize full screen widgets to fit the scene rect.
    if (widget->widget()->windowState() & Qt::WindowFullScreen)
    {
        fullScreenWidgets << widget;
        widget->setGeometry(graphicsScene->sceneRect().toRect());
    }

    graphicsScene->addItem(widget);
    return true;
}

void UiAPI::RemoveWidgetFromScene(QWidget *widget)
{
    if (!widget)
        return;

    if (graphicsScene)
        graphicsScene->removeItem(widget->graphicsProxyWidget());
    widgets.removeOne(widget->graphicsProxyWidget());
    fullScreenWidgets.removeOne(widget->graphicsProxyWidget());
}

void UiAPI::RemoveWidgetFromScene(QGraphicsProxyWidget *widget)
{
    if (!widget)
        return;

    if (graphicsScene)
        graphicsScene->removeItem(widget);
    widgets.removeOne(widget);
    fullScreenWidgets.removeOne(widget);
}

void UiAPI::OnProxyDestroyed(QObject* obj)
{
    // Make sure we don't get dangling pointers
    // Note: at this point it's a QObject, not a QGraphicsProxyWidget anymore
    QGraphicsProxyWidget* proxy = static_cast<QGraphicsProxyWidget*>(obj);
    widgets.removeOne(proxy);
    fullScreenWidgets.removeOne(proxy);
}

QWidget *UiAPI::LoadFromFile(const QString &filePath, bool addToScene, QWidget *parent)
{
    QWidget *widget = 0;

    if (AssetAPI::ParseAssetRefType(filePath) != AssetAPI::AssetRefLocalPath)
    {
        AssetPtr asset = owner->Asset()->GetAsset(filePath);
        if (!asset)
        {
            LogError(("LoadFromFile: Asset \"" + filePath + "\" is not loaded to the asset system. Call RequestAsset prior to use!").toStdString());
            return 0;
        }

        QtUiAsset *uiAsset = dynamic_cast<QtUiAsset*>(asset.get());
        if (!uiAsset)
        {
            LogError(("LoadFromFile: Asset \"" + filePath + "\" is not of type QtUiFile!").toStdString());
            return 0;
        }
        if (!uiAsset->IsLoaded())
        {
            LogError(("LoadFromFile: Asset \"" + filePath + "\" data is not valid!").toStdString());
            return 0;
        }

        // Get the asset data with the assetrefs replaced to point to the disk sources on the current local system.
        QByteArray data = uiAsset->GetRefReplacedAssetData();
        
        QUiLoader loader;
        QDataStream dataStream(&data, QIODevice::ReadOnly);
        widget = loader.load(dataStream.device(), parent);
    }
    else // The file is from absolute source location.
    {
        QFile file(filePath);
        QUiLoader loader;
        file.open(QFile::ReadOnly);
        widget = loader.load(&file, parent);
    }

    if (!widget)
    {
        LogError(("LoadFromFile: Failed to load widget from file \"" + filePath + "\"!").toStdString());
        return 0;
    }

    if (addToScene && widget)
        AddWidgetToScene(widget);

    return widget;
}

void UiAPI::EmitContextMenuAboutToOpen(QMenu *menu, QList<QObject *> targets)
{
    emit ContextMenuAboutToOpen(menu,targets);
}

void UiAPI::EmitAddAction(QAction *action, const QString &group)
{
    emit AddAction(action, group);
}

void UiAPI::EmitOpenUrl(const QUrl &url)
{
    emit OpenUrl(url);
}

void UiAPI::ShowWidget(QWidget *widget) const
{
    if (!widget)
    {
        LogError("ShowWidget called on a null widget!");
        return;
    }

    if (widget->graphicsProxyWidget())
        widget->graphicsProxyWidget()->show();
    else
        widget->show();
}

void UiAPI::HideWidget(QWidget *widget) const
{
    if (!widget)
    {
        LogError("HideWidget called on a null widget!");
        return;
    }

    if (widget->graphicsProxyWidget())
        widget->graphicsProxyWidget()->hide();
    else
        widget->hide();
}

void UiAPI::BringWidgetToFront(QWidget *widget) const
{
    if (!widget)
    {
        LogError("BringWidgetToFront called on a null widget!");
        return;
    }

    ShowWidget(widget);
    graphicsScene->setActiveWindow(widget->graphicsProxyWidget());
    graphicsScene->setFocusItem(widget->graphicsProxyWidget(), Qt::ActiveWindowFocusReason);
}

void UiAPI::BringProxyWidgetToFront(QGraphicsProxyWidget *widget) const
{
    if (!widget)
    {
        LogError("BringWidgetToFront called on a null QGraphicsProxyWidget!");
        return;
    }

    graphicsScene->setActiveWindow(widget);
    graphicsScene->setFocusItem(widget, Qt::ActiveWindowFocusReason);
}

void UiAPI::OnSceneRectChanged(const QRectF &rect)
{
    foreach(QGraphicsProxyWidget *widget, fullScreenWidgets)
        widget->setGeometry(rect);
}

void UiAPI::DeleteCallingWidgetOnClose()
{
    QGraphicsProxyWidget *proxy = dynamic_cast<QGraphicsProxyWidget *>(sender());
    if (proxy && !proxy->isVisible())
        proxy->deleteLater();
}
