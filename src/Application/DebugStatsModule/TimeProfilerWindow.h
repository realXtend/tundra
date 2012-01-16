// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "CoreTypes.h"

#include <QWidget>
#include <QTimer>
#include <QDir>
#include <QMap>
#include <QString>
#include <Ogre.h>

#ifdef OGREASSETEDITOR_ENABLED
class TexturePreviewEditor;
#endif

class QTreeWidget;
class QComboBox;
class QTabWidget;
class QLabel;
class QTreeWidget;
class QTreeWidgetItem;
class QTextEdit;
class QMenu;
class QPushButton;
class QTextStream;

class Framework;
class ProfilerNodeTree;
class ProfilerNode;

/// Provides various profiling data, performance statistics and tools.
class TimeProfilerWindow : public QWidget
{
    Q_OBJECT

public:
    /// The ctor adds this window to scene, but does not show it.
    explicit TimeProfilerWindow(Framework *fw);
    void RedrawFrameTimeHistoryGraph(const std::vector<std::pair<u64, double> > &frameTimes);
    void RedrawFrameTimeHistoryGraphDelta(const std::vector<std::pair<u64, double> > &frameTimes);
    void DoThresholdLogging();

public slots:
    void RefreshProfilingData();
    void OnProfilerWindowTabChanged(int newPage);
    void RefreshOgreProfilingWindow();

    void RefreshTextureProfilingData();
    void ToggleTreeButtonPressed();
    void CollapseAllButtonPressed();
    void ExpandAllButtonPressed();
    void ShowUnusedButtonPressed();
    void RefreshAssetProfilingData();
    void RefreshSceneComplexityProfilingData();
    void RefreshRenderTargetProfilingData();
    void DumpOgreResourceStatsToFile();
    void DumpSceneComplexityToFile();
    void Arrange();
    //void DumpNodeData();
    void ChangeLoggerThreshold();
    void SetVisibility(bool visibility) { visibility_ = visibility; }
    void ShowMeshAsset(QTreeWidgetItem* item, int column);
    void ShowTextureAsset(QTreeWidgetItem* item, int column);

signals:
    void Visible(bool visible);

protected:
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);

private slots:
    void CopyTextureAssetName();
    void CopyMeshAssetName();
    void CopyMaterialAssetName();
    void PopulateBulletStats();

private:
    bool eventFilter(QObject *obj, QEvent *event);

    void FillThresholdLogger(QTextStream& out, const ProfilerNodeTree *profilerNode);
    void FillProfileTimingWindow(QTreeWidgetItem *qtNode, const ProfilerNodeTree *profilerNode, int numFrames, float frameTotalTimeSecs);
    int ReadProfilingRefreshInterval();
    void RefreshProfilingDataTree(float msecsOccurred);
    void RefreshProfilingDataList(float msecsOccurred);
    void CollectProfilerNodes(ProfilerNodeTree *node, std::vector<const ProfilerNode *> &dst);
    void FillItem(QTreeWidgetItem* item, const Ogre::ResourcePtr& resource, QString drawType);
    void RefreshAssetData(Ogre::ResourceManager& manager, QTreeWidget* widget, QString drawType);
    uint GetNumResources(Ogre::ResourceManager& manager);
    void GetVerticesAndTrianglesFromMesh(Ogre::Mesh* mesh, uint& vertices, uint& triangles);
    void GetMaterialsFromEntity(Ogre::Entity* entity, std::set<Ogre::Material*>& dest);
    void GetTexturesFromMaterials(const std::set<Ogre::Material*>& materials, std::set<Ogre::Texture*>& dest);

    void PopulateOgreSceneTree();

    Framework *framework_;
    int frame_time_update_x_pos_;
    /// If true, profiling data is shown in a tree, otherwise using a flat list.
    bool show_profiler_tree_;
    bool show_unused_;
    bool visibility_;
    QTimer profiler_update_timer_;
    float logThreshold_;
    /// Directory.for log files.
    QDir logDirectory_;

    QTreeWidget *tree_profiling_data_;
    QTreeWidget *treeBulletStats;
    QComboBox *combo_timing_refresh_interval_;
    QTabWidget *tab_widget_;
    QWidget *contents_widget_;
    QLabel *label_frame_time_history_;
    QLabel *label_top_frame_time_;
    QLabel *label_time_per_frame_;
    QLabel *labelTimings;

    QPushButton *push_button_toggle_tree_;
    QPushButton *push_button_collapse_all_;
    QPushButton *push_button_expand_all_;
    QPushButton *push_button_show_unused_;
    QTreeWidget *tree_asset_cache_;
    QTreeWidget *tree_asset_transfers_;
    QTreeWidget *tree_rendertargets_;
    QTreeWidget* tree_texture_assets_;
    QMenu *menu_texture_assets_;
    QTreeWidget* tree_mesh_assets_; 
    QMenu *menu_mesh_assets_;
    QTreeWidget* tree_material_assets_;
    QMenu *menu_material_assets_;
    QTreeWidget* tree_skeleton_assets_;
    QTreeWidget* tree_compositor_assets_;
    QTreeWidget* tree_gpu_assets_;
    QTreeWidget* tree_font_assets_;
    QTextEdit* text_scenecomplexity_;

#ifdef OGREASSETEDITOR_ENABLED
    TexturePreviewEditor* tex_preview_;
#endif
};
