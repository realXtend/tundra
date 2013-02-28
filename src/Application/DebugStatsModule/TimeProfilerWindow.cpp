// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "TimeProfilerWindow.h"

#include "Profiler.h"
#include "HighPerfClock.h"
#include "Framework.h"
#include "FrameAPI.h"
#include "Application.h"
#include "Scene/Scene.h"
#include "OgreRenderingModule.h"
#include "EC_Mesh.h"
#include "EC_OgreCustomObject.h"
#include "EC_Terrain.h"
#include "UiAPI.h"
#include "UiMainWindow.h"
#include "SceneAPI.h"
#include "Entity.h"
#include "Renderer.h"
#include "OgreWorld.h"
#include "AssetAPI.h"
#include "LoggingFunctions.h"
#include "EC_RigidBody.h"
#include "PhysicsModule.h"
#include "PhysicsWorld.h"
#include "IAsset.h"

#ifdef EC_Script_ENABLED
#include "IScriptInstance.h"
#include "EC_Script.h"
#endif

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
#include <QTableWidget>
#include <QMenu>

#include <btBulletDynamicsCommon.h>

#include <OgreFontManager.h>

#include <kNet/Network.h>

#include "MemoryLeakCheck.h"

using namespace std;

const QString DEFAULT_LOG_DIR("logs");

TimeProfilerWindow::TimeProfilerWindow(Framework *fw, QWidget *parent) :
    QWidget(parent),
    framework_(fw)
{
    QUiLoader loader;
    QFile file(Application::InstallationDirectory() + "data/ui/profiler.ui");
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
    treeBulletStats = findChild<QTreeWidget*>("treeBulletStats");
    combo_timing_refresh_interval_ = findChild<QComboBox*>("comboTimingRefreshInterval");
    tab_widget_ = findChild<QTabWidget*>("tabWidget");
    label_frame_time_history_ = findChild<QLabel*>("labelFrameTimeHistory");
    label_top_frame_time_ = findChild<QLabel*>("labelTopFrameTime");
    label_time_per_frame_ = findChild<QLabel*>("labelTimePerFrame");
    labelTimings = findChild<QLabel*>("labelTimings");
    assert(tab_widget_);
    assert(tree_profiling_data_);
    assert(combo_timing_refresh_interval_);
    assert(label_frame_time_history_);
    assert(label_top_frame_time_);

    // Create a QImage object and set it in label.
    QImage frameTimeHistory(label_frame_time_history_->width(), label_frame_time_history_->height(), QImage::Format_RGB32);
    frameTimeHistory.fill(0xFF000000);
    label_frame_time_history_->setPixmap(QPixmap::fromImage(frameTimeHistory));

    tree_profiling_data_->header()->resizeSection(0, 300);
    tree_profiling_data_->header()->resizeSection(1, 60);
    tree_profiling_data_->header()->resizeSection(2, 50);
    tree_profiling_data_->header()->resizeSection(3, 50);
    tree_profiling_data_->header()->resizeSection(4, 50);

    connect(tab_widget_, SIGNAL(currentChanged(int)), this, SLOT(OnProfilerWindowTabChanged(int)));

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
    for(int i = 0; i < buttons.size(); ++i )
        if (buttons[i]->objectName().contains("arrange") )
            connect(buttons[i], SIGNAL(clicked()), this, SLOT(Arrange()));

    connect(push_button_toggle_tree_, SIGNAL(pressed()), this, SLOT(ToggleTreeButtonPressed()));
    connect(push_button_collapse_all_, SIGNAL(pressed()), this, SLOT(CollapseAllButtonPressed()));
    connect(push_button_expand_all_, SIGNAL(pressed()), this, SLOT(ExpandAllButtonPressed()));
    connect(push_button_show_unused_, SIGNAL(pressed()), this, SLOT(ShowUnusedButtonPressed()));

    connect(findChild<QPushButton*>("pushButtonDumpOgreStats"), SIGNAL(pressed()), this, SLOT(DumpOgreResourceStatsToFile()));
    connect(findChild<QPushButton*>("buttonDumpSceneComplexity"), SIGNAL(pressed()), this, SLOT(DumpSceneComplexityToFile()));

    label_scripts_ = findChild<QLabel*>("labelScriptsInfo");
    table_scripts_ = findChild<QTableWidget*>("tableWidgetScripts");
    table_scripts_->horizontalHeader()->resizeSection(3, 80);
    table_scripts_->horizontalHeader()->resizeSection(4, 60);
    table_scripts_->horizontalHeader()->resizeSection(5, 70);
    table_scripts_->horizontalHeader()->resizeSection(6, 55);
    table_scripts_->horizontalHeader()->resizeSection(7, 55);
    table_scripts_->horizontalHeader()->resizeSection(8, 60);
    table_scripts_->horizontalHeader()->resizeSection(9, 60);
    table_scripts_->horizontalHeader()->resizeSection(10, 55);
    table_scripts_->horizontalHeader()->resizeSection(11, 60);
    
    frame_time_update_x_pos_ = 0;
    logThreshold_ = 100.0;

    QDoubleSpinBox* box = findChild<QDoubleSpinBox* >("loggerSpinbox");
    if (box != 0)
    {
        box->setRange(0.0, 1000.0);
        box->setValue(logThreshold_);
    }
    
    QPushButton* apply = findChild<QPushButton*>("loggerApply");
    if (apply != 0)
    {
        apply->setCheckable(true);
        connect(apply, SIGNAL(clicked()), this, SLOT(ChangeLoggerThreshold()));
    }

    // Set/init working directory for log files.
    logDirectory_ = Application::UserDataDirectory();
    if (!logDirectory_.exists(DEFAULT_LOG_DIR))
        logDirectory_.mkdir(DEFAULT_LOG_DIR);
    logDirectory_.cd(DEFAULT_LOG_DIR);
    ///\todo Regression. Reimplement support for meshes and other types
//    connect(tree_mesh_assets_, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(ShowAsset(QTreeWidgetItem*, int)));
    connect(tree_texture_assets_, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(ShowAsset(QTreeWidgetItem*, int)));

    // Add a context menu to Textures, Meshes and Materials widgets.
    if (tree_texture_assets_)
    {
        tree_texture_assets_->installEventFilter(this);
        menu_texture_assets_ = new QMenu(tree_texture_assets_);
        menu_texture_assets_->setAttribute(Qt::WA_DeleteOnClose);
        QAction *copyAssetName = new QAction(tr("Copy"), menu_texture_assets_);
        connect(copyAssetName, SIGNAL(triggered()), this, SLOT(CopyTextureAssetName()));
        menu_texture_assets_->addAction(copyAssetName);
    }
    if (tree_mesh_assets_)
    {
        tree_mesh_assets_->installEventFilter(this);
        menu_mesh_assets_ = new QMenu(tree_mesh_assets_);
        menu_mesh_assets_->setAttribute(Qt::WA_DeleteOnClose);
        QAction *copyAssetName = new QAction(tr("Copy"), menu_mesh_assets_);
        connect(copyAssetName, SIGNAL(triggered()), this, SLOT(CopyMeshAssetName()));
        menu_mesh_assets_->addAction(copyAssetName);
    }
    if (tree_material_assets_)
    {
        tree_material_assets_->installEventFilter(this);
        menu_material_assets_ = new QMenu(tree_material_assets_);
        menu_material_assets_->setAttribute(Qt::WA_DeleteOnClose);
        QAction *copyAssetName = new QAction(tr("Copy"), menu_material_assets_);
        connect(copyAssetName, SIGNAL(triggered()), this, SLOT(CopyMaterialAssetName()));
        menu_material_assets_->addAction(copyAssetName);
    }

    profiler_update_timer_.setSingleShot(true);
    connect(&profiler_update_timer_, SIGNAL(timeout()), this, SLOT(RefreshProfilerWindow()));

    connect(findChild<QPushButton*>("buttonRefresh"), SIGNAL(pressed()), this, SLOT(RefreshProfilingData()));
    connect(combo_timing_refresh_interval_, SIGNAL(currentIndexChanged(int)), this, SLOT(TimingRefreshIntervalChanged()));
}

namespace
{
    /// Takes the currentItem() of the given widget and copies the text of its first column to clipboard, if it exists.
    void CopySelectedItemName(QTreeWidget *treeWidget)
    {
        if (!treeWidget)
            return;

        QTreeWidgetItem *item = treeWidget->currentItem();

        if (!item)
            return;

        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(item->text(0));
    }
}

bool TimeProfilerWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::ContextMenu)
    {
        QTreeWidget *widget = dynamic_cast<QTreeWidget*>(obj);
        if (widget == tree_texture_assets_)
            menu_texture_assets_->popup(framework_->Ui()->MainWindow()->mapFromGlobal(QCursor::pos()));
        if (widget == tree_mesh_assets_)
            menu_mesh_assets_->popup(framework_->Ui()->MainWindow()->mapFromGlobal(QCursor::pos()));
        if (widget == tree_material_assets_)
            menu_material_assets_->popup(framework_->Ui()->MainWindow()->mapFromGlobal(QCursor::pos()));

        return true;
    }
    return false;
}

void TimeProfilerWindow::CopyTextureAssetName()
{
    CopySelectedItemName(tree_texture_assets_);
}

void TimeProfilerWindow::CopyMeshAssetName()
{
    CopySelectedItemName(tree_mesh_assets_);
}

void TimeProfilerWindow::CopyMaterialAssetName()
{
    CopySelectedItemName(tree_material_assets_);
}

void TimeProfilerWindow::ShowAsset(QTreeWidgetItem* item, int column)
{
    AssetPtr asset = framework_->Asset()->GetAsset(item->text(0));
    if (!asset)
    {
        LogError("TimeProfilerWindow::ShowAsset: could not obtain asset " + item->text(0));
        return;
    }

    QMenu dummyMenu;
    QList<QObject*> targets;
    targets.push_back(asset.get());

    framework_->Ui()->EmitContextMenuAboutToOpen(&dummyMenu, targets);
    foreach(QAction* action, dummyMenu.actions())
        if (action->text() == "Open")
        {
            action->activate(QAction::ActionEvent());
            break;
        }
}

void TimeProfilerWindow::RefreshScriptProfilingData()
{
    if (!visibility_ || !tab_widget_ || tab_widget_->currentIndex() != 15 || !table_scripts_ || !label_scripts_)
        return;   
    
#ifdef EC_Script_ENABLED
    QTime t;
    t.start();
    
    if (!framework_->Scene()->MainCameraScene())
    {
        label_scripts_->setText("Scene is null, no scripts running.");
        return;
    }
    EntityList scripts = framework_->Scene()->MainCameraScene()->GetEntitiesWithComponent(EC_Script::TypeNameStatic());
    if (scripts.empty())
    {
        label_scripts_->setText("No scripts running in the scene");
        return;
    }
    
    // This is the column order in the table widget. Also the dump QMap should have these keys if filled properly.
    QStringList keys;
    keys << "QScriptValues" << "Objects" << "Functions" << "Strings" << "Arrays" << "Numbers" << "Booleans" << "Is null" << "QObjects" << "QObject methods";
    
    bool createdNew = false;
    QList<entity_id_t> processedParentEnts;
    for(EntityList::iterator iter=scripts.begin(); iter!=scripts.end(); iter++)
    {
        EntityPtr ent = (*iter);
        if (!ent.get())
            continue;

        EC_Script *script = ent->GetComponent<EC_Script>().get();
        if (!script || !script->ScriptInstance() || !script->ScriptInstance()->IsEvaluated())
            continue;
        
        QMap<QString, uint> dump = script->ScriptInstance()->DumpEngineInformation();
        if (!dump.isEmpty())
        {
            if (!processedParentEnts.contains(ent->Id()))
                processedParentEnts.push_back(ent->Id());

            QString idStr = QString::number(ent->Id());
            
            QStringList scriptRefs;
            foreach(QVariant variantRef, script->scriptRef.Get().refs)
            {
                QString ref = variantRef.value<AssetReference>().ref;
                if (!ref.isEmpty())
                    scriptRefs << ref;
            }
            QString scriptRefCombined = scriptRefs.join(", ");
                
            QTableWidgetItem *existingItem = 0;
            foreach(QTableWidgetItem *item, table_scripts_->findItems(idStr, Qt::MatchExactly))
            {
                if (item->column() == 0)
                {
                    existingItem = item;
                    break;
                }
            }
            
            if (!existingItem)
            {
                // Create new cell and init data
                int row = table_scripts_->rowCount();
                table_scripts_->insertRow(row);
                table_scripts_->setItem(row, 0, new QTableWidgetItem(idStr));
                table_scripts_->setItem(row, 1, new QTableWidgetItem(ent->Name()));
                table_scripts_->setItem(row, 2, new QTableWidgetItem(script->Name()));
                for (int i=0; i<keys.size(); i++)
                    table_scripts_->setItem(row, i+3, new QTableWidgetItem(QString::number(dump.value(keys[i], 0))));
                table_scripts_->setItem(row, table_scripts_->columnCount()-1, new QTableWidgetItem(scriptRefCombined));
                createdNew = true;
            }
            else 
            {
                int row = existingItem->row();
                
                // Update names, they might have been renamed. Only set when changed to save update rendering costs.
                if (table_scripts_->item(row, 1) && table_scripts_->item(row, 1)->text() != ent->Name()) 
                    table_scripts_->item(row, 1)->setText(ent->Name());
                if (table_scripts_->item(row, 2) && table_scripts_->item(row, 2)->text() != script->Name()) 
                    table_scripts_->item(row, 2)->setText(script->Name());
                    
                // Iterate dump map with column keys
                for (int i=0; i<keys.size(); i++)
                {
                    uint value = dump.value(keys[i], 0);
                    QString valueStr = QString::number(value);
                    if (table_scripts_->item(row, i+3)) 
                    {
                        // Only update if value changed or toUint() fails
                        bool ok = false;
                        uint cellValue = table_scripts_->item(row, i+3)->text().toUInt(&ok);
                        if (ok)
                        {
                            if (cellValue != value)
                            {
                                int difference = value - cellValue;
                                QString diffString = (difference < 0 ? " (" : " (+") + QString::number(difference) + ")";
                                table_scripts_->item(row, i+3)->setText(valueStr + diffString);
                            }
                        }
                        else
                            table_scripts_->item(row, i+3)->setText(valueStr);
                    }
                }
                
                // Update script refs
                QTableWidgetItem *refsColumnItem = table_scripts_->item(row, table_scripts_->columnCount()-1);
                if (refsColumnItem && refsColumnItem->text() != scriptRefCombined)
                    refsColumnItem->setText(scriptRefCombined);
            }
        }
    }
    
    // This is done to show the entity/component name nicely at all times
    if (createdNew)
    {
        table_scripts_->resizeColumnToContents(0);
        table_scripts_->resizeColumnToContents(1);
        table_scripts_->resizeColumnToContents(2);
    }
        
    // If any removed entities/scripts are found, nuke the whole table.
    // It will get a fresh update on the next cycle, and is whole lot easier than
    // removing row by row as the indexes change when removed.
    bool clearTable = false;
    for (int i=0; i<table_scripts_->rowCount(); i++)
    {
        if (table_scripts_->item(i, 0))
        {
            entity_id_t id = table_scripts_->item(i, 0)->text().toUInt();
            if (!processedParentEnts.contains(id))
            {
                clearTable = true;
                break;
            }
        }
    }
    if (clearTable)
    {
        table_scripts_->clearContents();
        while(table_scripts_->rowCount() > 0)
            table_scripts_->removeRow(0);
    }
    
    int elapsed = t.elapsed();
    label_scripts_->setText(QString("<span style=\"color:rgb(120,120,120);\">Information gathered in " + (elapsed <= 0 ? "<1" : QString::number(elapsed)) + " msec</span>"));
        
    QTimer::singleShot(1000, this, SLOT(RefreshScriptProfilingData()));
#else
    label_scripts_->setText("EC_Script not enabled in the build, cannot show information.");
#endif
}

void TimeProfilerWindow::ChangeLoggerThreshold()
{
     QDoubleSpinBox* box = findChild<QDoubleSpinBox* >("loggerSpinbox");
     if (box != 0)
        logThreshold_ = box->value();
}

/** @cond PRIVATE */
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
/** @endcond */

void TimeProfilerWindow::Arrange()
{
    if (!tab_widget_)
        return;

    QString name = QObject::sender()->objectName();

    QTreeWidget* widget = 0;
    switch (tab_widget_->currentIndex())
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

    if (widget == 0)
        return;

    if (name.contains("name") )
    {
        // By name
        widget->sortItems(0, Qt::AscendingOrder);
    }
    else if (name.contains("size") )
    {
        // By size
        int va = widget->topLevelItemCount();
        QList<QTreeWidgetItem* > items;
        for(int i = va; i--;)
        {
           QTreeWidgetItem* item =  widget->takeTopLevelItem(i);
           if (item != 0)
               items.append(item);
           else
               break;
        }

        if (items.size () != 0)
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
        RefreshProfilingDataTree(1.f); // The passed 1.f value is bogus, but after the first timed update, the widget will get the proper timings.
        push_button_toggle_tree_->setText("Top");
    }
    else
    {
        RefreshProfilingDataList(1.f); // The passed 1.f value is bogus, but after the first timed update, the widget will get the proper timings.
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
        RefreshProfilingDataTree(1.f);  // The passed 1.f value is bogus, but after the first timed update, the widget will get the proper timings.
    else
        RefreshProfilingDataList(1.f);  // The passed 1.f value is bogus, but after the first timed update, the widget will get the proper timings.

    ExpandAllButtonPressed();
}

void ColorTreeWidgetItemByTime(QTreeWidgetItem *item, float time)
{
    QColor q;

    if (time > 8)
        q = QColor(255,0,0);
    else if (time > 4.f)
        q = QColor(255, 127, 0);
    else if (time > 2.f)
        q = QColor(0, 200, 0);
    else if (time > 1.f)
        q = QColor(0, 0, 0);
    else
        q = QColor(128, 128, 128);
	item->setForeground(0, QBrush(q));
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
        RefreshAssetProfilingData();
        break;
    case 4:
        RefreshSceneComplexityProfilingData();
        break;
    case 5:
        RefreshRenderTargetProfilingData();
        break;
    case 6: // Textures
        RefreshAssetData(Ogre::TextureManager::getSingleton(), tree_texture_assets_, "texture");
        break;
    case 7: // Meshes
        RefreshAssetData(Ogre::MeshManager::getSingleton(), tree_mesh_assets_, "mesh");
        break;
    case 8: // Material
        RefreshAssetData(Ogre::MaterialManager::getSingleton(), tree_material_assets_, "material");
        break;
    case 9: // Skeleton
        RefreshAssetData(Ogre::SkeletonManager::getSingleton(), tree_skeleton_assets_, "skeleton");
        break;
    case 10: // Composition
        RefreshAssetData(Ogre::CompositorManager::getSingleton(), tree_compositor_assets_, "compositor");
        break;
    case 11: // Gpu assets
        RefreshAssetData(Ogre::HighLevelGpuProgramManager::getSingleton(), tree_gpu_assets_, "gpuasset");
        break;
    case 12: // Font assets
        RefreshAssetData(Ogre::FontManager::getSingleton(), tree_font_assets_, "font");
        break;
    case 13: // OgreSceneTree
        PopulateOgreSceneTree();
        break;
    case 14: // Bullet collision statistics
        PopulateBulletStats();
        break;
    case 15: // Scripts
        RefreshScriptProfilingData();
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

void TimeProfilerWindow::FillProfileTimingWindow(QTreeWidgetItem *qtNode, const ProfilerNodeTree *profilerNode, int numFrames, float frameTotalTimeSecs)
{
    const ProfilerNode *parentNode = dynamic_cast<const ProfilerNode*>(profilerNode);

    const ProfilerNodeTree::NodeList &children = profilerNode->GetChildren();
    for(ProfilerNodeTree::NodeList::const_iterator iter = children.begin(); iter != children.end(); ++iter)
    {
        ProfilerNodeTree *node = iter->get();

        const ProfilerNode *timings_node = dynamic_cast<const ProfilerNode*>(node);

        QTreeWidgetItem *item = FindItemByName(qtNode, node->Name().c_str());

        if (timings_node && timings_node->num_called_custom_ == 0 && !show_unused_ && !QString(node->Name().c_str()).startsWith("Thread"))
        {
            if (item)
                delete item;
            continue;
        }

//        QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString(node->Name().c_str())));

        if (!item)
        {
            item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString(node->Name().c_str())));
            qtNode->addChild(item);
        }

        if (timings_node)
        {
            char str[256] = "-";
            sprintf(str, "%d", (int)timings_node->num_called_custom_);
            item->setText(1, str);
            sprintf(str, "%.2f", (float)timings_node->num_called_custom_ / numFrames);
            item->setText(2, str);
            if (timings_node->num_called_custom_ > 0 && timings_node->custom_elapsed_min_ < 1e8)
            {
                sprintf(str, "%.2fms", timings_node->custom_elapsed_min_*1000.f);
                item->setText(3, str);
            }
            else
                item->setText(3, "-");
            if (timings_node->num_called_custom_ > 0)
            {
                sprintf(str, "%.2fms", timings_node->total_custom_*1000.f / timings_node->num_called_custom_);
                item->setText(4, str);
                sprintf(str, "%.2fms", timings_node->total_custom_*1000.f / numFrames);
                item->setText(5, str);
                sprintf(str, "%.2fms", timings_node->custom_elapsed_max_*1000.f);
                item->setText(6, str);
                sprintf(str, "%.2fms", timings_node->total_custom_*1000.f);
                item->setText(7, str);
                if (parentNode)
                    sprintf(str, "%.2f%%", timings_node->total_custom_ * 100.f / parentNode->total_custom_);
                else
                    sprintf(str, "-");
                item->setText(8, str);
                sprintf(str, "%.2f%%", timings_node->total_custom_ * 100.f / frameTotalTimeSecs);
                item->setText(9, str);
                float timeSpentInChildren = timings_node->TotalCustomSpentInChildren();
                if (timeSpentInChildren >= 0.f)
                    sprintf(str, "%.2f%%", 100.f - timeSpentInChildren * 100.f / timings_node->total_custom_);
                else
                    sprintf(str, "-");
                item->setText(10, str);
                float timeSpentInThisExclusive = timings_node->total_custom_ - max(timeSpentInChildren, 0.f);
                sprintf(str, "%.2fms", timeSpentInThisExclusive * 1000.f / numFrames);
                item->setText(11, str);
                sprintf(str, "%.2fms", timeSpentInThisExclusive * 1000.f);
                item->setText(12, str);
            }
            else
            {
                item->setText(4, "-");
                item->setText(5, "-");
                item->setText(6, "-");
                item->setText(7, "-");
                item->setText(8, "-");
                item->setText(9, "-");
                item->setText(10, "-");
                item->setText(11, "-");
                item->setText(12, "-");
            }
            ColorTreeWidgetItemByTime(item, timings_node->total_custom_ * 1000.f / numFrames);
        }

        FillProfileTimingWindow(item, node, numFrames, frameTotalTimeSecs);

        if (timings_node)
        {
            timings_node->num_called_custom_ = 0;
            timings_node->total_custom_ = 0;
            timings_node->custom_elapsed_min_ = 1e9;
            timings_node->custom_elapsed_max_ = 0;
        }
    }
}

void TimeProfilerWindow::RedrawFrameTimeHistoryGraphDelta(const std::vector<std::pair<u64, double> > &frameTimes)
{
    ///\todo Implement.
}

void DumpProfilerSpikes(ProfilerNodeTree *node, float limit, int frameNumber)
{
    const ProfilerNode *timings_node = dynamic_cast<const ProfilerNode*>(node);
    if (timings_node && timings_node->custom_elapsed_max_*1000.f >= limit)
    {
        LogInfo("Frame " + QString::number(frameNumber) + ", " + QString(node->Name().c_str()) + ": " + QString::number(timings_node->custom_elapsed_max_*1000.f) + " msecs.");
        timings_node->num_called_custom_ = 0;
        timings_node->total_custom_ = 0;
        timings_node->custom_elapsed_min_ = 1e9;
        timings_node->custom_elapsed_max_ = 0;
    }

    const ProfilerNodeTree::NodeList &children = node->GetChildren();
    for(ProfilerNodeTree::NodeList::const_iterator iter = children.begin(); iter != children.end(); ++iter)
        DumpProfilerSpikes(iter->get(), limit, frameNumber);
}

void TimeProfilerWindow::RedrawFrameTimeHistoryGraph(const std::vector<std::pair<u64, double> > &frameTimes)
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
        // ProfilerBlock::ElapsedTimeSeconds(*(LARGE_INTEGER*)&frameTimes[firstEntry + i].first, now);
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

    //if (timePerFrame >= logThreshold_ )
    //     DumpNodeData();

    QPushButton *loggerEnabled = findChild<QPushButton*>("loggerApply");
    if (loggerEnabled && loggerEnabled->isChecked())
    {
        float threshold = 100.f;
        QDoubleSpinBox* box = findChild<QDoubleSpinBox* >("loggerSpinbox");
        if (box != 0)
            threshold = (float)box->value();

#ifdef PROFILING
        Profiler &profiler = *framework_->GetProfiler();
        profiler.Lock();
        DumpProfilerSpikes(profiler.GetRoot(), threshold, framework_->Frame()->FrameNumber());
        profiler.Release();
#endif
    }
}

/*
void TimeProfilerWindow::DumpNodeData()
{
   
    QFile file("performancelogger.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
         return;

    if (tree_profiling_data_ == 0)
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

    while(*it) 
     {
         QString name = (*it)->text(0);
         if (name.contains("thread", Qt::CaseInsensitive) && !name.contains("ThreadTask") )
             out<<endl;

         for(int i = 0; i < cols; ++i)
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

    Profiler &profiler = *framework_->GetProfiler();
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
                //if (((timings_node->total_custom_*1000.f/ timings_node->num_called_custom_) * timings_node->num_called_) > logThreshold_)
                if (timings_node->total_custom_*1000.f >= logThreshold_)
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
            if (saveData)
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

void TimeProfilerWindow::FillThresholdLogger(QTextStream& out, const ProfilerNodeTree *profilerNode)
{
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
                 //if (((timings_node->total_custom_*1000.f/ timings_node->num_called_custom_) * timings_node->num_called_) > logThreshold_)
                if (timings_node->total_custom_*1000.f >= logThreshold_ )     
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
            if (saveData)
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

void TimeProfilerWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    emit Visible(true);
}

void TimeProfilerWindow::hideEvent(QHideEvent  *event)
{
    QWidget::hideEvent(event);
    emit Visible(false);
}

int TimeProfilerWindow::ReadProfilingRefreshInterval()
{
    assert(combo_timing_refresh_interval_);

    // Positive values denote
    const int refreshTimes[] = { -1, 10000, 5000, 2000, 1000, 500 };

    int selection = combo_timing_refresh_interval_->currentIndex();
    return refreshTimes[min(max(selection, 0), (int)(sizeof(refreshTimes)/sizeof(refreshTimes[0])-1))];
}

void TimeProfilerWindow::TimingRefreshIntervalChanged()
{
    findChild<QPushButton*>("buttonRefresh")->setEnabled(ReadProfilingRefreshInterval() <= 0);
    RefreshProfilerWindow();
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

static std::string ReadOgreManagerStatus(Ogre::ResourceManager &manager)
{
    char str[256];
    Ogre::ResourceManager::ResourceMapIterator iter = manager.getResourceIterator();
    sprintf(str, "Budget: %s, Usage: %s, # of resources: %d",
        kNet::FormatBytes((u64)manager.getMemoryBudget()).c_str(),
        kNet::FormatBytes((u64)manager.getMemoryUsage()).c_str(),
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

    if (!framework_->IsHeadless())
        findChild<QLabel*>("labelTextureManager")->setText(ReadOgreManagerStatus(Ogre::TextureManager::getSingleton()).c_str());
    else
        findChild<QLabel*>("labelTextureManager")->setText("Texture information not available in headless mode.");
    findChild<QLabel*>("labelMeshManager")->setText(ReadOgreManagerStatus(Ogre::MeshManager::getSingleton()).c_str());
    findChild<QLabel*>("labelMaterialManager")->setText(ReadOgreManagerStatus(Ogre::MaterialManager::getSingleton()).c_str());
    findChild<QLabel*>("labelSkeletonManager")->setText(ReadOgreManagerStatus(Ogre::SkeletonManager::getSingleton()).c_str());
    findChild<QLabel*>("labelCompositorManager")->setText(ReadOgreManagerStatus(Ogre::CompositorManager::getSingleton()).c_str());
    findChild<QLabel*>("labelGPUProgramManager")->setText(ReadOgreManagerStatus(Ogre::HighLevelGpuProgramManager::getSingleton()).c_str());
    findChild<QLabel*>("labelFontManager")->setText(ReadOgreManagerStatus(Ogre::FontManager::getSingleton()).c_str());

    OgreRenderer::RendererPtr renderer = framework_->GetModule<OgreRenderer::OgreRenderingModule>()->GetRenderer();
    if (renderer.get() && renderer->GetActiveOgreWorld().get())
    {
        Ogre::SceneManager *scene = renderer->GetActiveOgreWorld()->OgreSceneManager();
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
    }

    profiler_update_timer_.start(500);
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

void TimeProfilerWindow::RefreshProfilerWindow()
{
    OnProfilerWindowTabChanged(tab_widget_->currentIndex());
}

void TimeProfilerWindow::RefreshProfilingData()
{
#ifdef PROFILING
    if (!visibility_ || !tab_widget_ || tab_widget_->currentIndex() != 0)
        return;

    static tick_t timePrev = GetCurrentClockTime();
    tick_t timeNow = GetCurrentClockTime();

    static tick_t timerFrequency = GetCurrentClockFreq();

    double msecsOccurred = (double)(timeNow - timePrev) * 1000.0 / timerFrequency;
    timePrev = timeNow;

    if (labelTimings)
    {
        Profiler &profiler = *framework_->GetProfiler();
        profiler.Lock();

        int numFrames = 1;
        ProfilerNode *processFrameNode = dynamic_cast<ProfilerNode*>(profiler.FindBlockByName("Framework_ProcessOneFrame")); // We use this node to estimate FPS for display.
        char str[256];
        if (processFrameNode)
        {
            numFrames = std::max<int>(processFrameNode->num_called_custom_, 1);
            sprintf(str, "%.2f FPS (%.2f msecs/frame)", (numFrames * 1000.f / msecsOccurred), msecsOccurred / numFrames);
        }
        else
            sprintf(str, "- FPS");
        labelTimings->setText(str);
        profiler.Release();
    }

    if (show_profiler_tree_)
        RefreshProfilingDataTree(msecsOccurred);
    else
        RefreshProfilingDataList(msecsOccurred);

    int refreshInterval = ReadProfilingRefreshInterval();
    if (refreshInterval > 0) // If refreshInterval < 0, we treat it as 'user refreshes manually'.
        profiler_update_timer_.start(refreshInterval);
#endif
}

void TimeProfilerWindow::RefreshProfilingDataTree(float msecsOccurred)
{
#ifdef PROFILING
//    Profiler *profiler = ProfilerSection::GetProfiler();

    Profiler &profiler = *framework_->GetProfiler();
    profiler.Lock();
//    ProfilerNodeTree *node = profiler.Lock().get();
    ProfilerNodeTree *node = profiler.GetRoot();

    int numFrames = 1;
    ProfilerNode *processFrameNode = dynamic_cast<ProfilerNode*>(profiler.FindBlockByName("Framework_ProcessOneFrame")); // We use this node to estimate FPS for display.
    if (processFrameNode)
        numFrames = std::max<int>(processFrameNode->num_called_custom_, 1);

    const ProfilerNodeTree::NodeList &children = node->GetChildren();
    for(ProfilerNodeTree::NodeList::const_iterator iter = children.begin(); iter != children.end(); ++iter)
    {
        node = iter->get();

        const ProfilerNode *timings_node = dynamic_cast<const ProfilerNode*>(node);

        QTreeWidgetItem *item = FindItemByName(tree_profiling_data_, node->Name().c_str());
        if (!item)
        {
            item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString(node->Name().c_str())));
            tree_profiling_data_->addTopLevelItem(item);
        }

        if (timings_node)
        {
            char str[256] = "-";
            sprintf(str, "%d", (int)timings_node->num_called_custom_);
            item->setText(1, str);
            sprintf(str, "%.2f", (float)timings_node->num_called_custom_ / numFrames);
            item->setText(2, str);
            if (timings_node->num_called_custom_ > 0 && timings_node->custom_elapsed_min_ < 1e8)
            {
                sprintf(str, "%.2fms", timings_node->custom_elapsed_min_*1000.f);
                item->setText(3, str);
            }
            else
                item->setText(3, "-");
            if (timings_node->num_called_custom_ > 0)
            {
                sprintf(str, "%.2fms", timings_node->total_custom_*1000.f / timings_node->num_called_custom_);
                item->setText(4, str);
                sprintf(str, "%.2fms", timings_node->total_custom_*1000.f / numFrames);
                item->setText(5, str);
                sprintf(str, "%.2fms", timings_node->custom_elapsed_max_*1000.f);
                item->setText(6, str);
                sprintf(str, "%.2fms", timings_node->total_custom_*1000.f);
                item->setText(7, str);
                sprintf(str, "100%%");
                item->setText(8, str);
                sprintf(str, "%.2f%%", timings_node->total_custom_ * 1000.f * 100.f / msecsOccurred);
                item->setText(9, str);
                float timeSpentInChildren = timings_node->TotalCustomSpentInChildren();
                if (timeSpentInChildren >= 0.f)
                    sprintf(str, "%.2f%%", 100.f - timeSpentInChildren * 100.f / timings_node->total_custom_);
                else
                    sprintf(str, "-");
                item->setText(10, str);
                float timeSpentInThisExclusive = timings_node->total_custom_ - max(timeSpentInChildren, 0.f);
                sprintf(str, "%.2fms", timeSpentInThisExclusive * 1000.f / numFrames);
                item->setText(11, str);
                sprintf(str, "%.2fms", timeSpentInThisExclusive * 1000.f);
                item->setText(12, str);
            }
            else
            {
                item->setText(4, "-");
                item->setText(5, "-");
                item->setText(6, "-");
                item->setText(7, "-");
                item->setText(8, "-");
                item->setText(9, "-");
                item->setText(10, "-");
                item->setText(11, "-");
                item->setText(12, "-");
            }

            ColorTreeWidgetItemByTime(item, timings_node->total_custom_ * 1000.f / numFrames);

            timings_node->num_called_custom_ = 0;
            timings_node->total_custom_ = 0;
            timings_node->custom_elapsed_min_ = 1e9;
            timings_node->custom_elapsed_max_ = 0;
        }

        FillProfileTimingWindow(item, node, numFrames, msecsOccurred / 1000.f);
    }
    Physics::UpdateBulletProfilingData(tree_profiling_data_->invisibleRootItem(), numFrames);
    profiler.Release();
#endif
}

void TimeProfilerWindow::CollectProfilerNodes(ProfilerNodeTree *node, std::vector<const ProfilerNode *> &dst)
{
    const ProfilerNodeTree::NodeList &children = node->GetChildren();
    for(ProfilerNodeTree::NodeList::const_iterator iter = children.begin(); iter != children.end(); ++iter)
    {
        ProfilerNodeTree *child = iter->get();

        const ProfilerNode *timings_node = dynamic_cast<const ProfilerNode*>(child);
        if (timings_node)
            dst.push_back(timings_node);
        CollectProfilerNodes(child, dst);
    }
}

bool ProfilingNodeLessThan(const ProfilerNode *a, const ProfilerNode *b)
{
    double aTimePerFrame = (a->num_called_custom_ == 0) ? 0 : (a->total_custom_ / a->num_called_custom_);
    double bTimePerFrame = (b->num_called_custom_ == 0) ? 0 : (b->total_custom_ / b->num_called_custom_);
    return aTimePerFrame > bTimePerFrame;
}

void TimeProfilerWindow::RefreshProfilingDataList(float msecsOccurred)
{
#ifdef PROFILING
    Profiler &profiler = *framework_->GetProfiler();
    profiler.Lock();

    ProfilerNodeTree *root = profiler.GetRoot();
    std::vector<const ProfilerNode *> nodes;
    CollectProfilerNodes(root, nodes);
    std::sort(nodes.begin(), nodes.end(), ProfilingNodeLessThan);

    tree_profiling_data_->clear();

    int numFrames = 1;
    ProfilerNode *processFrameNode = dynamic_cast<ProfilerNode*>(profiler.FindBlockByName("Framework_ProcessOneFrame")); // We use this node to estimate FPS for display.
    if (processFrameNode)
        numFrames = std::max<int>(processFrameNode->num_called_custom_, 1);

    for(std::vector<const ProfilerNode *>::iterator iter = nodes.begin(); iter != nodes.end(); ++iter)
    {
        const ProfilerNode *timings_node = *iter;
        assert(timings_node);

        if (timings_node->num_called_custom_ == 0 && !show_unused_)
            continue;

        QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0, QStringList(QString(timings_node->Name().c_str())));
        tree_profiling_data_->addTopLevelItem(item);

        char str[256] = "-";
        sprintf(str, "%d", (int)timings_node->num_called_custom_);
        item->setText(1, str);
        sprintf(str, "%.2f", (float)timings_node->num_called_custom_ / numFrames);
        item->setText(2, str);
        if (timings_node->num_called_custom_ > 0 && timings_node->custom_elapsed_min_ < 1e8)
        {
            sprintf(str, "%.2fms", timings_node->custom_elapsed_min_*1000.f);
            item->setText(3, str);
        }
        else
            item->setText(3, "-");
        if (timings_node->num_called_custom_ > 0)
        {
            sprintf(str, "%.2fms", timings_node->total_custom_*1000.f / timings_node->num_called_custom_);
            item->setText(4, str);
            sprintf(str, "%.2fms", timings_node->total_custom_*1000.f / numFrames);
            item->setText(5, str);
            sprintf(str, "%.2fms", timings_node->custom_elapsed_max_*1000.f);
            item->setText(6, str);
            sprintf(str, "%.2fms", timings_node->total_custom_*1000.f);
            item->setText(7, str);
            sprintf(str, "-");
            item->setText(8, str);
            sprintf(str, "%.2f%%", timings_node->total_custom_ * 100.f * 1000.f / msecsOccurred);
            item->setText(9, str);
            float timeSpentInChildren = timings_node->TotalCustomSpentInChildren();
            if (timeSpentInChildren >= 0.f)
                sprintf(str, "%.2f%%", 100.f - timeSpentInChildren * 100.f / timings_node->total_custom_);
            else
                sprintf(str, "-");
            item->setText(10, str);
        }
        else
        {
            item->setText(4, "-");
            item->setText(5, "-");
            item->setText(6, "-");
            item->setText(7, "-");
            item->setText(8, "-");
            item->setText(9, "-");
            item->setText(10, "-");
        }

        ColorTreeWidgetItemByTime(item, timings_node->total_custom_ * 1000.f / numFrames);

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

void TimeProfilerWindow::RefreshAssetProfilingData()
{
    if (!visibility_ || !tab_widget_ || tab_widget_->currentIndex() != 3)
        return;

    tree_asset_cache_->clear();
    tree_asset_transfers_->clear();
/*     /// \todo Regression. Reimplement using the Asset API. -jj.

    shared_ptr<Asset ServiceInterface> asset_service = 
        framework_->GetS erviceM anager()->Get Service<AssetServiceInterface>(Service::ST_Asset).lock();
    if (!asset_service)
        return;
        
    AssetCacheInfoMap cache_map = asset_service->GetAssetCacheInfo();
    AssetCacheInfoMap::const_iterator i = cache_map.begin();
    while(i != cache_map.end())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0, QStringList());
        tree_asset_cache_->addTopLevelItem(item);
        
        item->setText(0, QString(i->first.c_str()));
        item->setText(1, QString(QString("%1").arg(i->second.count_)));
        item->setText(2, QString(kNet::FormatBytes((int)i->second.size_).c_str()));
        
        ++i;
    }

    AssetTransferInfoVector transfer_vector = asset_service->GetAssetTransferInfo();
    AssetTransferInfoVector::const_iterator j = transfer_vector.begin();
    while(j != transfer_vector.end())
    {
        QTreeWidgetItem *item = new QTreeWidgetItem((QTreeWidget*)0, QStringList());
        tree_asset_transfers_->addTopLevelItem(item);
        
        item->setText(0, QString((*j).id_.c_str()));
        item->setText(1, QString((*j).type_.c_str()));
        item->setText(2, QString((*j).provider_.c_str()));
        item->setText(3, QString(kNet::FormatBytes((int)(*j).size_).c_str()));
        item->setText(4, QString(kNet::FormatBytes((int)(*j).received_).c_str()));
        ++j;
    }

    QTimer::singleShot(500, this, SLOT(RefreshAssetProfilingData()));
    */
}

void TimeProfilerWindow::RefreshSceneComplexityProfilingData()
{
    if (!visibility_ || !text_scenecomplexity_ || !tab_widget_ || tab_widget_->currentIndex() != 4)
        return;
    
    Scene *scene = framework_->Scene()->MainCameraScene();
    if (!scene)
        return;
    
    OgreRenderer::RendererPtr renderer = framework_->GetModule<OgreRenderer::OgreRenderingModule>()->GetRenderer();
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

    uint visible_entities = renderer->GetActiveOgreWorld()->VisibleEntities().size();
    uint batches = 0;
    uint triangles = 0;
    float avgfps = 0.0f;

    // Get overall batchcount/trianglecount/fps data
    Ogre::RenderSystem::RenderTargetIterator rtiter = rendersys->getRenderTargetIterator();
    while(rtiter.hasMoreElements())
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
    for(Scene::iterator iter = scene->begin(); iter != scene->end(); ++iter)
    {
        Entity &entity = *iter->second;
        entities++;

        EC_Terrain* terrain = entity.GetComponent<EC_Terrain>().get();
        EC_Mesh* mesh = entity.GetComponent<EC_Mesh>().get();
        EC_OgreCustomObject* custom = entity.GetComponent<EC_OgreCustomObject>().get();
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
            for(int y = 0; y < terrain->PatchHeight(); ++y)
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
        
        {
            if (ogre_entity)
                meshentities++;
        }
        if (entity.GetComponent("EC_AnimationController").get())
            animated++;
    }
    
    // Go through all meshes and see which of them are in the scene
    Ogre::ResourceManager::ResourceMapIterator iter = Ogre::MeshManager::getSingleton().getResourceIterator();
    while(iter.hasMoreElements())
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
    while(mi != all_meshes.end())
    {
        Ogre::Mesh* mesh = *mi;
        GetVerticesAndTrianglesFromMesh(mesh, mesh_vertices, mesh_triangles);
        ++mi;
    }
    // Count scene/other mesh byte sizes
    mi = scene_meshes.begin();
    while(mi != scene_meshes.end())
    {
        scene_meshes_size += (*mi)->getSize();
        ++mi;
    }
    mi = other_meshes.begin();
    while(mi != other_meshes.end())
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
    while(tex_iter.hasMoreElements())
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
    while(ti != scene_textures.end())
    {
        scene_tex_size += (*ti)->getSize();
        scene_tex_pixels += (*ti)->getWidth() * (*ti)->getHeight();
        ++ti;
    }
    ti = all_textures.begin();
    while(ti != all_textures.end())
    {
        total_tex_size += (*ti)->getSize();
        total_tex_pixels += (*ti)->getWidth() * (*ti)->getHeight();
        ++ti;
    }
    ti = other_textures.begin();
    while(ti != other_textures.end())
    {
        other_tex_size += (*ti)->getSize();
        other_tex_pixels += (*ti)->getWidth() * (*ti)->getHeight();
        ++ti;
    }
    
    // Sort textures into categories
    uint scene_tex_categories[5];
    uint other_tex_categories[5];
    for(uint i = 0; i < 5; ++i)
    {
        scene_tex_categories[i] = 0;
        other_tex_categories[i] = 0;
    }
    
    ti = scene_textures.begin();
    while(ti != scene_textures.end())
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
    while(ti != other_textures.end())
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
    for(uint i = 0; i < mesh->getNumSubMeshes(); ++i)
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
    for(uint i = 0; i < entity->getNumSubEntities(); ++i)
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
    while(i != materials.end())
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

QTreeWidgetItem *AddNewItem(QTreeWidgetItem *parent, QString name)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent, QStringList(name));
    return item;
}

void AddOgreTUState(QTreeWidgetItem *parent, Ogre::TextureUnitState *node, int idx)
{
    QString str = QString("TUState ") + QString::number(idx) + ": " + node->getName().c_str();
    str += QString(", texture: ") + node->getTextureName().c_str();

    /*QTreeWidgetItem *item = */AddNewItem(parent, str);
}

void AddOgrePass(QTreeWidgetItem *parent, Ogre::Pass *node, int idx)
{
    QString str = QString("Pass ") + QString::number(idx) + ": \"" + node->getName().c_str() + "\"";

    if (node->getNumTextureUnitStates() == 0)
        str += ", contains 0 TextureUnitStates";

    QTreeWidgetItem *item = AddNewItem(parent, str);

    for(int i = 0; i < node->getNumTextureUnitStates(); ++i)
        AddOgreTUState(item, node->getTextureUnitState(i), i);
}

void AddOgreTechnique(QTreeWidgetItem *parent, Ogre::Technique *node, int idx)
{
    QString str = QString("Technique ") + QString::number(idx) + ": " + node->getName().c_str();

    QTreeWidgetItem *item = AddNewItem(parent, str);

    for(int i = 0; i < node->getNumPasses(); ++i)
        AddOgrePass(item, node->getPass(i), i);
}

QString VertexElementTypeToString(Ogre::VertexElementType type)
{
    if (type == Ogre::VET_FLOAT1) return "Float1";
    if (type == Ogre::VET_FLOAT2) return "Float2";
    if (type == Ogre::VET_FLOAT3) return "Float3";
    if (type == Ogre::VET_FLOAT4) return "Float4";
    if (type == Ogre::VET_COLOUR) return "Color";
    if (type == Ogre::VET_SHORT1) return "Short1";
    if (type == Ogre::VET_SHORT2) return "Short2";
    if (type == Ogre::VET_SHORT3) return "Short3";
    if (type == Ogre::VET_SHORT4) return "Short4";
    if (type == Ogre::VET_UBYTE4) return "UByte4";
    if (type == Ogre::VET_COLOUR_ARGB) return "Color_ARGB";
    if (type == Ogre::VET_COLOUR_ABGR) return "Color_ABGR";

    return "Unknown VET(" + QString::number((int)type) + ")";
}

QString VertexElementSemanticToString(Ogre::VertexElementSemantic sem)
{
    if (sem == Ogre::VES_POSITION) return "Position";
    if (sem == Ogre::VES_BLEND_WEIGHTS) return "BlendWeights";
    if (sem == Ogre::VES_BLEND_INDICES) return "BlendIndices";
    if (sem == Ogre::VES_NORMAL) return "Normal";
    if (sem == Ogre::VES_DIFFUSE) return "Diffuse";
    if (sem == Ogre::VES_SPECULAR) return "Specular";
    if (sem == Ogre::VES_TEXTURE_COORDINATES) return "TexCoord";
    if (sem == Ogre::VES_BINORMAL) return "Binormal";
    if (sem == Ogre::VES_TANGENT) return "Tangent";

    return "Unknown VES(" + QString::number((int)sem) + ")";
}

void AddOgreVertexElement(QTreeWidgetItem *parent, const Ogre::VertexElement *node, int idx)
{
    QString str = QString("VertexElem ") + QString::number(idx) + ": ";
    if (node)
    {
        str += "sourceBufferIndex: " + QString::number(node->getSource());
        str += ", offset: " + QString::number(node->getOffset());
        str += ", type: " + VertexElementTypeToString(node->getType());
        str += ", semantic: " + VertexElementSemanticToString(node->getSemantic());
        str += ", index: " + QString::number(node->getIndex());
        str += ", size: " + QString::number(node->getSize());
    }

    /*QTreeWidgetItem *nodeItem = */AddNewItem(parent, str);
}

void AddOgreMaterial(QTreeWidgetItem *parent, Ogre::MaterialPtr material)
{
    if (material.get())
    {
        for(int i = 0; i < material->getNumTechniques(); ++i)
            AddOgreTechnique(parent, material->getTechnique(i), i);
    }
}

void AddOgreSubEntity(QTreeWidgetItem *parent, Ogre::SubEntity *node, int idx)
{
    QString str = QString("SubEntity ") + QString::number(idx);
    str += QString(", material: ") + node->getMaterialName().c_str();

    Ogre::MaterialPtr material = node->getMaterial();
    if (!material.get())
        str += "(not loaded)";

    QTreeWidgetItem *item = AddNewItem(parent, str);
    if (material.get())
        AddOgreMaterial(item, material);

    Ogre::SubMesh *submesh = node->getSubMesh();
    if (submesh && submesh->vertexData && !submesh->useSharedVertices && submesh->vertexData->vertexDeclaration)
    {
        Ogre::VertexDeclaration *vd = submesh->vertexData->vertexDeclaration;
        for(size_t i = 0; i < vd->getElementCount(); ++i)
            AddOgreVertexElement(item, vd->getElement(i), i);
    }
}

void AddOgreMovableObject(QTreeWidgetItem *parent, Ogre::MovableObject *node)
{
    QString str = (node->getMovableType() + "(MovableObject): ").c_str();
    str += node->getName().c_str();
    str += ", isVisible: " + QString::number(node->isVisible());
    str += ", isAttached: " + QString::number(node->isAttached());
//    str += ", isParentTagPoint: " + QString::number(node->isParentTagPoint());
    str += ", isInScene: " + QString::number(node->isInScene());
    str += ", visibilityFlags: " + QString::number(node->getVisibilityFlags());

    Ogre::Entity *e = dynamic_cast<Ogre::Entity*>(node);
    if (e)
    {
        Ogre::MeshPtr mesh = e->getMesh();
        if (mesh.get())
            str += QString(", mesh: ") + mesh->getName().c_str();
        else
            str += ", (null MeshPtr)";
    }

    Ogre::ParticleSystem *ps = dynamic_cast<Ogre::ParticleSystem*>(node);
    if (ps)
    {
        str += QString(", material: ") + ps->getMaterialName().c_str();

        Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName(ps->getMaterialName());
        if (!material.get())
            str += " (not loaded)";
    }

    QTreeWidgetItem *nodeItem = AddNewItem(parent, str);

    if (ps)
    {
        Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().getByName(ps->getMaterialName());
        if (material.get())
            AddOgreMaterial(nodeItem, material);
    }

    if (e)
    {
        if (!e->hasSkeleton())
            AddNewItem(nodeItem, "Ogre::SkeletonInstance: (no skeleton)");
        else
            AddNewItem(nodeItem, ("Ogre::SkeletonInstance: " + e->getSkeleton()->getName()).c_str());
        for(size_t i = 0; i < e->getNumSubEntities(); ++i)
            AddOgreSubEntity(nodeItem, e->getSubEntity(i), i);
    }
}

void AddOgreSceneNode(QTreeWidgetItem *parent, Ogre::SceneNode *node);

void AddOgreNode(QTreeWidgetItem *parent, Ogre::Node *node)
{
    QTreeWidgetItem *nodeItem = AddNewItem(parent, ("Node: " + node->getName()).c_str());

    for(int i = 0; i < node->numChildren(); ++i)
    {
        Ogre::Node *child = node->getChild(i);
        Ogre::SceneNode *sceneNode = dynamic_cast<Ogre::SceneNode*>(child);
        if (sceneNode)
            AddOgreSceneNode(nodeItem, sceneNode);
        else
            AddOgreNode(nodeItem, sceneNode);
    }
}

void AddOgreSceneNode(QTreeWidgetItem *parent, Ogre::SceneNode *node)
{
    QTreeWidgetItem *nodeItem = AddNewItem(parent, ("SceneNode: " + node->getName()).c_str());

    for(int i = 0; i < node->numChildren(); ++i)
    {
        Ogre::Node *child = node->getChild(i);
        Ogre::SceneNode *sceneNode = dynamic_cast<Ogre::SceneNode*>(child);
        if (sceneNode)
            AddOgreSceneNode(nodeItem, sceneNode);
        else
            AddOgreNode(nodeItem, sceneNode);
    }

    for(int i = 0; i < node->numAttachedObjects(); ++i)
    {
        Ogre::MovableObject *movableObject = node->getAttachedObject(i);
        AddOgreMovableObject(nodeItem, movableObject);
    }
}

void AddOgreScene(QTreeWidgetItem *parent, Ogre::SceneManager *scene)
{
    QTreeWidgetItem *sceneItem = AddNewItem(parent, ("Scene: " + scene->getName()).c_str());
    AddOgreSceneNode(sceneItem, scene->getRootSceneNode());    
}

void TimeProfilerWindow::PopulateOgreSceneTree()
{
    QTreeWidget *tree = findChild<QTreeWidget* >("sceneDataTree");
    if (!tree)
        return;

    tree->clear();

    Ogre::Root *root = Ogre::Root::getSingletonPtr();
    assert(root);

    Ogre::SceneManagerEnumerator::SceneManagerIterator iter = root->getSceneManagerIterator();
    while(iter.hasMoreElements())
    {
        Ogre::SceneManager *scene = iter.getNext();
        if (scene)
            AddOgreScene(tree->invisibleRootItem(), scene);
    }    
}

void TimeProfilerWindow::PopulateBulletStats()
{
    Scene *scene = framework_->Scene()->MainCameraScene();
    if (!scene)
        return;
    shared_ptr<Physics::PhysicsWorld> physics = scene->GetWorld<Physics::PhysicsWorld>();
    const std::set<std::pair<btCollisionObject*, btCollisionObject*> > &collisions = physics->PreviousFrameCollisions();

    treeBulletStats->clear();
    for(std::set<std::pair<btCollisionObject*, btCollisionObject*> >::const_iterator iter = collisions.begin(); iter != collisions.end(); ++iter)
    {
        btCollisionObject* objectA = iter->first;
        btCollisionObject* objectB = iter->second;
        EC_RigidBody* bodyA = static_cast<EC_RigidBody*>(objectA->getUserPointer());
        EC_RigidBody* bodyB = static_cast<EC_RigidBody*>(objectB->getUserPointer());
        if (!bodyA || !bodyB)
            continue;

        Entity* entityA = bodyA->ParentEntity();
        Entity* entityB = bodyB->ParentEntity();
        if (!entityA || !entityB)
            continue;

        QStringList item;
        item << entityA->Name();
        static const char *shapeType[] = { "Box", "Sphere", "Cylinder", "Capsule", "TriMesh", "HeightField", "ConvexHull" };
        item << shapeType[bodyA->shapeType.Get()];
        item << entityB->Name();
        item << shapeType[bodyB->shapeType.Get()];

        treeBulletStats->insertTopLevelItem(0, new QTreeWidgetItem(item, 0));
    }
    if (tab_widget_->currentIndex() == 14)
        QTimer::singleShot(1000, this, SLOT(PopulateBulletStats()));
}

void TimeProfilerWindow::RefreshRenderTargetProfilingData()
{
    if (!visibility_ || !text_scenecomplexity_ || !tab_widget_ || tab_widget_->currentIndex() != 5)
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
    while(iter.hasMoreElements())
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
    if (!visibility_ || !tab_widget_ || tab_widget_->currentIndex() != 6)
        return;

    Ogre::ResourceManager::ResourceMapIterator iter = Ogre::TextureManager::getSingleton().getResourceIterator();

    while(iter.hasMoreElements())
    {
        Ogre::ResourcePtr resource = iter.getNext();
        // Is there already this kind of element? 
        QTreeWidgetItem *item = FindItemByName(tree_texture_assets_, resource->getName().c_str());
        if (item == 0) 
            item = new QTreeWidgetItem(tree_texture_assets_);

        FillItem(item, resource, "texture");
    }
}

/// Derive the tree widget item to implement a custom sort predicate that sorts certain columns by numbers. 
/// @cond PRIVATE
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

class OgreTextureAssetTreeWidgetItem : public QTreeWidgetItem
{
public:
    OgreTextureAssetTreeWidgetItem(QTreeWidget *parent)
    :QTreeWidgetItem(parent)
    {
    }

    bool operator<(const QTreeWidgetItem &rhs) const
    {
        switch(treeWidget()->sortColumn())
        {
        case 1: // Sort the number columns as numbers. 'Size'
        case 2: // Width
        case 3: // Height
        case 5: // # mips
        case 14: // 'Loading State'
        case 15: // 'State Count'
            return this->text(treeWidget()->sortColumn()).toInt() > rhs.text(treeWidget()->sortColumn()).toInt();
        default: // Others as text.
            return this->text(treeWidget()->sortColumn()) < rhs.text(treeWidget()->sortColumn());
        }
    }
};

class OgreMeshAssetTreeWidgetItem : public QTreeWidgetItem
{
public:
    OgreMeshAssetTreeWidgetItem(QTreeWidget *parent)
    :QTreeWidgetItem(parent)
    {
    }

    bool operator<(const QTreeWidgetItem &rhs) const
    {
        switch(treeWidget()->sortColumn())
        {
        case 5: // Sphere radius
            return this->text(treeWidget()->sortColumn()).toFloat() > rhs.text(treeWidget()->sortColumn()).toFloat();
        case 1: // Sort the number columns as numbers. 'Size'
        case 2: // # submeshes
        case 7: // # LOD levels
        case 8: // # Morph anims
        case 9: // # Poses
        case 19: // 'Loading State'
        case 20: // 'State Count'
            return this->text(treeWidget()->sortColumn()).toInt() > rhs.text(treeWidget()->sortColumn()).toInt();
        case 3: // # vertices
        case 4: // # indices
            {
                QStringList l = this->text(treeWidget()->sortColumn()).split(" ");
                QStringList r = rhs.text(treeWidget()->sortColumn()).split(" ");
                if (l.size() == 0 || r.size() == 0)
                    return false;
                return l[0].toInt() > r[0].toInt();
            }
        default: // Others as text.
            return this->text(treeWidget()->sortColumn()) < rhs.text(treeWidget()->sortColumn());
        }
    }
};
/// @endcond

void TimeProfilerWindow::RefreshAssetData(Ogre::ResourceManager& manager, QTreeWidget* widget, QString drawType)
{
    // Clear tree.
    widget->clear();

    if (framework_->IsHeadless() && drawType == "texture")
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(widget);
        item->setText(0, "Texture asset information not available in headless mode.");
        widget->addTopLevelItem(item);
        widget->resizeColumnToContents(0);
        return;
    }
   
    Ogre::ResourceManager::ResourceMapIterator iter = manager.getResourceIterator();
    while(iter.hasMoreElements())
    {
        Ogre::ResourcePtr resource = iter.getNext();
        // Is there already this kind of element? 
        QTreeWidgetItem *item = FindItemByName(widget, resource->getName().c_str());
        if (item == 0)
        {
            if (drawType == "texture")
                item = new OgreTextureAssetTreeWidgetItem(widget);
            else if (drawType == "mesh")
                item = new OgreMeshAssetTreeWidgetItem(widget);
            else
                item = new OgreAssetTreeWidgetItem(widget);
        }

        FillItem(item, resource, drawType);
    }
    for(int i = 0; i < widget->columnCount(); ++i)
        widget->resizeColumnToContents(i);
}

QString OgrePixelFormatToString(Ogre::PixelFormat fmt)
{
    switch(fmt)
    {
    case Ogre::PF_UNKNOWN: return "PF_UNKNOWN";
    case Ogre::PF_L8: return "PF_L8(PF_BYTE_L)";
    case Ogre::PF_L16: return "PF_L16(PF_SHORT_L)";
    case Ogre::PF_A8: return "PF_A8(PF_BYTE_A)";
    case Ogre::PF_A4L4: return "PF_A4L4";
    case Ogre::PF_BYTE_LA: return "PF_BYTE_LA";
    case Ogre::PF_R5G6B5: return "PF_R5G6B5";
    case Ogre::PF_B5G6R5: return "PF_B5G6R5";
    case Ogre::PF_R3G3B2: return "PF_R3G3B2";
    case Ogre::PF_A4R4G4B4: return "PF_A4R4G4B4";
    case Ogre::PF_A1R5G5B5: return "PF_A1R5G5B5";
    case Ogre::PF_R8G8B8: return "PF_R8G8B8(PF_BYTE_BGR)";
    case Ogre::PF_B8G8R8: return "PF_B8G8R8(PF_BYTE_RGB)";
    case Ogre::PF_A8R8G8B8: return "PF_A8R8G8B8(PF_BYTE_BGRA)";
    case Ogre::PF_A8B8G8R8: return "PF_A8B8G8R8(PF_BYTE_RGBA)";
    case Ogre::PF_B8G8R8A8: return "PF_B8G8R8A8";
    case Ogre::PF_R8G8B8A8: return "PF_R8G8B8A8";
    case Ogre::PF_X8R8G8B8: return "PF_X8R8G8B8";
    case Ogre::PF_X8B8G8R8: return "PF_X8B8G8R8";
    case Ogre::PF_A2R10G10B10: return "PF_A2R10G10B10";
    case Ogre::PF_A2B10G10R10: return "PF_A2B10G10R10";
    case Ogre::PF_DXT1: return "PF_DXT1";
    case Ogre::PF_DXT2: return "PF_DXT2";
    case Ogre::PF_DXT3: return "PF_DXT3";
    case Ogre::PF_DXT4: return "PF_DXT4";
    case Ogre::PF_DXT5: return "PF_DXT5";
    case Ogre::PF_FLOAT16_R: return "PF_FLOAT16_R";
    case Ogre::PF_FLOAT16_RGB: return "PF_FLOAT16_RGB";
    case Ogre::PF_FLOAT16_RGBA: return "PF_FLOAT16_RGBA";
    case Ogre::PF_FLOAT32_R: return "PF_FLOAT32_R";
    case Ogre::PF_FLOAT32_RGB: return "PF_FLOAT32_RGB";
    case Ogre::PF_FLOAT32_RGBA: return "PF_FLOAT32_RGBA";
    case Ogre::PF_FLOAT16_GR: return "PF_FLOAT16_GR";
    case Ogre::PF_FLOAT32_GR: return "PF_FLOAT32_GR";
    case Ogre::PF_DEPTH: return "PF_DEPTH";
    case Ogre::PF_SHORT_RGBA: return "PF_SHORT_RGBA";
    case Ogre::PF_SHORT_GR: return "PF_SHORT_GR";
    case Ogre::PF_SHORT_RGB: return "PF_SHORT_RGB";
    case Ogre::PF_PVRTC_RGB2: return "PF_PVRTC_RGB2";
    case Ogre::PF_PVRTC_RGBA2: return "PF_PVRTC_RGBA2";
    case Ogre::PF_PVRTC_RGB4: return "PF_PVRTC_RGB4";
    case Ogre::PF_PVRTC_RGBA4: return "PF_PVRTC_RGBA4";
    case Ogre::PF_COUNT: return "Invalid(PF_COUNT)";
    default: return "Invalid(" + QString::number(fmt) + ")";
    }
}

void CountNumVertices(Ogre::MeshPtr mesh, int &vertices, int &indices)
{
    vertices = 0;
    indices = 0;

    if (mesh->sharedVertexData)
        vertices += mesh->sharedVertexData->vertexCount;

    for(unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);
        if (submesh && submesh->vertexData)
            vertices += submesh->vertexData->vertexCount;
        if (submesh && submesh->indexData)
            indices += submesh->indexData->indexCount;
    }
}

// Returns a string of form "5+102+42 (shared 100)" to describe the vertex breakdown in the submeshes of this mesh.
QString VertexSumString(Ogre::MeshPtr mesh)
{
    QString str;

    for(unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);
        if (i != 0)
            str += "+";
        if (submesh && submesh->vertexData)
            str += QString::number(submesh->vertexData->vertexCount);
        else
            str += "(null)";
    }
    if (mesh->sharedVertexData)
        str += " (shared " + QString::number(mesh->sharedVertexData->vertexCount) + ")";

    return str;
}

// Returns a string of form "5+102+42" to describe the index breakdown in the submeshes of this mesh.
QString IndexSumString(Ogre::MeshPtr mesh)
{
    QString str;

    for(unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);
        if (i != 0)
            str += "+";
        if (submesh && submesh->indexData)
            str += QString::number(submesh->indexData->indexCount);
        else
            str += "(null)";
    }
    return str;
}

void TimeProfilerWindow::FillItem(QTreeWidgetItem* item, const Ogre::ResourcePtr& resource, QString viewType)
{
    if (item == 0)
        return;

    item->setText(0, AssetAPI::DesanitateAssetRef(resource->getName()).c_str());
    QString size;
    size.setNum(resource->getSize());
    item->setText(1,size);

    int i = 2;
    if (viewType == "texture")
    {
        i = 6;
        Ogre::TexturePtr tex = Ogre::TexturePtr(resource);
        
        item->setText(2, QString::number(tex->getWidth()));
        item->setText(3, QString::number(tex->getHeight()));
        item->setText(4, OgrePixelFormatToString(tex->getFormat()));
        item->setText(5, QString::number(tex->getNumMipmaps()));
    }
    else if (viewType == "mesh")
    {
        i = 11;
        Ogre::MeshPtr mesh = Ogre::MeshPtr(resource);
        if (mesh.get())
        {
            item->setText(2, QString::number(mesh->getNumSubMeshes()));
            int numVertices;
            int numIndices;
            CountNumVertices(mesh, numVertices, numIndices);
            item->setText(3, QString::number(numVertices) + " (" + VertexSumString(mesh) + ")");
            item->setText(4, QString::number(numIndices) + " (" + IndexSumString(mesh) + ")");
            item->setText(5, QString::number(mesh->getBoundingSphereRadius()));
            item->setText(6, mesh->getSkeletonName().c_str());
            item->setText(7, QString::number(mesh->getNumLodLevels()));
            item->setText(8, QString::number(mesh->getNumAnimations()));
            item->setText(9, QString::number(mesh->getPoseCount()));
            item->setText(10, "todo"); ///\todo Aggregate the material names from submeshes here.
        }
    }

    item->setText(i, resource->isManuallyLoaded() ? "Yes" : "No");
    item->setText(i+1, resource->isReloadable() ? "Yes" : "No");
    item->setText(i+2, resource->getGroup().c_str());
    item->setText(i+3, resource->getOrigin().c_str());
    item->setText(i+4, resource->isLoaded()  ? "Yes" : "No");
    item->setText(i+5, resource->isLoading()  ? "Yes" : "No");
    item->setText(i+6, resource->isBackgroundLoaded() ? "Yes" : "No");
    item->setText(i+7, resource->isPrepared()  ? "Yes" : "No");
    item->setText(i+8, QString::number(resource->getLoadingState()));
    item->setText(i+9, QString::number(resource->getStateCount()));
}
