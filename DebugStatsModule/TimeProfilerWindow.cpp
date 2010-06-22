// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "TimeProfilerWindow.h"
#include "Profiler.h"
#include "DebugStats.h"
#include "HighPerfClock.h"
#include "Framework.h"
#include "NetworkMessages/NetInMessage.h"
#include "NetworkMessages/NetMessageManager.h"
#include "AssetServiceInterface.h"
#include "WorldStream.h"

#include <utility>

#include <QVBoxLayout>
#include <QTreeWidget>
#include <QUiLoader>
#include <QFile>
#include <QHeaderView>
#include <QPainter>
#include <QTreeWidgetItemIterator>
#include <QtAlgorithms>


#include <OgreFontManager.h>

#include "MemoryLeakCheck.h"

using namespace std;

namespace DebugStats
{

TimeProfilerWindow::TimeProfilerWindow(Foundation::Framework *fw) : framework_(fw)
{
    QUiLoader loader;
    QFile file("./data/ui/profiler.ui");
    file.open(QFile::ReadOnly);
    contents_widget_ = loader.load(&file, this);
    assert(contents_widget_);
    file.close();

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

    QObject::connect(tab_widget_, SIGNAL(currentChanged(int)), this, SLOT(OnProfilerWindowTabChanged(int)));

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
    {
        if ( buttons[i]->objectName().contains("arrange") )
            QObject::connect(buttons[i], SIGNAL(clicked()), this, SLOT(Arrange()));

    }
   
    QObject::connect(push_button_toggle_tree_, SIGNAL(pressed()), this, SLOT(ToggleTreeButtonPressed()));
    QObject::connect(push_button_collapse_all_, SIGNAL(pressed()), this, SLOT(CollapseAllButtonPressed()));
    QObject::connect(push_button_expand_all_, SIGNAL(pressed()), this, SLOT(ExpandAllButtonPressed()));
    QObject::connect(push_button_show_unused_, SIGNAL(pressed()), this, SLOT(ShowUnusedButtonPressed()));

    QObject::connect(findChild<QPushButton*>("pushButtonDumpOgreStats"), SIGNAL(pressed()), this, SLOT(DumpOgreResourceStatsToFile()));

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
        QObject::connect(apply, SIGNAL(clicked()), this, SLOT(ChangeLoggerThreshold()));
    }
}

void TimeProfilerWindow::ChangeLoggerThreshold()
{
     QDoubleSpinBox* box = findChild<QDoubleSpinBox* >("loggerSpinbox");
     if ( box != 0)
        logThreshold_ = box->value();

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
        case 7:  // Texture
            widget =  tree_texture_assets_;
            break;
        case 8: // Mesh
            widget =  tree_mesh_assets_;
            break;
        case 9: // Material
            widget =  tree_material_assets_;
            break;
        case 10:
            widget =  tree_skeleton_assets_;
            break;
        case 11:
            widget = tree_compositor_assets_ ;
            break;
        case 12:
            widget = tree_gpu_assets_;
            break;
        case 13:
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
        RefreshRenderTargetProfilingData();
        break;
    case 7: // Textures
        RefressAssetData(Ogre::TextureManager::getSingleton(), tree_texture_assets_ );
        break;
    case 8: // Meshes
        RefressAssetData(Ogre::MeshManager::getSingleton(), tree_mesh_assets_ );
        break;
    case 9: // Material
        RefressAssetData(Ogre::MaterialManager::getSingleton(), tree_material_assets_);
        break;
    case 10: // Skeleton
        RefressAssetData(Ogre::SkeletonManager::getSingleton(), tree_skeleton_assets_);
        break;
    case 11: // Composition
        RefressAssetData(Ogre::CompositorManager::getSingleton(), tree_compositor_assets_);
        break;
    case 12: // Gpu assets
        RefressAssetData(Ogre::HighLevelGpuProgramManager::getSingleton(), tree_gpu_assets_);
        break;
    case 13: // Font assets
        RefressAssetData(Ogre::FontManager::getSingleton(), tree_font_assets_);
        break;

    }
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
    current_world_stream_ = worldStream;
}

void TimeProfilerWindow::FillProfileTimingWindow(QTreeWidgetItem *qtNode, const Foundation::ProfilerNodeTree *profilerNode)
{
    const Foundation::ProfilerNodeTree::NodeList &children = profilerNode->GetChildren();

    for(Foundation::ProfilerNodeTree::NodeList::const_iterator iter = children.begin(); iter != children.end(); ++iter)
    {
        Foundation::ProfilerNodeTree *node = iter->get();

        const Foundation::ProfilerNode *timings_node = dynamic_cast<const Foundation::ProfilerNode*>(node);
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
        double age = (double)frameTimes[firstEntry + i].first / freq.QuadPart;// Foundation::ProfilerBlock::ElapsedTimeSeconds(*(LARGE_INTEGER*)&frameTimes[firstEntry + i].first, now);
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
     QPushButton* button = findChild<QPushButton* >("loggerApply");
     
     if ( button == 0)
         return;

     if ( tab_widget_->currentIndex() != 1 && button->isChecked() )
     {
        button->setChecked(false);
        return;
     }
    
     if ( tab_widget_->currentIndex() == 1 && !button->isChecked())
        return;
    

    using namespace Foundation;

    QFile file("performancelogger.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
         return;

 
    QTextStream out(&file);
    

//   Foundation::Profiler *profiler = Foundation::ProfilerSection::GetProfiler();

    Profiler &profiler = framework_->GetProfiler();
    profiler.Lock();
//    ProfilerNodeTree *node = profiler.Lock().get();
    Foundation::ProfilerNodeTree *node = profiler.GetRoot();

    const Foundation::ProfilerNodeTree::NodeList &children = node->GetChildren();

    for(Foundation::ProfilerNodeTree::NodeList::const_iterator iter = children.begin(); iter != children.end(); ++iter)
    {
        node = iter->get();

        const Foundation::ProfilerNode *timings_node = dynamic_cast<const Foundation::ProfilerNode*>(node);
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
    

}

void TimeProfilerWindow::FillThresholdLogger(QTextStream& out, const Foundation::ProfilerNodeTree *profilerNode)
{
    const Foundation::ProfilerNodeTree::NodeList &children = profilerNode->GetChildren();

    for(Foundation::ProfilerNodeTree::NodeList::const_iterator iter = children.begin(); iter != children.end(); ++iter)
    {
        Foundation::ProfilerNodeTree *node = iter->get();

        const Foundation::ProfilerNode *timings_node = dynamic_cast<const Foundation::ProfilerNode*>(node);
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
    if (!tab_widget_ || tab_widget_->currentIndex() != 2)
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



void TimeProfilerWindow::RefreshProfilingData()
{
    if (!tab_widget_ || tab_widget_->currentIndex() != 0)
        return;

    if (show_profiler_tree_)
        RefreshProfilingDataTree();
    else
        RefreshProfilingDataList();

    QTimer::singleShot(ReadProfilingRefreshInterval(), this, SLOT(RefreshProfilingData()));
}

void TimeProfilerWindow::RefreshProfilingDataTree()
{
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
    Core::tick_t time = Core::GetCurrentClockTime();
    Core::tick_t modulus = (Core::tick_t)(Core::GetCurrentClockFreq() * bucketSize);
    Core::tick_t modulus2 = (Core::tick_t)(Core::GetCurrentClockFreq() * bucketSize * 2.0);
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
    if (!tab_widget_ || tab_widget_->currentIndex() != 3)
        return;

    if (current_world_stream_)
    {
        ProtocolUtilities::NetMessageManager *netMessageManager = current_world_stream_->GetCurrentProtocolModule()->GetNetworkMessageManager();
        assert(netMessageManager);
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
    if (!tab_widget_ || tab_widget_->currentIndex() != 5)
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

void TimeProfilerWindow::RefreshRenderTargetProfilingData()
{
    if (!tree_rendertargets_)
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


 void TimeProfilerWindow::RefressTextureProfilingData()
 {
     if (!tab_widget_ || tab_widget_->currentIndex() != 7)
        return;

     Ogre::ResourceManager::ResourceMapIterator iter = Ogre::TextureManager::getSingleton().getResourceIterator();
    
     while ( iter.hasMoreElements() )
     {
       
       Ogre::ResourcePtr resource = iter.getNext();

       // Is there allready this kind element? 
       QTreeWidgetItem *item = FindItemByName(tree_texture_assets_, resource->getName().c_str());
       
       if ( item == 0) 
        item = new QTreeWidgetItem(tree_texture_assets_);
       
       FillItem(item, resource);

     }



 }

 void TimeProfilerWindow::RefressAssetData(Ogre::ResourceManager& manager, QTreeWidget* widget)
 {
   
     Ogre::ResourceManager::ResourceMapIterator iter = manager.getResourceIterator();
     
     while ( iter.hasMoreElements() )
     {
       
       Ogre::ResourcePtr resource = iter.getNext();

       // Is there allready this kind element? 
       QTreeWidgetItem *item = FindItemByName(widget, resource->getName().c_str());
       
       if ( item == 0) 
        item = new QTreeWidgetItem(widget);
       
       FillItem(item, resource);

     }

 }

 
 void TimeProfilerWindow::FillItem(QTreeWidgetItem* item, const Ogre::ResourcePtr& resource)
 {
       if ( item == 0)
           return;

       item->setText(0,resource->getName().c_str());
       QString size;
       size.setNum(resource->getSize());
       item->setText(1,size);

        
       item->setText(2, resource->isReloadable() ? "Yes" : "False" );
       item->setText(3, resource->isManuallyLoaded() ? "Yes" : "False");
       item->setText(4, resource->isPrepared()  ? "Yes" : "False");
       item->setText(5, resource->isLoaded()  ? "Yes" : "False");
       item->setText(6, resource->isLoading()  ? "Yes" : "False");
       
       QString tmp;
       tmp.setNum(resource->getLoadingState());
       item->setText(7,tmp);

       item->setText(8, resource->isBackgroundLoaded() ? "Yes" : "False");
       item->setText(9, resource->getGroup().c_str());
       item->setText(10, resource->getOrigin().c_str());
       tmp.clear();
       tmp.setNum(resource->getStateCount());
       item->setText(11,tmp);


 }

}
