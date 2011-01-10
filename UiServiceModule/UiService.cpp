/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   UiService.cpp
 *  @brief  Light-weight UI service. Implements UiServiceInterface and provides 
 *          means of embedding Qt widgets to the same scene/canvas as the 3D in-world
 *          view. Uses only one UI scene for everything.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "AssetAPI.h"
#include "UiService.h"
#include "UiProxyWidget.h"

#include "MemoryLeakCheck.h"
#include "LoggingFunctions.h"
#include "BinaryAsset.h"
#include <QDomDocument>
#include <QUiLoader>

#include <QDebug>

DEFINE_POCO_LOGGING_FUNCTIONS("UiService")

UiService::UiService(Foundation::Framework *framework, QGraphicsView *view)
:view_(view), 
scene_(view->scene()),
framework_(framework)
{
    assert(view_);
    assert(scene_);

    connect(scene_, SIGNAL(sceneRectChanged(const QRectF &)), SLOT(SceneRectChanged(const QRectF &)));
}

UiService::~UiService()
{
}

UiProxyWidget *UiService::AddWidgetToScene(QWidget *widget, Qt::WindowFlags flags)
{
    if (!widget)
    {
        LogError("UiService::AddWidgetToScene called with a null proxywidget!");
        return 0;
    }

    /*  QGraphicsProxyWidget maintains symmetry for the following states:
     *  state, enabled, visible, geometry, layoutDirection, style, palette,
     *  font, cursor, sizeHint, getContentsMargins and windowTitle
     */

    UiProxyWidget *proxy = new UiProxyWidget(widget, flags);
    assert(proxy->widget() == widget);
    
    // Synchronize windowState flags
    proxy->widget()->setWindowState(widget->windowState());

    AddWidgetToScene(proxy);

    // If the widget has WA_DeleteOnClose on, connect its proxy's visibleChanged()
    // signal to a slot which handles the deletion. This must be done because closing
    // proxy window in our system doesn't yield closeEvent, but hideEvent instead.
    if (widget->testAttribute(Qt::WA_DeleteOnClose))
        connect(proxy, SIGNAL(visibleChanged()), SLOT(DeleteCallingWidgetOnClose()));

    return proxy;
}

bool UiService::AddProxyWidgetToScene(UiProxyWidget *proxy) { return AddWidgetToScene(proxy); }

bool UiService::AddWidgetToScene(UiProxyWidget *widget)
{
    if (!widget)
    {
        LogError("UiService::AddWidgetToScene called with a null proxywidget!");
        return false;
    }

    if (!widget->widget())
    {
        LogError("UiService::AddWidgetToScene called for proxywidget that does not embed a widget!");
        return false;
    }

    if (widgets_.contains(widget))
    {
        LogWarning("UiService::AddWidgetToScene: Scene already contains the given widget!");
        return false;
    }

    QObject::connect(widget, SIGNAL(destroyed(QObject *)), this, SLOT(OnProxyDestroyed(QObject *)));
    
    widgets_.append(widget);

    if (widget->isVisible())
        widget->hide();

    // If no position has been set for Qt::Dialog widget, use default one so that the window's title
    // bar - or any other critical part, doesn't go outside the view.
    if ((widget->windowFlags() & Qt::Dialog) && widget->pos() == QPointF() && !(widget->widget()->windowState() & Qt::WindowFullScreen))
        widget->setPos(10.0, 200.0);

    // Resize full screen widgets to fit the scene rect.
    if (widget->widget()->windowState() & Qt::WindowFullScreen)
    {
        fullScreenWidgets_ << widget;
        widget->setGeometry(scene_->sceneRect().toRect());
    }

    scene_->addItem(widget);
    return true;
}

void UiService::AddWidgetToMenu(QWidget *widget)
{
}

void UiService::AddWidgetToMenu(QWidget *widget, const QString &entry, const QString &menu, const QString &icon)
{
}

void UiService::AddWidgetToMenu(UiProxyWidget *widget, const QString &entry, const QString &menu, const QString &icon)
{
}

void UiService::RemoveWidgetFromScene(QWidget *widget)
{
    if (!widget)
        return;

    if (scene_)
        scene_->removeItem(widget->graphicsProxyWidget());
    widgets_.removeOne(widget->graphicsProxyWidget());
    fullScreenWidgets_.removeOne(widget->graphicsProxyWidget());
}

void UiService::RemoveWidgetFromScene(QGraphicsProxyWidget *widget)
{
    if (!widget)
        return;

    if (scene_)
        scene_->removeItem(widget);
    widgets_.removeOne(widget);
    fullScreenWidgets_.removeOne(widget);
}

void UiService::OnProxyDestroyed(QObject* obj)
{
    // Make sure we don't get dangling pointers
    // Note: at this point it's a QObject, not a QGraphicsProxyWidget anymore
    QGraphicsProxyWidget* proxy = static_cast<QGraphicsProxyWidget*>(obj);
    widgets_.removeOne(proxy);
    fullScreenWidgets_.removeOne(proxy);
}

QByteArray UiService::UpdateAssetPaths(const QByteArray& data)
{
    QDomDocument document;
    QString str;
    
    if ( document.setContent(data) )
    {
       AssetAPI *assetAPI = framework_->Asset();
     
       str = document.toString();
      
       while(str.contains("local://", Qt::CaseInsensitive) || str.contains("file://", Qt::CaseInsensitive))
       {
            int sIndex = str.indexOf("local://",0,Qt::CaseInsensitive);
            if ( sIndex == -1 )
            {
                sIndex = str.indexOf("file://", 0, Qt::CaseInsensitive);
                if ( sIndex == -1)
                    break;
            }
          
            int eIndex = 0;
            int i = str.size();

            for (i = sIndex; i < str.size(); ++i )
            {
                if ( QString(str[i]) == QString(")") || QString(str[i]) == QString("<") )
                {
                    eIndex = i;
                    break;
                }
            }

            if ( i < str.size())
            {
                QString name = str.mid(sIndex, eIndex-sIndex);
             
                // Ok we have possible asset candidate check that is it loaded, if it is not loaded assume that currently we have not loaded anything.
                AssetPtr asset;
                asset = assetAPI->GetAsset(name);
                
                if (!asset)
                {
                    LogError(("UiService::UpdateAssetPaths: Asset \"" + name + "\" is not loaded to the asset system. Call RequestAsset prior to use!").toStdString());
                    return data;
                }
                // Get absolute path where this asset is ..
                QString fileName = asset->DiskSource();
                str.replace(name, fileName);
                
                
            }
            else
            {   
                // We are end-of-file
                break;
            }
        } 

       // External asset ref (example, http:// or something)

       while(str.contains("://", Qt::CaseInsensitive) )
       {
            int sIndex = str.indexOf("://",0,Qt::CaseInsensitive);
            if ( sIndex == -1 )
                break;
        
            // Get type
            int eIndex = 0;
            for (int j = sIndex; j--;)
            {
                if ( QString(str[j]) == QString("(") || QString(str[j]) == QString(">") )
                {
                    eIndex = j+1;
                    break;
                }
            }
           
            int typeSize = sIndex - eIndex;
            sIndex = sIndex - typeSize;
            QString tmp = str.mid(sIndex, typeSize);
       
            int i = str.size();
            for (i = sIndex; i < str.size(); ++i )
            {
                if ( QString(str[i]) == QString(")") || QString(str[i]) == QString("<") )
                {
                    eIndex = i;
                    break;
                }
            }

            if ( i < str.size())
            {
                QString name = str.mid(sIndex, eIndex-sIndex);
             
                // Ok we have possible asset candidate check that is it loaded, if it is not loaded assume that currently we have not loaded anything.
                AssetPtr asset;
                asset = assetAPI->GetAsset(name);
                
                if (!asset)
                {
                    LogError(("UiService::UpdateAssetPaths: Asset \"" + name + "\" is not loaded to the asset system. Call RequestAsset prior to use!").toStdString());
                    return data;
                }
                // Get absolute path where this asset is ..
                QString fileName = asset->DiskSource();
                str.replace(name, fileName);
                
                
            }
            else
            {   
                // We are end-of-file
                break;
            }
        } 
           
    
    }
    else
    {   
        LogError("UiService::UpdateAssetNames given data was not valid xml file");
        return data;
    }
  
    return str.toUtf8();
}




QWidget *UiService::LoadFromFile(const QString &file_path, bool add_to_scene, QWidget *parent)
{
    AssetAPI *assetAPI = framework_->Asset();
    QString outPath = "";
    AssetPtr asset;
    QWidget *widget = 0;

    if (AssetAPI::ParseAssetRefType(file_path) != AssetAPI::AssetRefLocalPath)
    {
        asset = assetAPI->GetAsset(file_path);
        if (!asset)
        {
            LogError(("UiService::LoadFromFile: Asset \"" + file_path + "\" is not loaded to the asset system. Call RequestAsset prior to use!").toStdString());
            return 0;
        }
        BinaryAssetPtr binaryAsset = boost::dynamic_pointer_cast<BinaryAsset>(asset);
        if (!binaryAsset)
        {
            LogError(("UiService::LoadFromFile: Asset \"" + file_path + "\" is not of type BinaryAsset!").toStdString());
            return 0;
        }
        if (binaryAsset->data.size() == 0)
        {
            LogError(("UiService::LoadFromFile: Asset \"" + file_path + "\" size is zero!").toStdString());
            return 0;
        }

        QByteArray data((char*)&binaryAsset->data[0], binaryAsset->data.size());
        // Update asset paths
        data = UpdateAssetPaths(data);
        
        QDataStream dataStream(&data, QIODevice::ReadOnly);
        QUiLoader loader;
        widget = loader.load(dataStream.device(), parent);
    }
    else // The file is from absolute source location.
    {
        QFile file(file_path); 
        QUiLoader loader;
        file.open(QFile::ReadOnly);    
        widget = loader.load(&file, parent);
    }

    if (!widget)
    {
        LogError(("UiService::LoadFromFile: Failed to load widget from file \"" + file_path + "\"!").toStdString());
        return 0;
    }

    if (add_to_scene && widget)
        AddWidgetToScene(widget);
    return widget;
}
    
void UiService::RemoveWidgetFromMenu(QWidget *widget)
{
}

void UiService::RemoveWidgetFromMenu(QGraphicsProxyWidget *widget)
{
}

void UiService::ShowWidget(QWidget *widget) const
{
    if (!widget)
    {
        LogError("UiService::ShowWidget called on a null widget!");
        return;
    }

    if (widget->graphicsProxyWidget())
        widget->graphicsProxyWidget()->show();
    else
        widget->show();
}

void UiService::HideWidget(QWidget *widget) const
{
    if (!widget)
    {
        LogError("UiService::HideWidget called on a null widget!");
        return;
    }

    if (widget->graphicsProxyWidget())
        widget->graphicsProxyWidget()->hide();
    else
        widget->hide();
}

void UiService::BringWidgetToFront(QWidget *widget) const
{
    if (!widget)
    {
        LogError("UiService::BringWidgetToFront called on a null widget!");
        return;
    }

    ShowWidget(widget);
    scene_->setActiveWindow(widget->graphicsProxyWidget());
    scene_->setFocusItem(widget->graphicsProxyWidget(), Qt::ActiveWindowFocusReason);
}

void UiService::BringWidgetToFront(QGraphicsProxyWidget *widget) const
{
    if (!widget)
    {
        LogError("UiService::BringWidgetToFront called on a null QGraphicsProxyWidget!");
        return;
    }

    scene_->setActiveWindow(widget);
    scene_->setFocusItem(widget, Qt::ActiveWindowFocusReason);
}

void UiService::SceneRectChanged(const QRectF &rect)
{
    foreach(QGraphicsProxyWidget *widget, fullScreenWidgets_)
        widget->setGeometry(rect);
}

void UiService::DeleteCallingWidgetOnClose()
{
    QGraphicsProxyWidget *proxy = dynamic_cast<QGraphicsProxyWidget *>(sender());
    if (proxy && !proxy->isVisible())
        proxy->deleteLater();
}
