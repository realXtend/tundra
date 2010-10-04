// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "TimeProfilerWindow.h"
#include "Profiler.h"
#include "DebugStats.h"
#include "HighPerfClock.h"
#include "Framework.h"
#include "NetworkMessages/NetInMessage.h"
#include "NetworkMessages/NetOutMessage.h"
#include "NetworkMessages/NetMessageManager.h"
#include "AssetServiceInterface.h"
#include "WorldStream.h"
#include "SceneManager.h"
#include "RenderServiceInterface.h"
#include "EC_OpenSimPrim.h"
#include "EC_Mesh.h"
#include "EC_OgreCustomObject.h"
#include "EC_Terrain.h"
#include <AssetEvents.h>
#include <EventManager.h>
//#include "RealXtend/RexProtocolMsgIDs.h"
//#include "GenericMessageUtils.h"

#include <utility>

#include <QVBoxLayout>
#include <QTreeWidget>
#include <QUiLoader>
#include <QFile>
#include <QHeaderView>
#include <QPainter>
#include <QTreeWidgetItemIterator>
#include <QtAlgorithms>
#include <QTextEdit>

#include <OgreFontManager.h>

#include "MemoryLeakCheck.h"

using namespace std;

namespace DebugStats
{

const QString DEFAULT_LOG_DIR("logs");

TimeProfilerWindow::TimeProfilerWindow(Foundation::Framework *fw) : framework_(fw)
{
    QUiLoader loader;
    QFile file("./data/ui/profiler.ui");
    file.open(QFile::ReadOnly);
    contents_widget_ = loader.load(&file, this);
    assert(contents_widget_);
    file.close();

    setWindowTitle(tr("Profiler"));

    QVBoxLayout *layout = new QVBoxLayout;
    assert(layout);
    layout->addWidget(contents_widget_);
    layout->setContentsMargins(0,0,0,0);
    setLayout(layout);

    tree_profiling_data_ = findChild<QTreeWidget*>("treeProfilingData");
    combo_timing_refresh_interval_ = findChild<QComboBox*>("comboTimingRefreshInterval");
    tab_widget_ = findChild<QTabWidget*>("tabWidget");
    label_frame_time_history_ = findChild<QLabel*>("labelFrameTimeHistory");
    label_top_frame_time_ = findChild<QLabel*>("labelTopFrameTime");
    label_time_per_frame_ = findChild<QLabel*>("labelTimePerFrame");
    assert(tab_widget_);
    assert(tree_profiling_data_);
    assert(combo_timing_refresh_interval_);
    assert(label_frame_time_history_);
    assert(label_top_frame_time_);

    // Create a QImage object and set it in label.
    QImage frameTimeHistory(label_frame_time_history_->width(), label_frame_time_history_->height(), QImage::Format_RGB32);
    frameTimeHistory.fill(0xFF000000);
    label_frame_time_history_->setPixmap(QPixmap::fromImage(frameTimeHistory));

    QLabel *label = findChild<QLabel*>("labelDataInSecGraph");
    QImage img(label->width(), label->height(), QImage::Format_RGB32);
    img.fill(0xFF000000);
    label->setPixmap(QPixmap::fromImage(img));

    label = findChild<QLabel*>("labelDataOutSecGraph");
    img = QImage(label->width(), label->height(), QImage::Format_RGB32);
    img.fill(0xFF000000);
    label->setPixmap(QPixmap::fromImage(img));

    label = findChild<QLabel*>("labelPacketsInSecGraph");
    img = QImage(label->width(), label->height(), QImage::Format_RGB32);
    img.fill(0xFF000000);
    label->setPixmap(QPixmap::fromImage(img));

    label = findChild<QLabel*>("labelPacketsOutSecGraph");
    img = QImage(label->width(), label->height(), QImage::Format_RGB32);
    img.fill(0xFF000000);
    label->setPixmap(QPixmap::fromImage(img));

    const int headerHeight = tree_profiling_data_->headerItem()->sizeHint(0).height();

    tree_profiling_data_->header()->resizeSection(0, 300);
    tree_profiling_data_->header()->resizeSection(1, 60);
    tree_profiling_data_->header()->resizeSection(2, 50);
    tree_profiling_data_->header()->resizeSection(3, 50);
    tree_profiling_data_->header()->resizeSection(4, 50);

    connect(tab_widget_, SIGNAL(currentChanged(int)), this, SLOT(OnProfilerWindowTabChanged(int)));

    label_region_map_coords_ = findChild<QLabel*>("labelRegionMapCoords");
    label_region_object_capacity_ = findChild<QLabel*>("labelRegionObjectCapacity");
    tree_sim_stats_ = findChild<QTreeWidget*>("treeSimStats");
    label_pid_stat_ = findChild<QLabel*>("labelPidStat");
    assert(label_pid_stat_);
    assert(label_region_map_coords_);
    assert(label_region_object_capacity_);
    assert(tree_sim_stats_);

    tree_sim_stats_->header()->resizeSection(0, 400);
    tree_sim_stats_->header()->resizeSection(1, 100);

    show_profiler_tree_ = false;
    show_unused_ = false;

    push_button_toggle_tree_ = findChild<QPushButton*>("pushButtonToggleTree");
    push_button_collapse_all_ = findChild<QPushButton*>("pushButtonCollapseAll");
    push_button_expand_all_ = findChild<QPushButton*>("pushButtonExpandAll");
    push_button_show_unused_ = findChild<QPushButton*>("pushButtonShowUnused");
    assert(push_button_toggle_tree_);
    assert(push_button_collapse_all_);
    assert(push_button_expand_all_);
    assert(push_button_show_unused_);

    tree_asset_cache_ = findChild<QTreeWidget*>("treeAssetCache");
    tree_asset_transfers_ = findChild<QTreeWidget*>("treeAssetTransfers");
    assert(tree_asset_cache_);
    assert(tree_asset_transfers_);
    tree_asset_cache_->header()->resizeSection(1, 60);
    tree_asset_transfers_->header()->resizeSection(0, 240);
    tree_asset_transfers_->header()->resizeSection(1, 90);
    tree_asset_transfers_->header()->resizeSection(2, 90);
    
    text_scenecomplexity_ = findChild<QTextEdit*>("textSceneComplexity");
    assert(text_scenecomplexity_);
    
    tree_rendertargets_ = findChild<QTreeWidget*>("treeRenderTargets");
    assert(tree_rendertargets_);
    tree_rendertargets_->header()->resizeSection(0, 220);
    tree_rendertargets_->header()->resizeSection(1, 50);
    tree_rendertargets_->header()->resizeSection(2, 50);
    tree_rendertargets_->header()->resizeSection(3, 50);
    tree_rendertargets_->header()->resizeSection(4, 60);
    tree_rendertargets_->header()->resizeSection(5, 55);
    tree_rendertargets_->header()->resizeSection(6, 60);
    tree_rendertargets_->header()->resizeSection(7, 60);
    tree_rendertargets_->header()->resizeSection(8, 60);
    tree_rendertargets_->header()->resizeSection(9, 60);
    tree_rendertargets_->header()->resizeSection(10, 60);
    tree_rendertargets_->header()->resizeSection(11, 90);
    tree_rendertargets_->header()->resizeSection(12, 90);
    tree_rendertargets_->header()->resizeSection(13, 800);
    
    tree_texture_assets_ = findChild<QTreeWidget* >("textureDataTree"); 
    tree_mesh_assets_ = findChild<QTreeWidget* >("meshDataTree");
    tree_material_assets_ = findChild<QTreeWidget*>("materialDataTree");
    tree_skeleton_assets_ = findChild<QTreeWidget*>("skeletonDataTree");
    tree_compositor_assets_ = findChild<QTreeWidget*>("compositorDataTree");
    tree_gpu_assets_ = findChild<QTreeWidget*>("gpuDataTree");
    tree_font_assets_ = findChild<QTreeWidget*>("fontDataTree");

    QList<QPushButton* > buttons = findChildren<QPushButton* >();
    for (int i = 0; i < buttons.size(); ++i )
        if ( buttons[i]->objectName().contains("arrange") )
            connect(buttons[i], SIGNAL(clicked()), this, SLOT(Arrange()));

    connect(push_button_toggle_tree_, SIGNAL(pressed()), this, SLOT(ToggleTreeButtonPressed()));
    connect(push_button_collapse_all_, SIGNAL(pressed()), this, SLOT(CollapseAllButtonPressed()));
    connect(push_button_expand_all_, SIGNAL(pressed()), this, SLOT(ExpandAllButtonPressed()));
    connect(push_button_show_unused_, SIGNAL(pressed()), this, SLOT(ShowUnusedButtonPressed()));

    connect(findChild<QPushButton*>("pushButtonDumpOgreStats"), SIGNAL(pressed()), this, SLOT(DumpOgreResourceStatsToFile()));
    connect(findChild<QPushButton*>("buttonDumpSceneComplexity"), SIGNAL(pressed()), this, SLOT(DumpSceneComplexityToFile()));

    frame_time_update_x_pos_ = 0;
    logThreshold_ = 100.0;

    QDoubleSpinBox* box = findChild<QDoubleSpinBox* >("loggerSpinbox");
    if ( box != 0)
    {
        box->setRange(0.0, 1000.0);
        box->setValue(logThreshold_);
    }
    
    QPushButton* apply = findChild<QPushButton*>("loggerApply");
    if ( apply != 0)
    {
        apply->setCheckable(true);
        connect(apply, SIGNAL(clicked()), this, SLOT(ChangeLoggerThreshold()));
    }

    QCheckBox* checkBoxLogTraffic = findChild<QCheckBox*>("checkBoxLogTraffic");
    assert(checkBoxLogTraffic);
    if (checkBoxLogTraffic)
        connect(checkBoxLogTraffic, SIGNAL(stateChanged (int)), this, SLOT(SetNetworkLogging(int)));

    // Set/init working directory for log files.
    logDirectory_ = framework_->GetPlatform()->GetApplicationDataDirectory().c_str();
    if (!logDirectory_.exists(DEFAULT_LOG_DIR))
        logDirectory_.mkdir(DEFAULT_LOG_DIR);
    logDirectory_.cd(DEFAULT_LOG_DIR);

    QObject::connect(tree_mesh_assets_, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(ShowMeshAsset(QTreeWidgetItem*, int)));
    QObject::connect(tree_texture_assets_, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(ShowTextureAsset(QTreeWidgetItem*, int)));

    boost::shared_ptr<Foundation::EventManager> event_manager_ = framework_->GetEventManager();
    if ( event_manager_ != 0)
        asset_event_category_ = event_manager_->QueryEventCategory("Asset");

}

void TimeProfilerWindow::ShowMeshAsset(QTreeWidgetItem* item, int column)
{
    Asset::Events::AssetOpen open(item->text(0), QString::number(RexAT_Mesh));
    boost::shared_ptr<Foundation::EventManager> event_manager_ = framework_->GetEventManager();
    if ( event_manager_ != 0 )
    {
        event_manager_->SendEvent(asset_event_category_,Asset::Events::ASSET_OPEN, &open);
    }

}

void TimeProfilerWindow::ShowTextureAsset(QTreeWidgetItem* item, int column)
{
    Asset::Events::AssetOpen open(item->text(0), QString::number(RexAT_Texture));
    boost::shared_ptr<Foundation::EventManager> event_manager_ = framework_->GetEventManager();
    if ( event_manager_ != 0 )
    {
        event_manager_->SendEvent(asset_event_category_,Asset::Events::ASSET_OPEN, &open);
    }

}


void TimeProfilerWindow::ChangeLoggerThreshold()
{
     QDoubleSpinBox* box = findChild<QDoubleSpinBox* >("loggerSpinbox");
     if ( box != 0)
        logThreshold_ = box->value();
}

void TimeProfilerWindow::SetNetworkLogging(int value)
{
    QFile file(logDirectory_.path() + "/networking.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        return;

    QTextStream log(&file);
    if (value)
        log << "========== Logging started " << GetLocalTimeString().c_str() << " ==========\n";
    else if (!value)
    {
     // Dump packages 
     DumpNetworkSummary(&log);
     log << "========== Logging ended " << GetLocalTimeString().c_str() << "==========\n";
    }
}

void TimeProfilerWindow::DumpNetworkSummary(QTextStream* log)
{
    if ( log == 0)
    {
        QFile file(logDirectory_.path() + "/networking.txt");
        if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
            return;
    
        // ehheh
        DumpNetworkSummary(log);
    }
    else
    {

        (*log)<<endl;
        (*log)<<" Summary of network IN packages (name, bytes, count) "<<endl;
        
        for ( QMap<QString, NetworkLogData >::iterator iter = mapNetInData_.begin(); iter != mapNetInData_.end(); ++iter)
        {
            (*log)<<" "<<iter.key()<<" "<<QString::number(iter.value().bytes)<<" "<<QString::number(iter.value().packages)<<endl;
        }
        
        (*log)<<endl;
        (*log)<<" Summary of network OUT packages (name, bytes, count) "<<endl;
        
        for ( QMap<QString, NetworkLogData >::iterator iter = mapNetOutData_.begin(); iter != mapNetOutData_.end(); ++iter)
        {
            (*log)<<" "<<iter.key()<<" "<<QString::number(iter.value().bytes)<<" "<<QString::number(iter.value().packages)<<endl;
        }
        (*log)<<endl;
    }
 }


struct ResNameAndSize
{
    std::string name;
    bool isLoaded;
    size_t size;

    bool operator <(const ResNameAndSize &rhs) const
    {
        return size < rhs.size;
    }
};

bool LessThen(const QTreeWidgetItem* left, const QTreeWidgetItem* right)
{
    return left->text(1).toInt() < right->text(1).toInt();
}

void TimeProfilerWindow::Arrange()
{
    // Arrange. 
    if (!tab_widget_)
        return;

    int index = tab_widget_->currentIndex();
    QString name = QObject::sender()->objectName();

    QTreeWidget* widget = 0;
    switch (index)
    {
        case 8:  // Texture
            widget =  tree_texture_assets_;
            break;
        case 9: // Mesh
            widget =  tree_mesh_assets_;
            break;
        case 10: // Material
            widget =  tree_material_assets_;
            break;
        case 11:
            widget =  tree_skeleton_assets_;
            break;
        case 12:
            widget = tree_compositor_assets_ ;
            break;
        case 13:
            widget = tree_gpu_assets_;
            break;
        case 14:
            widget = tree_font_assets_;
            break;
    }

    if ( widget == 0)
        return;

    if ( name.contains("name") )
    {
        // By name
        widget->sortItems(0, Qt::AscendingOrder);
    }
    else if ( name.contains("size") )
    {
        // By size
        int va = widget->topLevelItemCount();
        QList<QTreeWidgetItem* > items;
        for ( int i = va; i--;)
        {
           QTreeWidgetItem* item =  widget->takeTopLevelItem(i);
           if ( item != 0)
               items.append(item);
           else
               break;
        }

        if ( items.size () != 0)
        {
            qStableSort(items.begin(), items.end(), LessThen);
            widget->addTopLevelItems(items);
        }
    }
}

void TimeProfilerWindow::ToggleTreeButtonPressed()
{
    show_profiler_tree_ = !show_profiler_tree_;
    tree_profiling_data_->clear();

    if (show_profiler_tree_)
    {
        RefreshProfilingDataTree();
        push_button_toggle_tree_->setText("Top");
    }
    else
    {
        RefreshProfilingDataList();
        push_button_toggle_tree_->setText("Tree");
    }
}

void TimeProfilerWindow::CollapseAllButtonPressed()
{
    tree_profiling_data_->collapseAll();
}

void TimeProfilerWindow::ExpandAllButtonPressed()
{
    tree_profiling_data_->expandAll();
}

void TimeProfilerWindow::ShowUnusedButtonPressed()
{
    show_unused_ = !show_unused_;
    tree_profiling_data_->clear();

    if (show_unused_)
        push_button_show_unused_->setText("Hide Unused");
    else
        push_button_show_unused_->setText("Show Unused");

    if (show_profiler_tree_)
        RefreshProfilingDataTree();
    else
        RefreshProfilingDataList();

    ExpandAllButtonPressed();
}

void TimeProfilerWindow::OnProfilerWindowTabChanged(int newPage)
{
    if (newPage == -1)
        newPage = tab_widget_->currentIndex();

    switch(newPage)
    {
    case 0:
        RefreshProfilingData();
        break;
    case 2:
        RefreshOgreProfilingWindow();
        break;
    case 3:
        RefreshNetworkProfilingData();
        break;
    case 5:
        RefreshAssetProfilingData();
        break;
    case 6:
        RefreshSceneComplexityProfilingData();
        break;
    case 7:
        RefreshRenderTargetProfilingData();
        break;
    case 8: // Textures
        RefreshAssetData(Ogre::TextureManager::getSingleton(), tree_texture_assets_ );
        break;
    case 9: // Meshes
        RefreshAssetData(Ogre::MeshManager::getSingleton(), tree_mesh_assets_ );
        break;
    case 10: // Material
        RefreshAssetData(Ogre::MaterialManager::getSingleton(), tree_material_assets_);
        break;
    case 11: // Skeleton
        RefreshAssetData(Ogre::SkeletonManager::getSingleton(), tree_skeleton_assets_);
        break;
    case 12: // Composition
        RefreshAssetData(Ogre::CompositorManager::getSingleton(), tree_compositor_assets_);
        break;
    case 13: // Gpu assets
        RefreshAssetData(Ogre::HighLevelGpuProgramManager::getSingleton(), tree_gpu_assets_);
        break;
    case 14: // Font assets
        RefreshAssetData(Ogre::FontManager::getSingleton(), tree_font_assets_);
        break;
    }
}

uint TimeProfilerWindow::GetNumResources(Ogre::ResourceManager& manager)
{
    uint count = 0;
    Ogre::ResourceManager::ResourceMapIterator iter = manager.getResourceIterator();
    while(iter.hasMoreElements())
    {
        Ogre::ResourcePtr resource = iter.getNext();
        if (!resource->isLoaded())
            continue;
        count++;
    }
    return count;
}

static QTreeWidgetItem *FindItemByName(QTreeWidgetItem *parent, const char *name)
{
    for(int i = 0; i < parent->childCount(); ++i)
        if (parent->child(i)->text(0) == name)
            return parent->child(i);

    return 0;
}

static QTreeWidgetItem *FindItemByName(QTreeWidget *parent, const char *name)
{
    for(int i = 0; i < parent->topLevelItemCount(); ++i)
        if (parent->topLevelItem(i)->text(0) == name)
            return parent->topLevelItem(i);

    return 0;
}

void TimeProfilerWindow::SetWorldStreamPtr(ProtocolUtilities::WorldStreamPtr worldStream)
{
    world_stream_ = worldStream;
}

void TimeProfilerWindow::FillProfileTimingWindow(QTreeWidgetItem *qtNode, const Foundation::ProfilerNodeTree *profilerNode)
{
    using namespace Foundation;
    const ProfilerNodeTree::NodeList &children = profilerNode->GetChildren();
    for(ProfilerNodeTree::NodeList::const_iterator iter = children.begin(); iter != children.end(); ++iter)
    {
        ProfilerNodeTree *node = iter->get();

        const ProfilerNode *timings_node = dynamic_cast<const ProfilerNode*>(node);
        if (timings_node && timings_node->num_called_ == 0 && !show_unused_)
            continue;

//        QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString(node->Name().c_str())));

        QTreeWidgetItem *item = FindItemByName(qtNode, node->Name().c_str());
        if (!item)
        {
            item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString(node->Name().c_str())));
            qtNode->addChild(item);
        }

        if (timings_node)
        {
            char str[256] = "-";
            if (timings_node->num_called_custom_ > 0 && timings_node->custom_elapsed_min_ < 1e8)
                sprintf(str, "%.2fms", timings_node->custom_elapsed_min_*1000.f);
            item->setText(2, str);
            if (timings_node->num_called_custom_ > 0)
                sprintf(str, "%.2fms", timings_node->total_custom_*1000.f / timings_node->num_called_custom_);
            item->setText(3, str);
            if (timings_node->num_called_custom_ > 0)
                sprintf(str, "%.2fms", timings_node->custom_elapsed_max_*1000.f);
            item->setText(4, str);
            sprintf(str, "%d", (int)timings_node->num_called_custom_);
            item->setText(1, str);

            timings_node->num_called_custom_ = 0;
            timings_node->total_custom_ = 0;
            timings_node->custom_elapsed_min_ = 1e9;
            timings_node->custom_elapsed_max_ = 0;
        }

        FillProfileTimingWindow(item, node);
    }
}

void TimeProfilerWindow::RedrawFrameTimeHistoryGraphDelta(const std::vector<std::pair<boost::uint64_t, double> > &frameTimes)
{
    ///\todo Implement.
}

void TimeProfilerWindow::RedrawFrameTimeHistoryGraph(const std::vector<std::pair<boost::uint64_t, double> > &frameTimes)
{
    if (!tab_widget_ || tab_widget_->currentIndex() != 1)
        return;

    PROFILE(DebugStats_DrawTimeHistoryGraph);
//    QPixmap picture(label_frame_time_history_->width(), label_frame_time_history_->height());
    const QPixmap *pixmap = label_frame_time_history_->pixmap();
    QImage image = pixmap->toImage();

#ifdef _WINDOWS
    LARGE_INTEGER now;
    LARGE_INTEGER freq;
    QueryPerformanceCounter(&now);
    QueryPerformanceFrequency(&freq);
#endif
    const int numEntries = min<int>(frameTimes.size(), image.width());
    const int firstEntry = max<int>(0, frameTimes.size() - numEntries);

    double maxTime = 1.0 / 60;
    for(size_t i = firstEntry; i < frameTimes.size(); ++i)
        maxTime = max(maxTime, frameTimes[i].second);

    maxTime = min(maxTime, 0.5);

    // 20fps is the lowest allowed target.
    const double maxAllowedFrameTime = 1.0 / 20;
    // If we spend less time than this, it's still OK as we'll get 30fps.
    const double okFrameTime = 1.0 / 30;
    
    QPainter painter(&image);
    painter.fillRect(0, 0, image.width()-1, image.height()-1, QColor(0,0,0));
    
    const uint colorOdd = qRgb(0x40, 0x40, 0xFF);
    const uint colorEven = qRgb(0x40, 0xFF, 0x40);
    const uint colorTrouble = qRgb(0xFF, 0, 0);
    int x = image.width() - numEntries;
    for(int i = 0; i < numEntries; ++i, ++x)
    {
        double r = 1.0 - min(1.0, frameTimes[firstEntry + i].second / maxTime);
        int y = (int)((image.height()-1)*r);

        uint color = colorOdd;
#ifdef _WINDOWS
        double age = (double)frameTimes[firstEntry + i].first / freq.QuadPart;
        // Foundation::ProfilerBlock::ElapsedTimeSeconds(*(LARGE_INTEGER*)&frameTimes[firstEntry + i].first, now);
        color = (fmod(age, 2.0) >= 1.0) ? colorOdd : colorEven;
#endif
        if ((frameTimes[firstEntry + i].second / maxTime >= 1.0 &&
            frameTimes[firstEntry + i].second >= okFrameTime) || frameTimes[firstEntry + i].second >= maxAllowedFrameTime)
            color = colorTrouble;

        painter.setPen(QColor(color));
        painter.drawLine(x, y, x, image.height()-1);
    }

    int vertLine60FPS = (int)((image.height()-1)*(1.0 - min(1.0, 1.0/60 / maxTime)));
    int vertLine30FPS = (int)((image.height()-1)*(1.0 - min(1.0, 1.0/30 / maxTime)));
    int vertLine20FPS = (int)((image.height()-1)*(1.0 - min(1.0, 1.0/20 / maxTime)));
    int vertLine15FPS = (int)((image.height()-1)*(1.0 - min(1.0, 1.0/15 / maxTime)));

    if (vertLine60FPS >= 1)
        painter.fillRect(0, vertLine60FPS, image.width()-1, 3, QColor(0, 0xFF, 0, 0xB0));
    if (vertLine30FPS >= 1)
        painter.fillRect(0, vertLine30FPS, image.width()-1, 3, QColor(0xFF, 0xFF, 0x60, 0x80));
    if (vertLine20FPS >= 1)
        painter.fillRect(0, vertLine20FPS, image.width()-1, 3, QColor(0xFF, 0x50, 0x50, 0x80));
    if (vertLine15FPS >= 1)
        painter.fillRect(0, vertLine15FPS, image.width()-1, 3, QColor(0xFF, 0x00, 0x00, 0x80));

    label_frame_time_history_->setPixmap(QPixmap::fromImage(image));

    // Update max Time/frame label.
    char str[256];
    sprintf(str, "%dms", (int)(maxTime*1000.f + 0.49f));
    label_top_frame_time_->setText(str);

    // Update smoothed avg time/frame label.
    double avg = 0;
    double denom = 0;
    double smoothFactor = 1.0;
    const double smoothCoeff = 0.8;
    for(int j = 0, i = frameTimes.size()-1; i >= 0 && j < 10; ++j)
    {
        avg += frameTimes[i].second * smoothFactor;
        denom += smoothFactor;
        smoothFactor *= smoothCoeff;
    }
    float timePerFrame = static_cast<float>(avg * 1000.0/denom); 
    sprintf(str, "%.2f msecs/frame.", timePerFrame);
    label_time_per_frame_->setText(str);

    //if ( timePerFrame >= logThreshold_ )
    //     DumpNodeData();
}

/*
void TimeProfilerWindow::DumpNodeData()
{
   
    QFile file("performancelogger.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
         return;

    if ( tree_profiling_data_ == 0)
        return;

    QTextStream out(&file);
    
    // Refresh tree data. 
    RefreshProfilingDataTree();

    // Now write data from tree to file
    QTreeWidgetItemIterator it(tree_profiling_data_);
    int cols = tree_profiling_data_->columnCount();

    out<<endl;
    out<<"#---------------- FRAME -----------------------------";
    out<<endl;
    out<<endl;

    while (*it) 
     {
         QString name = (*it)->text(0);
         if ( name.contains("thread", Qt::CaseInsensitive) && !name.contains("ThreadTask") )
             out<<endl;

         for ( int i = 0; i < cols; ++i)
            out<<(*it)->text(i)<<" ";
         
         out<<endl;
         ++it;
     }
           
    file.close();
}
*/

void TimeProfilerWindow::DoThresholdLogging()
{
#ifdef PROFILING
    using namespace Foundation;

     QPushButton* button = findChild<QPushButton* >("loggerApply");
     if (button == 0)
         return;

     if (tab_widget_->currentIndex() != 1 && button->isChecked())
     {
        button->setChecked(false);
        return;
     }

     if (tab_widget_->currentIndex() == 1 && !button->isChecked())
        return;

    assert(logDirectory_.exists());
    QFile file(logDirectory_.path() + "/performancelogger.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
         return;

    QTextStream out(&file);

    Profiler &profiler = framework_->GetProfiler();
    profiler.Lock();

    ProfilerNodeTree *node = profiler.GetRoot();
    const ProfilerNodeTree::NodeList &children = node->GetChildren();
    for(ProfilerNodeTree::NodeList::const_iterator iter = children.begin(); iter != children.end(); ++iter)
    {
        node = iter->get();

        const ProfilerNode *timings_node = dynamic_cast<const ProfilerNode*>(node);
        if (timings_node && timings_node->num_called_ == 0 )
            continue;

        /*
        QTreeWidgetItem *item = FindItemByName(tree_profiling_data_, node->Name().c_str());
        if (!item)
        {
            item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString(node->Name().c_str())));
            tree_profiling_data_->addTopLevelItem(item);
        }
        */

        if (timings_node)
        {
            char str[256] = "-";
            QString line;
            line = QTime::currentTime().toString();
            QString name(timings_node->Name().c_str());
            line = line + " " + name;
            bool saveData = false;
            if (timings_node->num_called_custom_ > 0 && timings_node->custom_elapsed_min_ < 1e8)
                sprintf(str, "%.2fms", timings_node->custom_elapsed_min_*1000.f);

            line = line + QString(" ")+ QString(str);
            //item->setText(2, str);
            if (timings_node->num_called_custom_ > 0)
            {
                sprintf(str, "%.2fms", timings_node->total_custom_*1000.f / timings_node->num_called_custom_);
                //if ( ((timings_node->total_custom_*1000.f/ timings_node->num_called_custom_) * timings_node->num_called_) > logThreshold_)
                if ( timings_node->total_custom_*1000.f >= logThreshold_)
                    saveData = true;
            }    
            line = line + QString(" ") + QString(str);
            
            //Avg
            //item->setText(3, str);
            
            if (timings_node->num_called_custom_ > 0)
                sprintf(str, "%.2fms", timings_node->custom_elapsed_max_*1000.f);
            //max
            line = line + QString(" ") + QString(str);
            //item->setText(4, str);
            sprintf(str, "%d", (int)timings_node->num_called_custom_);
            line = line + QString(" ") + QString(str);
            if ( saveData)
               out<<line<<endl;
            //count
            //item->setText(1, str);

            timings_node->num_called_custom_ = 0;
            timings_node->total_custom_ = 0;
            timings_node->custom_elapsed_min_ = 1e9;
            timings_node->custom_elapsed_max_ = 0;
        }

        FillThresholdLogger(out, node);
    }

    profiler.Release();
#endif
}

void TimeProfilerWindow::LogNetInMessage(const ProtocolUtilities::NetInMessage *msg)
{
    if (findChild<QCheckBox*>("checkBoxLogTraffic")->isChecked())
    {
        assert(logDirectory_.exists());
        QFile file(logDirectory_.path() + "/networking.txt");
        if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
            return;

        QTextStream log(&file);
        log <<'[' << GetLocalTimeString().c_str() << "]\t" << "IN\t" << msg->GetMessageID() ;

        size_t numWidth = QString::number(msg->GetMessageID()).length();
        if (numWidth < 9)
            log << '\t';
        if (numWidth < 5)
            log << '\t';

       
        QString name(msg->GetMessageInfo()->name.c_str());
        log << '\t' << name;

        if ( mapNetInData_.contains(name) )
        {
            NetworkLogData& ob = mapNetInData_[name];
            ob.bytes += msg->GetDataSize();
            ++ob.packages;

        }
        else
        {
             NetworkLogData ob;
             ob.bytes = msg->GetDataSize();
             ob.packages = 0;
             mapNetInData_.insert(name, ob);
        }

        ///\todo Get MethogName out from GenericMessage
//        if (msg->GetMessageID() == RexNetMsgGenericMessage)
//            log << ' ' << ProtocolUtilities::ParseGenericMessageMethod(*msg).c_str();

        log << '\n';
    }
}

void TimeProfilerWindow::LogNetOutMessage(const ProtocolUtilities::NetOutMessage *msg)
{
    if (findChild<QCheckBox*>("checkBoxLogTraffic")->isChecked())
    {
        assert(logDirectory_.exists());
        QFile file(logDirectory_.path() + "/networking.txt");
        if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
            return;

        QTextStream log(&file);
        log <<'[' << GetLocalTimeString().c_str() << "]\t" << "OUT\t" << msg->GetMessageID();

        size_t numWidth = QString::number(msg->GetMessageID()).length();
        if (numWidth < 9)
            log << '\t';
        if (numWidth < 5)
            log << '\t';

        
        QString name(msg->GetMessageInfo()->name.c_str());
        log << '\t' << name;
        
         if ( mapNetOutData_.contains(name) )
         {
            NetworkLogData& ob = mapNetOutData_[name];
            ob.bytes += msg->BytesFilled();
            ++ob.packages;
         }
         else
         {
            NetworkLogData ob;
            ob.bytes = msg->BytesFilled();
            ob.packages = 0;
            mapNetOutData_.insert(name, ob);
         }

        ///\todo Get MethogName out from GenericMessage
//        if (msg->GetMessageID() == RexNetMsgGenericMessage)
//            log << ' ' << ProtocolUtilities::ParseGenericMessageMethod(*msg).c_str();

         log << '\n';
    }
}

void TimeProfilerWindow::FillThresholdLogger(QTextStream& out, const Foundation::ProfilerNodeTree *profilerNode)
{
    using namespace Foundation;
    const ProfilerNodeTree::NodeList &children = profilerNode->GetChildren();
    for(ProfilerNodeTree::NodeList::const_iterator iter = children.begin(); iter != children.end(); ++iter)
    {
        ProfilerNodeTree *node = iter->get();

        const ProfilerNode *timings_node = dynamic_cast<const ProfilerNode*>(node);
        if (timings_node && timings_node->num_called_ == 0 )
           continue;

//        QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString(node->Name().c_str())));
/*
        QTreeWidgetItem *item = FindItemByName(qtNode, node->Name().c_str());
        if (!item)
        {
            item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString(node->Name().c_str())));
            qtNode->addChild(item);
        }
*/
        if (timings_node)
        {
            char str[256] = "-";
            QString line;
            line = QTime::currentTime().toString();
            QString name(timings_node->Name().c_str());
            line = line + " " + name;
            bool saveData = false;
            if (timings_node->num_called_custom_ > 0 && timings_node->custom_elapsed_min_ < 1e8)
                sprintf(str, "%.2fms", timings_node->custom_elapsed_min_*1000.f);
                
            line = line + QString(" ")+ QString(str);
            //item->setText(2, str);
            if (timings_node->num_called_custom_ > 0)
            {
                sprintf(str, "%.2fms", timings_node->total_custom_*1000.f / timings_node->num_called_custom_);
                 //if ( ((timings_node->total_custom_*1000.f/ timings_node->num_called_custom_) * timings_node->num_called_) > logThreshold_)
                if ( timings_node->total_custom_*1000.f >= logThreshold_ )     
                    saveData = true;
               
            }    
            line = line + QString(" ") + QString(str);
            
            //Avg
            //item->setText(3, str);
            
            if (timings_node->num_called_custom_ > 0)
                sprintf(str, "%.2fms", timings_node->custom_elapsed_max_*1000.f);
            //max
            line = line + QString(" ") + QString(str);
            //item->setText(4, str);
            sprintf(str, "%d", (int)timings_node->num_called_custom_);
            line = line + QString(" ") + QString(str);
            if ( saveData)
               out<<line<<endl;
            //count
            //item->setText(1, str);

            timings_node->num_called_custom_ = 0;
            timings_node->total_custom_ = 0;
            timings_node->custom_elapsed_min_ = 1e9;
            timings_node->custom_elapsed_max_ = 0;
            /*
            char str[256] = "-";
            if (timings_node->num_called_custom_ > 0 && timings_node->custom_elapsed_min_ < 1e8)
                sprintf(str, "%.2fms", timings_node->custom_elapsed_min_*1000.f);
            item->setText(2, str);
            
            if (timings_node->num_called_custom_ > 0)
                sprintf(str, "%.2fms", timings_node->total_custom_*1000.f / timings_node->num_called_custom_);
            item->setText(3, str);
            
            if (timings_node->num_called_custom_ > 0)
                sprintf(str, "%.2fms", timings_node->custom_elapsed_max_*1000.f);
            
            item->setText(4, str);
            sprintf(str, "%d", (int)timings_node->num_called_custom_);
            item->setText(1, str);

            timings_node->num_called_custom_ = 0;
            timings_node->total_custom_ = 0;
            timings_node->custom_elapsed_min_ = 1e9;
            timings_node->custom_elapsed_max_ = 0;
            */
        }

        FillThresholdLogger(out, node);
    }

}


void TimeProfilerWindow::resizeEvent(QResizeEvent *event)
{
//    contents_widget_->resize(this->width()-10, this->height()-10);
//    tab_widget_->resize(this->width()-20, this->height()-20);
//    tree_profiling_data_->resize(this->width()-40, this->height()-80);

    label_frame_time_history_->resize(this->width()-75, this->height()-75);

//    tree_sim_stats_->resize(this->width()-40, this->height()-80);

    QImage frameTimeHistory(label_frame_time_history_->width(), label_frame_time_history_->height(), QImage::Format_RGB32);
    frameTimeHistory.fill(0);
    label_frame_time_history_->setPixmap(QPixmap::fromImage(frameTimeHistory));

}

int TimeProfilerWindow::ReadProfilingRefreshInterval()
{
    assert(combo_timing_refresh_interval_);

    // Positive values denote
    const int refreshTimes[] = { 10000, 5000, 2000, 1000, 500 };

    int selection = combo_timing_refresh_interval_->currentIndex();
    return refreshTimes[min(max(selection, 0), (int)(sizeof(refreshTimes)/sizeof(refreshTimes[0])-1))];
}

template<typename T>
int CountSize(Ogre::MapIterator<T> iter)
{
    int count = 0;
    while(iter.hasMoreElements())
    {
        ++count;
        iter.getNext();
    }
    return count;
}

std::string FormatBytes(int bytes)
{
    char str[256];
    if (bytes < 0)
        return "-";
    if (bytes <= 1024)
        sprintf(str, "%d Bytes", bytes);
    else if (bytes <= 1024 * 1024)
        sprintf(str, "%.2f KBytes", bytes / 1024.f);
    else if (bytes <= 1024 * 1024 * 1024)
        sprintf(str, "%.2f MBytes", bytes / 1024.f / 1024.f);
    else
        sprintf(str, "%.2f GBytes", bytes / 1024.f / 1024.f / 1024.f);

    return str;
}

std::string FormatBytes(double bytes)
{
    char str[256];
    if (bytes < 0)
        return "-";
    if (bytes <= 1024)
        sprintf(str, "%.1f Bytes", (float)bytes);
    else if (bytes <= 1024 * 1024)
        sprintf(str, "%.2f KBytes", (float)bytes / 1024.f);
    else if (bytes <= 1024 * 1024 * 1024)
        sprintf(str, "%.2f MBytes", (float)bytes / 1024.f / 1024.f);
    else
        sprintf(str, "%.2f GBytes", (float)bytes / 1024.f / 1024.f / 1024.f);

    return str;
}

static std::string ReadOgreManagerStatus(Ogre::ResourceManager &manager)
{
    char str[256];
    Ogre::ResourceManager::ResourceMapIterator iter = manager.getResourceIterator();
    sprintf(str, "Budget: %s, Usage: %s, # of resources: %d",
        FormatBytes((int)manager.getMemoryBudget()).c_str(),
        FormatBytes((int)manager.getMemoryUsage()).c_str(),
        CountSize(iter));
    return str;
}

void TimeProfilerWindow::RefreshOgreProfilingWindow()
{
    if (!visibility_ || !tab_widget_ || tab_widget_->currentIndex() != 2)
        return;

    Ogre::Root *root = Ogre::Root::getSingletonPtr();
    assert(root);
/*
    Ogre::CompositorManager
    Ogre::TextureManager
    Ogre::MeshManager
    Ogre::MaterialManager
    Ogre::SkeletonManager
    Ogre::HighLevelGpuProgramManager
    Ogre::FontManager
*/
/*
    Ogre::ControllerManager
    Ogre::DefaultHardwareBufferManager
    Ogre::GpuProgramManager
    Ogre::OverlayManager
    Ogre::ParticleSystemManager
*/
//    Ogre::SceneManagerEnumerator::SceneManagerIterator iter = root->getSceneManagerIterator();
    findChild<QLabel*>("labelNumSceneManagers")->setText(QString("%1").arg(CountSize(root->getSceneManagerIterator())));
    findChild<QLabel*>("labelNumArchives")->setText(QString("%1").arg(CountSize(Ogre::ArchiveManager::getSingleton().getArchiveIterator())));

    findChild<QLabel*>("labelTextureManager")->setText(ReadOgreManagerStatus(Ogre::TextureManager::getSingleton()).c_str());
    findChild<QLabel*>("labelMeshManager")->setText(ReadOgreManagerStatus(Ogre::MeshManager::getSingleton()).c_str());
    findChild<QLabel*>("labelMaterialManager")->setText(ReadOgreManagerStatus(Ogre::MaterialManager::getSingleton()).c_str());
    findChild<QLabel*>("labelSkeletonManager")->setText(ReadOgreManagerStatus(Ogre::SkeletonManager::getSingleton()).c_str());
    findChild<QLabel*>("labelCompositorManager")->setText(ReadOgreManagerStatus(Ogre::CompositorManager::getSingleton()).c_str());
    findChild<QLabel*>("labelGPUProgramManager")->setText(ReadOgreManagerStatus(Ogre::HighLevelGpuProgramManager::getSingleton()).c_str());
    findChild<QLabel*>("labelFontManager")->setText(ReadOgreManagerStatus(Ogre::FontManager::getSingleton()).c_str());

    Ogre::SceneManager *scene = root->getSceneManagerIterator().getNext();
    if (scene)
    {
        findChild<QLabel*>("labelCameras")->setText(QString("%1").arg(CountSize(scene->getCameraIterator())));
        findChild<QLabel*>("labelAnimations")->setText(QString("%1").arg(CountSize(scene->getAnimationIterator())));
        findChild<QLabel*>("labelAnimationStates")->setText(QString("%1").arg(CountSize(scene->getAnimationStateIterator())));
        findChild<QLabel*>("labelLights")->setText(QString("%1").arg(CountSize(scene->getMovableObjectIterator(Ogre::LightFactory::FACTORY_TYPE_NAME))));
        findChild<QLabel*>("labelEntities")->setText(QString("%1").arg(CountSize(scene->getMovableObjectIterator(Ogre::EntityFactory::FACTORY_TYPE_NAME))));
        findChild<QLabel*>("labelBillboardSets")->setText(QString("%1").arg(CountSize(scene->getMovableObjectIterator(Ogre::BillboardSetFactory::FACTORY_TYPE_NAME))));
        findChild<QLabel*>("labelManualObjects")->setText(QString("%1").arg(CountSize(scene->getMovableObjectIterator(Ogre::ManualObjectFactory::FACTORY_TYPE_NAME))));
        findChild<QLabel*>("labelBillboardChains")->setText(QString("%1").arg(CountSize(scene->getMovableObjectIterator(Ogre::BillboardChainFactory::FACTORY_TYPE_NAME))));
        findChild<QLabel*>("labelRibbonTrails")->setText(QString("%1").arg(CountSize(scene->getMovableObjectIterator(Ogre::RibbonTrailFactory::FACTORY_TYPE_NAME))));
        findChild<QLabel*>("labelParticleSystems")->setText(QString("%1").arg(CountSize(scene->getMovableObjectIterator(Ogre::ParticleSystemFactory::FACTORY_TYPE_NAME))));
    }

    QTimer::singleShot(500, this, SLOT(RefreshOgreProfilingWindow()));
}

static void DumpOgreResManagerStatsToFile(Ogre::ResourceManager &manager, std::ofstream &file)
{
    std::vector<ResNameAndSize> resources;

    Ogre::ResourceManager::ResourceMapIterator iter = manager.getResourceIterator();

    file << "In load order(?): " << std::endl;
    while(iter.hasMoreElements())
    {
        Ogre::ResourcePtr resource = iter.getNext();
        file << "Name: \"" << resource->getName() << "\", size: " << resource->getSize() 
            << ", isReloadable: " << resource->isReloadable() 
            << ", isManuallyLoaded: " << resource->isManuallyLoaded() 
            << ", isPrepared: " << resource->isPrepared() 
            << ", isLoaded: " << resource->isLoaded()
            << ", isLoading: " << resource->isLoading()
            << ", getLoadingState: " << resource->getLoadingState() // todo: Could convert this enum to string.
            << ", isBackgroundLoaded: " << resource->isBackgroundLoaded()
            << ", group: " << resource->getGroup()
            << ", origin: " << resource->getOrigin()
            << ", stateCount: " << resource->getStateCount() << std::endl;

        ResNameAndSize r;
        r.name = resource->getName();
        r.size = resource->getSize();
        r.isLoaded = resource->isLoaded();
        resources.push_back(r);
    }

    file << "By descending size: " << std::endl;
    std::sort(resources.begin(), resources.end());
    for(int i = resources.size()-1; i >= 0; --i)
    {
        if (!resources[i].isLoaded)
            file << "(";
        file << resources[i].name << ": " << resources[i].size;
        if (!resources[i].isLoaded)
            file << ")";
        file << std::endl;
    }
}

void TimeProfilerWindow::DumpOgreResourceStatsToFile()
{
    std::ofstream file("ogrestats.txt");
    file << "Ogre Texture Manager:" << std::endl;
    DumpOgreResManagerStatsToFile(Ogre::TextureManager::getSingleton(), file);
    file << std::endl << std::endl << std::endl << "Ogre Mesh Manager:" << std::endl;
    DumpOgreResManagerStatsToFile(Ogre::MeshManager::getSingleton(), file);
    file << std::endl << std::endl << std::endl << "Ogre Material Manager:" << std::endl;
    DumpOgreResManagerStatsToFile(Ogre::MaterialManager::getSingleton(), file);
    file << std::endl << std::endl << std::endl << "Ogre Skeleton Manager:" << std::endl;
    DumpOgreResManagerStatsToFile(Ogre::SkeletonManager::getSingleton(), file);
    file << std::endl << std::endl << std::endl << "Ogre Compositor Manager:" << std::endl;
    DumpOgreResManagerStatsToFile(Ogre::CompositorManager::getSingleton(), file);
    file << std::endl << std::endl << std::endl << "Ogre GPUProgram Manager:" << std::endl;
    DumpOgreResManagerStatsToFile(Ogre::HighLevelGpuProgramManager::getSingleton(), file);
    file << std::endl << std::endl << std::endl << "Ogre Font Manager:" << std::endl;
    DumpOgreResManagerStatsToFile(Ogre::FontManager::getSingleton(), file);
}

void TimeProfilerWindow::DumpSceneComplexityToFile()
{
    if (!text_scenecomplexity_)
        return;
    
    std::ofstream file("scenestats.txt");
    file << text_scenecomplexity_->toPlainText().toStdString();
}

void TimeProfilerWindow::RefreshProfilingData()
{
    if (!visibility_ || !tab_widget_ || tab_widget_->currentIndex() != 0)
        return;

    if (show_profiler_tree_)
        RefreshProfilingDataTree();
    else
        RefreshProfilingDataList();

    QTimer::singleShot(ReadProfilingRefreshInterval(), this, SLOT(RefreshProfilingData()));
}

void TimeProfilerWindow::RefreshProfilingDataTree()
{
#ifdef PROFILING
    using namespace Foundation;

//    Foundation::Profiler *profiler = Foundation::ProfilerSection::GetProfiler();

    Profiler &profiler = framework_->GetProfiler();
    profiler.Lock();
//    ProfilerNodeTree *node = profiler.Lock().get();
    Foundation::ProfilerNodeTree *node = profiler.GetRoot();

    const Foundation::ProfilerNodeTree::NodeList &children = node->GetChildren();
    for(Foundation::ProfilerNodeTree::NodeList::const_iterator iter = children.begin(); iter != children.end(); ++iter)
    {
        node = iter->get();

        const Foundation::ProfilerNode *timings_node = dynamic_cast<const Foundation::ProfilerNode*>(node);
        if (timings_node && timings_node->num_called_ == 0 && !show_unused_)
            continue;

        QTreeWidgetItem *item = FindItemByName(tree_profiling_data_, node->Name().c_str());
        if (!item)
        {
            item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString(node->Name().c_str())));
            tree_profiling_data_->addTopLevelItem(item);
        }

        if (timings_node)
        {
            char str[256] = "-";
            if (timings_node->num_called_custom_ > 0 && timings_node->custom_elapsed_min_ < 1e8)
                sprintf(str, "%.2fms", timings_node->custom_elapsed_min_*1000.f);
            item->setText(2, str);
            if (timings_node->num_called_custom_ > 0)
                sprintf(str, "%.2fms", timings_node->total_custom_*1000.f / timings_node->num_called_custom_);
            item->setText(3, str);
            if (timings_node->num_called_custom_ > 0)
                sprintf(str, "%.2fms", timings_node->custom_elapsed_max_*1000.f);
            item->setText(4, str);
            sprintf(str, "%d", (int)timings_node->num_called_custom_);
            item->setText(1, str);

            timings_node->num_called_custom_ = 0;
            timings_node->total_custom_ = 0;
            timings_node->custom_elapsed_min_ = 1e9;
            timings_node->custom_elapsed_max_ = 0;
        }

        FillProfileTimingWindow(item, node);
    }
    profiler.Release();
#endif
}

void TimeProfilerWindow::CollectProfilerNodes(Foundation::ProfilerNodeTree *node, std::vector<const Foundation::ProfilerNode *> &dst)
{
    const Foundation::ProfilerNodeTree::NodeList &children = node->GetChildren();
    for(Foundation::ProfilerNodeTree::NodeList::const_iterator iter = children.begin(); iter != children.end(); ++iter)
    {
        Foundation::ProfilerNodeTree *child = iter->get();

        const Foundation::ProfilerNode *timings_node = dynamic_cast<const Foundation::ProfilerNode*>(child);
        if (timings_node)
            dst.push_back(timings_node);
        CollectProfilerNodes(child, dst);
    }
}

bool ProfilingNodeLessThan(const Foundation::ProfilerNode *a, const Foundation::ProfilerNode *b)
{
    double aTimePerFrame = (a->num_called_custom_ == 0) ? 0 : (a->total_custom_ / a->num_called_custom_);
    double bTimePerFrame = (b->num_called_custom_ == 0) ? 0 : (b->total_custom_ / b->num_called_custom_);
    return aTimePerFrame > bTimePerFrame;
}

void TimeProfilerWindow::RefreshProfilingDataList()
{
#ifdef PROFILING
    using namespace Foundation;

    Profiler &profiler = framework_->GetProfiler();
    profiler.Lock();

    Foundation::ProfilerNodeTree *root = profiler.GetRoot();
    std::vector<const Foundation::ProfilerNode *> nodes;
    CollectProfilerNodes(root, nodes);
    std::sort(nodes.begin(), nodes.end(), ProfilingNodeLessThan);

    tree_profiling_data_->clear();

    for(std::vector<const Foundation::ProfilerNode *>::iterator iter = nodes.begin(); iter != nodes.end(); ++iter)
    {
        const Foundation::ProfilerNode *timings_node = *iter;
        assert(timings_node);

        if (timings_node->num_called_custom_ == 0 && !show_unused_)
            continue;

        QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString(timings_node->Name().c_str())));
        tree_profiling_data_->addTopLevelItem(item);

        char str[256] = "-";
        if (timings_node->num_called_custom_ > 0 && timings_node->custom_elapsed_min_ < 1e8)
            sprintf(str, "%.2fms", timings_node->custom_elapsed_min_*1000.f);
        item->setText(2, str);
        if (timings_node->num_called_custom_ > 0)
            sprintf(str, "%.2fms", timings_node->total_custom_*1000.f / timings_node->num_called_custom_);
        item->setText(3, str);
        if (timings_node->num_called_custom_ > 0)
            sprintf(str, "%.2fms", timings_node->custom_elapsed_max_*1000.f);
        item->setText(4, str);
        sprintf(str, "%d", (int)timings_node->num_called_custom_);
        item->setText(1, str);

        timings_node->num_called_custom_ = 0;
        timings_node->total_custom_ = 0;
        timings_node->custom_elapsed_min_ = 1e9;
        timings_node->custom_elapsed_max_ = 0;
    }

    profiler.Release();
#endif
}

void RedrawHistoryGraph(const std::vector<double> &data, QLabel *label)
{
    assert(label);

    const QPixmap *pixmap = label->pixmap();
    QImage image = pixmap->toImage();
    QPainter painter(&image);
    painter.fillRect(0, 0, image.width()-1, image.height()-1, QColor(0,0,0));

    const int barWidth = 3;
    const int numElems = min<int>((image.width()+barWidth-1)/barWidth, data.size());

    double maxVal = 0;
    for(int i = 0; i < numElems; ++i)
        maxVal = max(maxVal, data[data.size()-1-i]);

    double bucketSize = 1.0;
    tick_t time = GetCurrentClockTime();
    tick_t modulus = (tick_t)(GetCurrentClockFreq() * bucketSize);
    tick_t modulus2 = (tick_t)(GetCurrentClockFreq() * bucketSize * 2.0);
    QColor colorEven = QColor(0, 0, 0xFF);
    QColor colorOdd = QColor(0xD0, 0xD0, 0xFF);
    if (time % modulus2 >= modulus)
        swap(colorEven, colorOdd);

    for(int i = data.size()-1, x = image.width()-1-barWidth; i >= 0 && x > -barWidth; --i, x -= barWidth)
    {
        double r = 1.0 - min<double>(1.0, data[i] / maxVal);
        int y = (int)((image.height()-1)*r);

        if (y >= image.height()-1)
            continue;

        QColor color = (i % 2 == 0) ? colorEven : colorOdd;
        painter.fillRect(x, y, barWidth, image.height()-y, color);
    }

    label->setPixmap(QPixmap::fromImage(image));
}

void TimeProfilerWindow::RefreshNetworkProfilingData()
{
#ifdef PROFILING
    if (!visibility_ || !tab_widget_ || tab_widget_->currentIndex() != 3)
        return;

    if (world_stream_)
    {
        ProtocolUtilities::NetMessageManager *netMessageManager = world_stream_->GetCurrentProtocolModule()->GetNetworkMessageManager();
        if (!netMessageManager)
            return;

        std::vector<double> dstAccum;
        std::vector<double> dstOccur;
        const size_t numEntries = 256;
        const double bucketSize = 1.0;
        const double smoothingCoeff = 0.8;
        std::vector<double> dataBytesIn;
        std::vector<double> packetsIn;
        std::vector<double> dataBytesOut;
        std::vector<double> packetsOut;

        netMessageManager->receivedDatabytes.OutputBucketedAccumulated(dataBytesIn, numEntries, bucketSize, &dstOccur);
        char str[256];
        double dataInPerSec = EventHistory::SmoothedAvgPerSecond(dataBytesIn, bucketSize, smoothingCoeff);
        sprintf(str, "%s/sec", FormatBytes(dataInPerSec).c_str());
        findChild<QLabel*>("labelDataInPerSec")->setText(str);

        RedrawHistoryGraph(dataBytesIn, findChild<QLabel*>("labelDataInSecGraph"));

        netMessageManager->sentDatabytes.OutputBucketedAccumulated(dataBytesOut, numEntries, bucketSize, &dstOccur);
        double dataOutPerSec = EventHistory::SmoothedAvgPerSecond(dataBytesOut, bucketSize, smoothingCoeff);
        sprintf(str, "%s/sec", FormatBytes(dataOutPerSec).c_str());
        findChild<QLabel*>("labelDataOutPerSec")->setText(str);

        RedrawHistoryGraph(dataBytesOut, findChild<QLabel*>("labelDataOutSecGraph"));

        netMessageManager->receivedDatagrams.OutputBucketedAccumulated(packetsIn, numEntries, bucketSize, &dstOccur);
        double packetsInPerSec = EventHistory::SmoothedAvgPerSecond(packetsIn, bucketSize, smoothingCoeff);
        sprintf(str, "%.2f p/sec", (float)packetsInPerSec);
        findChild<QLabel*>("labelPacketsInPerSec")->setText(str);

        RedrawHistoryGraph(packetsIn, findChild<QLabel*>("labelPacketsInSecGraph"));

        netMessageManager->sentDatagrams.OutputBucketedAccumulated(packetsOut, numEntries, bucketSize, &dstOccur);
        double packetsOutPerSec = EventHistory::SmoothedAvgPerSecond(packetsOut, bucketSize, smoothingCoeff);
        sprintf(str, "%.2f p/sec", (float)packetsOutPerSec);
        findChild<QLabel*>("labelPacketsOutPerSec")->setText(str);

        RedrawHistoryGraph(packetsOut, findChild<QLabel*>("labelPacketsOutSecGraph"));

        netMessageManager->resentPackets.OutputBucketedAccumulated(dstAccum, numEntries, bucketSize, &dstOccur);
        double resentPacketsPerSec = EventHistory::SmoothedAvgPerSecond(dstAccum, bucketSize, smoothingCoeff);
        sprintf(str, "%.2f p/sec", (float)resentPacketsPerSec);
        findChild<QLabel*>("labelPacketResends")->setText(str);

        netMessageManager->lostPackets.OutputBucketedAccumulated(dstAccum, numEntries, bucketSize, &dstOccur);
        double packetLossPerSec = EventHistory::SmoothedAvgPerSecond(dstAccum, bucketSize, smoothingCoeff);
        sprintf(str, "%.2f p/sec", (float)packetLossPerSec);
        findChild<QLabel*>("labelPacketLossIn")->setText(str);

        netMessageManager->duplicatesReceived.OutputBucketedAccumulated(dstAccum, numEntries, bucketSize, &dstOccur);
        double duplicatesRecvPerSec = EventHistory::SmoothedAvgPerSecond(dstAccum, bucketSize, smoothingCoeff);
        sprintf(str, "%.2f p/sec", (float)duplicatesRecvPerSec);
        findChild<QLabel*>("labelDuplicatesIn")->setText(str);

        double avgPacketSizeIn = (packetsInPerSec < 1e-5) ? 0 : (dataInPerSec / packetsInPerSec);
        findChild<QLabel*>("labelAvgPacketSizeIn")->setText(FormatBytes(avgPacketSizeIn).c_str());

        double avgPacketSizeOut = (packetsOutPerSec < 1e-5) ? 0 : (dataOutPerSec / packetsOutPerSec);
        findChild<QLabel*>("labelAvgPacketSizeOut")->setText(FormatBytes(avgPacketSizeOut).c_str());

        sprintf(str, "%.2f ms", (float)netMessageManager->lastRoundTripTime);
        findChild<QLabel*>("labelRoundTripTime")->setText(str);

        sprintf(str, "%.2f ms", (float)netMessageManager->smoothenedRoundTripTime);
        findChild<QLabel*>("labelSmoothenedRoundTripTime")->setText(str);

        sprintf(str, "%.2f ms", (float)netMessageManager->lastHeardSince);
        findChild<QLabel*>("labelLastHeardSince")->setText(str);

        sprintf(str, "%i", netMessageManager->NumUnackedReliablePackets());
        findChild<QLabel*>("labelDataInFlightPackets")->setText(str);

        sprintf(str, "%i", netMessageManager->NumBytesInUnackedReliablePackets());
        findChild<QLabel*>("labelDataInFlightBytes")->setText(str);

        const int ipHeaderSize = 20;
        const int udpHeaderSize = 8;
        const int sludpHeaderSize = 6;

        double dataIn = 0.0;
        double goodDataIn = 0.0;

        for(int i = (int)min(dataBytesIn.size(), packetsIn.size())-1, j = 0; i >= 0 && j < 10; --i, ++j)
        {
            dataIn += dataBytesIn[i] + packetsIn[i] * (ipHeaderSize + udpHeaderSize);
            goodDataIn += dataBytesIn[i] - packetsIn[i] * sludpHeaderSize;
        }

        if (dataIn < 1e-5)
            sprintf(str, "-");
        else
            sprintf(str, "%.1f%%", (float)(goodDataIn * 100.0 / dataIn));
        findChild<QLabel*>("labelDataGoodputIn")->setText(str);

        double dataOut = 0.0;
        double goodDataOut = 0.0;

        for(int i = (int)min(dataBytesOut.size(), packetsOut.size())-1, j = 0; i >= 0 && j < 10; --i, ++j)
        {
            dataOut += dataBytesOut[i] + packetsOut[i] * (ipHeaderSize + udpHeaderSize);
            goodDataOut += dataBytesOut[i] - packetsOut[i] * (sludpHeaderSize);
        }

        if (dataOut < 1e-5)
            sprintf(str, "-");
        else
            sprintf(str, "%.1f%%", (float)(goodDataOut * 100.0 / dataOut));
        findChild<QLabel*>("labelDataGoodputOut")->setText(str);
    }

    QTimer::singleShot(500, this, SLOT(RefreshNetworkProfilingData()));
#endif
}

const char *SimStatsStr(int statID)
{
    static const char *str[31] = 
    {
        "LL_SIM_STAT_TIME_DILATION",
        "LL_SIM_STAT_FPS",
        "LL_SIM_STAT_PHYSFPS",
        "LL_SIM_STAT_AGENTUPS",
        "LL_SIM_STAT_FRAMEMS",
        "LL_SIM_STAT_NETMS",
        "LL_SIM_STAT_SIMOTHERMS",
        "LL_SIM_STAT_SIMPHYSICSMS",
        "LL_SIM_STAT_AGENTMS",
        "LL_SIM_STAT_IMAGESMS",
        "LL_SIM_STAT_SCRIPTMS",
        "LL_SIM_STAT_NUMTASKS",
        "LL_SIM_STAT_NUMTASKSACTIVE",
        "LL_SIM_STAT_NUMAGENTMAIN",
        "LL_SIM_STAT_NUMAGENTCHILD",
        "LL_SIM_STAT_NUMSCRIPTSACTIVE",
        "LL_SIM_STAT_LSLIPS",
        "LL_SIM_STAT_INPPS",
        "LL_SIM_STAT_OUTPPS",
        "LL_SIM_STAT_PENDING_DOWNLOADS",
        "LL_SIM_STAT_PENDING_UPLOADS",
        "LL_SIM_STAT_PENDING_LOCAL_UPLOADS",
        "LL_SIM_STAT_TOTAL_UNACKED_BYTES",
        "LL_SIM_STAT_PHYSICS_PINNED_TASKS",
        "LL_SIM_STAT_PHYSICS_LOD_TASKS",
        "LL_SIM_STAT_SIMPHYSICSSTEPMS",
        "LL_SIM_STAT_SIMPHYSICSSHAPEMS",
        "LL_SIM_STAT_SIMPHYSICSOTHERMS",
        "LL_SIM_STAT_SIMPHYSICSMEMORY",
        "UnknownID"
    };
    if (statID < 0 || statID > 30)
        return str[31];
    return str[statID];
}

void TimeProfilerWindow::RefreshSimStatsData(ProtocolUtilities::NetInMessage *simStats)
{
    simStats->ResetReading();
    int regionX = simStats->ReadU32();
    int regionY = simStats->ReadU32();
    u32 regionFlags = simStats->ReadU32();
    int objectCapacity = simStats->ReadU32();

    tree_sim_stats_->clear();

    label_region_map_coords_->setText(QString("(%1, %2)").arg(regionX).arg(regionY));
    label_region_object_capacity_->setText(QString("%1").arg(objectCapacity));

    size_t numStats = simStats->ReadCurrentBlockInstanceCount();
    for(int i = 0; i < numStats; ++i)
    {
        u32 statID = simStats->ReadU32();
        float statValue = simStats->ReadF32();

        QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(SimStatsStr(statID)));
        item->setText(1, QString("%1").arg(statValue));
        tree_sim_stats_->addTopLevelItem(item);
    }
    int pidStat = simStats->ReadS32();
    label_pid_stat_->setText(QString("%1").arg(pidStat));
}

void TimeProfilerWindow::RefreshAssetProfilingData()
{
    if (!visibility_ || !tab_widget_ || tab_widget_->currentIndex() != 5)
        return;

    tree_asset_cache_->clear();
    tree_asset_transfers_->clear();

    boost::shared_ptr<Foundation::AssetServiceInterface> asset_service = 
        framework_->GetServiceManager()->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();
    if (!asset_service)
        return;
        
    Foundation::AssetCacheInfoMap cache_map = asset_service->GetAssetCacheInfo();
    Foundation::AssetCacheInfoMap::const_iterator i = cache_map.begin();
    while (i != cache_map.end())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0, QStringList());
        tree_asset_cache_->addTopLevelItem(item);
        
        item->setText(0, QString(i->first.c_str()));
        item->setText(1, QString(QString("%1").arg(i->second.count_)));
        item->setText(2, QString(FormatBytes((int)i->second.size_).c_str()));
        
        ++i;
    }

    Foundation::AssetTransferInfoVector transfer_vector = asset_service->GetAssetTransferInfo();
    Foundation::AssetTransferInfoVector::const_iterator j = transfer_vector.begin();
    while (j != transfer_vector.end())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0, QStringList());
        tree_asset_transfers_->addTopLevelItem(item);
        
        item->setText(0, QString((*j).id_.c_str()));
        item->setText(1, QString((*j).type_.c_str()));
        item->setText(2, QString((*j).provider_.c_str()));
        item->setText(3, QString(FormatBytes((int)(*j).size_).c_str()));
        item->setText(4, QString(FormatBytes((int)(*j).received_).c_str()));
        ++j;
    }

    QTimer::singleShot(500, this, SLOT(RefreshAssetProfilingData()));
}

void TimeProfilerWindow::RefreshSceneComplexityProfilingData()
{
    if (!visibility_ || !text_scenecomplexity_ || !tab_widget_ || tab_widget_->currentIndex() != 6)
        return;
    
    Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
    if (!scene)
        return;
    
    boost::shared_ptr<Foundation::RenderServiceInterface> renderer = 
        framework_->GetServiceManager()->GetService<Foundation::RenderServiceInterface>(Foundation::Service::ST_Renderer).lock();
    assert(renderer);
    if (!renderer)
        return;
    
    Ogre::Root *root = Ogre::Root::getSingletonPtr();
    assert(root);
    if (!root)
        return;
    
    Ogre::RenderSystem* rendersys = root->getRenderSystem();
    assert(rendersys);
    if (!rendersys)
        return;
    
    std::ostringstream text;
    
    uint visible_entities = renderer->GetVisibleEntities().size();
    uint batches = 0;
    uint triangles = 0;
    float avgfps = 0.0f;

    // Get overall batchcount/trianglecount/fps data
    Ogre::RenderSystem::RenderTargetIterator rtiter = rendersys->getRenderTargetIterator();
    while (rtiter.hasMoreElements())
    {
        Ogre::RenderTarget* target = rtiter.getNext();
        // Sum batches & triangles from all rendertargets updated last frame
        batches += target->getBatchCount();
        triangles += target->getTriangleCount();
        // Get FPS only from the primary
        if (target->isPrimary())
            avgfps = floor(target->getAverageFPS() * 100.0f) / 100.0f;
    }
    
    text << "Viewport" << std::endl;
    text << "# of currently visible entities: " << visible_entities << std::endl;
    text << "# of total batches rendered last frame: " << batches << std::endl;
    text << "# of total triangles rendered last frame: " << triangles << std::endl;
    text << "# of avg. triangles per batch: " << triangles / (batches ? batches : 1) << std::endl;
    text << "Avg. FPS: " << avgfps << std::endl;
    text << std::endl;
    
    uint entities = 0;
    uint prims = 0;
    uint invisible_prims = 0;
    uint meshentities = 0;
    uint animated = 0;
    
    std::set<Ogre::Mesh*> all_meshes;
    std::set<Ogre::Mesh*> scene_meshes;
    std::set<Ogre::Mesh*> other_meshes;
    std::set<Ogre::Texture*> all_textures;
    std::set<Ogre::Texture*> scene_textures;
    std::set<Ogre::Texture*> other_textures;
    uint scene_meshes_size = 0;
    uint other_meshes_size = 0;
    uint mesh_vertices = 0;
    uint mesh_triangles = 0;
    uint mesh_instance_vertices = 0;
    uint mesh_instance_triangles = 0;
    uint mesh_instances = 0;
    
    // Loop through entities to see mesh usage
    for (Scene::SceneManager::iterator iter = scene->begin(); iter != scene->end(); ++iter)
    {
        Scene::Entity &entity = **iter;
        entities++;
        EC_OpenSimPrim* prim = entity.GetComponent<EC_OpenSimPrim>().get();
        Environment::EC_Terrain* terrain = entity.GetComponent<Environment::EC_Terrain>().get();
        OgreRenderer::EC_Mesh* mesh = entity.GetComponent<OgreRenderer::EC_Mesh>().get();
        OgreRenderer::EC_OgreCustomObject* custom = entity.GetComponent<OgreRenderer::EC_OgreCustomObject>().get();
        Ogre::Entity* ogre_entity = 0;
        
        // Get Ogre mesh from mesh EC
        if (mesh)
        {
            ogre_entity = mesh->GetEntity();
            if (ogre_entity)
            {
                Ogre::Mesh* ogre_mesh = ogre_entity->getMesh().get();
                scene_meshes.insert(ogre_mesh);
                GetVerticesAndTrianglesFromMesh(ogre_mesh, mesh_instance_vertices, mesh_instance_triangles);
                mesh_instances++;
                std::set<Ogre::Material*> temp_mat;
                GetMaterialsFromEntity(ogre_entity, temp_mat);
                GetTexturesFromMaterials(temp_mat, scene_textures);
            }
        }
        // Get Ogre mesh from customobject EC
        else if (custom)
        {
            ogre_entity = custom->GetEntity();
            if (ogre_entity)
            {
                Ogre::Mesh* ogre_mesh = ogre_entity->getMesh().get();
                scene_meshes.insert(ogre_mesh);
                GetVerticesAndTrianglesFromMesh(ogre_mesh, mesh_instance_vertices, mesh_instance_triangles);
                mesh_instances++;
                std::set<Ogre::Material*> temp_mat;
                GetMaterialsFromEntity(ogre_entity, temp_mat);
                GetTexturesFromMaterials(temp_mat, scene_textures);
            }
        }
        // Get Ogre meshes from terrain EC
        else if (terrain)
        {
            for (int y = 0; y < terrain->PatchHeight(); ++y)
                for(int x = 0; x < terrain->PatchWidth(); ++x)
                {
                    Ogre::SceneNode *node = terrain->GetPatch(x, y).node;
                    if (!node)
                        continue;
                    if (!node->numAttachedObjects())
                        continue;
                    ogre_entity = dynamic_cast<Ogre::Entity*>(node->getAttachedObject(0));
                    if (ogre_entity)
                    {
                        Ogre::Mesh* ogre_mesh = ogre_entity->getMesh().get();
                        scene_meshes.insert(ogre_mesh);
                        GetVerticesAndTrianglesFromMesh(ogre_mesh, mesh_instance_vertices, mesh_instance_triangles);
                        mesh_instances++;
                        std::set<Ogre::Material*> temp_mat;
                        GetMaterialsFromEntity(ogre_entity, temp_mat);
                        GetTexturesFromMaterials(temp_mat, scene_textures);
                    }
                }
        }
        
        // Check drawtype for prims
        if (prim)
        {
            
            int drawtype = prim->getDrawType();
            if ((drawtype == RexTypes::DRAWTYPE_MESH) && (ogre_entity))
                meshentities++;
            if (drawtype == RexTypes::DRAWTYPE_PRIM)
            {
                if (ogre_entity)
                    prims++;
                else
                    invisible_prims++;
            }
        }
        else
        {
            if (ogre_entity)
                meshentities++;
        }
        if (entity.GetComponent("EC_OgreAnimationController").get())
            animated++;
    }
    
    // Go through all meshes and see which of them are in the scene
    Ogre::ResourceManager::ResourceMapIterator iter = Ogre::MeshManager::getSingleton().getResourceIterator();
    while (iter.hasMoreElements())
    {
        Ogre::ResourcePtr resource = iter.getNext();
        if (!resource->isLoaded())
            continue;
        Ogre::Mesh* mesh = dynamic_cast<Ogre::Mesh*>(resource.get());
        if (mesh)
            all_meshes.insert(mesh);
        if (scene_meshes.find(mesh) == scene_meshes.end())
            other_meshes.insert(mesh);
    }
    
    text << "Scene" << std::endl;
    text << "# of entities in the scene: " << entities << std::endl;
    text << "# of prims with geometry in the scene: " << prims << std::endl;
    text << "# of invisible prims in the scene: " << invisible_prims << std::endl;
    text << "# of mesh entities in the scene: " << meshentities << std::endl;
    text << "# of animated entities in the scene: " << animated << std::endl;
    text << std::endl;
    
    // Count total vertices/triangles per mesh
    std::set<Ogre::Mesh*>::iterator mi = all_meshes.begin();
    while (mi != all_meshes.end())
    {
        Ogre::Mesh* mesh = *mi;
        GetVerticesAndTrianglesFromMesh(mesh, mesh_vertices, mesh_triangles);
        ++mi;
    }
    // Count scene/other mesh byte sizes
    mi = scene_meshes.begin();
    while (mi != scene_meshes.end())
    {
        scene_meshes_size += (*mi)->getSize();
        ++mi;
    }
    mi = other_meshes.begin();
    while (mi != other_meshes.end())
    {
        other_meshes_size += (*mi)->getSize();
        ++mi;
    }
    
    text << "Ogre Meshes" << std::endl;
    text << "# of loaded meshes: " << all_meshes.size() << std::endl;
    text << "# of unique meshes in scene: " << scene_meshes.size() << std::endl;
    text << "# of mesh instances in scene: " << mesh_instances << std::endl;
    text << "Total mesh data size: (" << scene_meshes_size / 1024 << " KBytes scene)+(" << other_meshes_size / 1024 << " KBytes other)" << std::endl;
    text << "# of vertices in the meshes: " << mesh_vertices << std::endl;
    text << "# of triangles in the meshes: " << mesh_triangles << std::endl;
    text << "# of vertices in the scene: " << mesh_instance_vertices << std::endl;
    text << "# of triangles in the scene: " << mesh_instance_triangles << std::endl;
    text << "# of avg. triangles in the scene per mesh: " << mesh_instance_triangles / (mesh_instances ? mesh_instances : 1) << std::endl;
    text << std::endl;
    
    // Go through all textures and see which of them are in the scene
    Ogre::ResourceManager::ResourceMapIterator tex_iter = ((Ogre::ResourceManager*)Ogre::TextureManager::getSingletonPtr())->getResourceIterator();
    while (tex_iter.hasMoreElements())
    {
        Ogre::ResourcePtr resource = tex_iter.getNext();
        if (!resource->isLoaded())
            continue;
        Ogre::Texture* texture = dynamic_cast<Ogre::Texture*>(resource.get());
        if (texture)
        {
            all_textures.insert(texture);
            if (scene_textures.find(texture) == scene_textures.end())
                other_textures.insert(texture);
        }
    }
    
    // Count total/scene texture byte sizes and amount of total pixels
    uint scene_tex_size = 0;
    uint total_tex_size = 0;
    uint other_tex_size = 0;
    uint scene_tex_pixels = 0;
    uint total_tex_pixels = 0;
    uint other_tex_pixels = 0;
    std::set<Ogre::Texture*>::iterator ti = scene_textures.begin();
    while (ti != scene_textures.end())
    {
        scene_tex_size += (*ti)->getSize();
        scene_tex_pixels += (*ti)->getWidth() * (*ti)->getHeight();
        ++ti;
    }
    ti = all_textures.begin();
    while (ti != all_textures.end())
    {
        total_tex_size += (*ti)->getSize();
        total_tex_pixels += (*ti)->getWidth() * (*ti)->getHeight();
        ++ti;
    }
    ti = other_textures.begin();
    while (ti != other_textures.end())
    {
        other_tex_size += (*ti)->getSize();
        other_tex_pixels += (*ti)->getWidth() * (*ti)->getHeight();
        ++ti;
    }
    
    // Sort textures into categories
    uint scene_tex_categories[5];
    uint other_tex_categories[5];
    for (uint i = 0; i < 5; ++i)
    {
        scene_tex_categories[i] = 0;
        other_tex_categories[i] = 0;
    }
    
    ti = scene_textures.begin();
    while (ti != scene_textures.end())
    {
        uint dimension = max((*ti)->getWidth(), (*ti)->getHeight());
        if (dimension > 2048)
            scene_tex_categories[0]++;
        else if (dimension > 1024)
            scene_tex_categories[1]++;
        else if (dimension > 512)
            scene_tex_categories[2]++;
        else if (dimension > 256)
            scene_tex_categories[3]++;
        else
            scene_tex_categories[4]++;
        ++ti;
    }
    
    ti = other_textures.begin();
    while (ti != other_textures.end())
    {
        uint dimension = max((*ti)->getWidth(), (*ti)->getHeight());
        if (dimension > 2048)
            other_tex_categories[0]++;
        else if (dimension > 1024)
            other_tex_categories[1]++;
        else if (dimension > 512)
            other_tex_categories[2]++;
        else if (dimension > 256)
            other_tex_categories[3]++;
        else
            other_tex_categories[4]++;
        ++ti;
    }
    
    if (!total_tex_pixels)
        total_tex_pixels = 1;
    if (!scene_tex_pixels)
        scene_tex_pixels = 1;
    if (!other_tex_pixels)
        other_tex_pixels = 1;
    
    text << "Ogre Textures" << std::endl;
    text << "# of loaded textures: " << all_textures.size() << std::endl;
    text << "Texture data size in scene: " << scene_tex_size / 1024 << " KBytes" << std::endl;
    text << "Texture data size total: " << total_tex_size / 1024 << " KBytes" << std::endl;
    text << "Texture dimensions: " << std::endl;
    text << "> 2048: (" << scene_tex_categories[0] << " scene)+(" << other_tex_categories[0] << " other)" << std::endl;
    text << "> 1024: (" << scene_tex_categories[1] << " scene)+(" << other_tex_categories[1] << " other)" << std::endl;
    text << "> 512: (" << scene_tex_categories[2] << " scene)+(" << other_tex_categories[2] << " other)" << std::endl;
    text << "> 256: (" << scene_tex_categories[3] << " scene)+(" << other_tex_categories[3] << " other)" << std::endl;
    text << "smaller: (" << scene_tex_categories[4] << " scene)+(" << other_tex_categories[4] << " other)" << std::endl;
    text << "Avg. bytes/pixel: (" << floor(((float)scene_tex_size / scene_tex_pixels) * 100.0f) / 100.f
                   << " scene)/(" << floor(((float)total_tex_size / total_tex_pixels) * 100.0f) / 100.f 
                   << " total)" << std::endl;
    text << std::endl;
    
    text << "Ogre Materials" << std::endl;
    text << "# of materials total: " << GetNumResources(Ogre::MaterialManager::getSingleton()) << std::endl;
    
    uint vertex_shaders = 0;
    uint pixel_shaders = 0;
    Ogre::ResourceManager::ResourceMapIterator shader_iter = ((Ogre::ResourceManager*)Ogre::HighLevelGpuProgramManager::getSingletonPtr())->getResourceIterator();
    while(shader_iter.hasMoreElements())
    {
        Ogre::ResourcePtr resource = shader_iter.getNext();
        // Count only loaded programs
        if (!resource->isLoaded())
            continue;
        Ogre::HighLevelGpuProgram* program = dynamic_cast<Ogre::HighLevelGpuProgram*>(resource.get());
        if (program)
        {
            Ogre::GpuProgramType type = program->getType();
            if (type == Ogre::GPT_VERTEX_PROGRAM)
                vertex_shaders++;
            if (type == Ogre::GPT_FRAGMENT_PROGRAM)
                pixel_shaders++;
        }
    }
    text << "# of vertex shaders total: " << vertex_shaders << std::endl;
    text << "# of pixel shaders total: " << pixel_shaders << std::endl;
    text << std::endl;
    
    text << "# of loaded skeletons: " << GetNumResources(Ogre::SkeletonManager::getSingleton()) << std::endl;
    text << std::endl;
    
    // Update only if no selection
    QTextCursor cursor(text_scenecomplexity_->textCursor());
    bool text_selected = cursor.hasSelection();
    if (!text_selected)
    {
        // Save previous pos, update & restore pos
        int cursorPos = cursor.position();
        int anchorPos = cursor.anchor();
        int scrollPos = 0;
        if (text_scenecomplexity_->verticalScrollBar())
            scrollPos = text_scenecomplexity_->verticalScrollBar()->value();
        
        text_scenecomplexity_->setPlainText(text.str().c_str());
        
        cursor.setPosition(anchorPos);
        cursor.setPosition(cursorPos, QTextCursor::KeepAnchor);
        text_scenecomplexity_->setTextCursor(cursor);
        if (text_scenecomplexity_->verticalScrollBar())
            text_scenecomplexity_->verticalScrollBar()->setValue(scrollPos);
    }
    
    QTimer::singleShot(500, this, SLOT(RefreshSceneComplexityProfilingData()));
}

void TimeProfilerWindow::GetVerticesAndTrianglesFromMesh(Ogre::Mesh* mesh, uint& vertices, uint& triangles)
{
    // Count total vertices/triangles for each mesh instance
    for (uint i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);
        if (submesh)
        {
            Ogre::VertexData* vtx = submesh->vertexData;
            Ogre::IndexData* idx = submesh->indexData;
            if (vtx)
                vertices += vtx->vertexCount;
            if (idx)
                triangles += idx->indexCount / 3;
        }
    }
}

void TimeProfilerWindow::GetMaterialsFromEntity(Ogre::Entity* entity, std::set<Ogre::Material*>& dest)
{
    for (uint i = 0; i < entity->getNumSubEntities(); ++i)
    {
        Ogre::SubEntity* subentity = entity->getSubEntity(i);
        if (subentity)
        {
            Ogre::Material* mat = subentity->getMaterial().get();
            if (mat)
                dest.insert(mat);
        }
    }
}

void TimeProfilerWindow::GetTexturesFromMaterials(const std::set<Ogre::Material*>& materials, std::set<Ogre::Texture*>& dest)
{
    Ogre::TextureManager& texMgr = Ogre::TextureManager::getSingleton();
    
    std::set<Ogre::Material*>::const_iterator i = materials.begin();
    while (i != materials.end())
    {
        Ogre::Material::TechniqueIterator iter = (*i)->getTechniqueIterator();
        while(iter.hasMoreElements())
        {
            Ogre::Technique *tech = iter.getNext();
            assert(tech);
            Ogre::Technique::PassIterator passIter = tech->getPassIterator();
            while(passIter.hasMoreElements())
            {
                Ogre::Pass *pass = passIter.getNext();
                
                Ogre::Pass::TextureUnitStateIterator texIter = pass->getTextureUnitStateIterator();
                while(texIter.hasMoreElements())
                {
                    Ogre::TextureUnitState *texUnit = texIter.getNext();
                    std::string texName = texUnit->getTextureName();
                    Ogre::Texture* tex = dynamic_cast<Ogre::Texture*>(texMgr.getByName(texName).get());
                    if ((tex) && (tex->isLoaded()))
                        dest.insert(tex);
                }
            }
        }
        ++i;
    }
}

void TimeProfilerWindow::RefreshRenderTargetProfilingData()
{
    if (!visibility_ || !text_scenecomplexity_ || !tab_widget_ || tab_widget_->currentIndex() != 7)
        return;
    
    tree_rendertargets_->clear();
    
    Ogre::Root *root = Ogre::Root::getSingletonPtr();
    assert(root);
    if (!root)
        return;
    Ogre::RenderSystem* rendersys = root->getRenderSystem();
    assert(rendersys);
    if (!rendersys)
        return;
    Ogre::RenderSystem::RenderTargetIterator iter = rendersys->getRenderTargetIterator();
    while (iter.hasMoreElements())
    {
        Ogre::RenderTarget* target = iter.getNext();
        QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0, QStringList());
        tree_rendertargets_->addTopLevelItem(item);
        
        item->setText(0, QString(target->getName().c_str()));
        item->setText(1, QString("%1").arg(target->getWidth()));
        item->setText(2, QString("%1").arg(target->getHeight()));
        item->setText(3, QString("%1").arg(target->getColourDepth()));
        item->setText(4, QString("%1").arg(target->getNumViewports()));
        item->setText(5, QString("%1").arg(target->getBatchCount()));
        item->setText(6, QString("%1").arg(target->getTriangleCount()));
        item->setText(7, QString("%1").arg(target->getLastFPS(), 0, 'f', 2));
        item->setText(8, QString("%1").arg(target->getAverageFPS(), 0, 'f', 2));
        item->setText(9, QString("%1").arg(target->getBestFPS(), 0, 'f', 2));
        item->setText(10, QString("%1").arg(target->getWorstFPS(), 0, 'f', 2));
        item->setText(11, QString("%1ms").arg(target->getBestFrameTime(), 0, 'f', 2));
        item->setText(12, QString("%1ms").arg(target->getWorstFrameTime(), 0, 'f', 2));
        
        std::ostringstream details;
        details << "Priority " << (int)target->getPriority()
                << " - Active " << (target->isActive() ? "true" : "false")
                << " - AutoUpdated " << (target->isAutoUpdated() ? "true" : "false")
                << " - Primary " << (target->isPrimary() ? "true" : "false")
                << " - HWGamma " << (target->isHardwareGammaEnabled() ? "true" : "false")
                << " - TextureFlip " << (target->requiresTextureFlipping() ? "true" : "false")
                << " - FSAALevel " << target->getFSAA()
                << " - SuggestedPixelFormat " << Ogre::PixelUtil::getFormatName(target->suggestPixelFormat());
        item->setText(13, QString(details.str().c_str()));
    }
    
    QTimer::singleShot(500, this, SLOT(RefreshRenderTargetProfilingData()));
}


void TimeProfilerWindow::RefreshTextureProfilingData()
{
    if (!visibility_ || !tab_widget_ || tab_widget_->currentIndex() != 8)
        return;

    Ogre::ResourceManager::ResourceMapIterator iter = Ogre::TextureManager::getSingleton().getResourceIterator();

    while(iter.hasMoreElements())
    {
        Ogre::ResourcePtr resource = iter.getNext();
        // Is there already this kind of element? 
        QTreeWidgetItem *item = FindItemByName(tree_texture_assets_, resource->getName().c_str());
        if (item == 0) 
            item = new QTreeWidgetItem(tree_texture_assets_);

        FillItem(item, resource);
    }
}

/// Derive the tree widget item to implement a custom sort predicate that sorts certain columns by numbers. 
class OgreAssetTreeWidgetItem : public QTreeWidgetItem
{
public:
    OgreAssetTreeWidgetItem(QTreeWidget *parent)
    :QTreeWidgetItem(parent)
    {
    }

    bool operator<(const QTreeWidgetItem &rhs) const
    {
        switch(treeWidget()->sortColumn())
        {
        case 1: // Sort the number columns as numbers. 'Size'
        case 10: // 'Loading State'
        case 11: // 'State Count'
            return this->text(treeWidget()->sortColumn()).toInt() > rhs.text(treeWidget()->sortColumn()).toInt();
        default: // Others as text.
            return this->text(treeWidget()->sortColumn()) < rhs.text(treeWidget()->sortColumn());
        }
    }
};

void TimeProfilerWindow::RefreshAssetData(Ogre::ResourceManager& manager, QTreeWidget* widget)
{
    Ogre::ResourceManager::ResourceMapIterator iter = manager.getResourceIterator();
    while(iter.hasMoreElements())
    {
        Ogre::ResourcePtr resource = iter.getNext();
        // Is there already this kind of element? 
        QTreeWidgetItem *item = FindItemByName(widget, resource->getName().c_str());
        if (item == 0) 
            item = new OgreAssetTreeWidgetItem(widget);

        FillItem(item, resource);
    }
    for(int i = 0; i < widget->columnCount(); ++i)
        widget->resizeColumnToContents(i);
}

void TimeProfilerWindow::FillItem(QTreeWidgetItem* item, const Ogre::ResourcePtr& resource)
{
    if (item == 0)
        return;

    item->setText(0,resource->getName().c_str());
    QString size;
    size.setNum(resource->getSize());
    item->setText(1,size);

    item->setText(2, resource->isManuallyLoaded() ? "Yes" : "No");
    item->setText(3, resource->isReloadable() ? "Yes" : "No");
    item->setText(4, resource->getGroup().c_str());
    item->setText(5, resource->getOrigin().c_str());
    item->setText(6, resource->isLoaded()  ? "Yes" : "No");
    item->setText(7, resource->isLoading()  ? "Yes" : "No");
    item->setText(8, resource->isBackgroundLoaded() ? "Yes" : "No");
    item->setText(9, resource->isPrepared()  ? "Yes" : "No");

    QString tmp;
    tmp.setNum(resource->getLoadingState());
    item->setText(10,tmp);

    tmp.clear();
    tmp.setNum(resource->getStateCount());
    item->setText(11,tmp);
}

}
