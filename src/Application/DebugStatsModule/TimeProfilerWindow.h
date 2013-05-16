// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "CoreTypes.h"

#include <QWidget>
#include <QTimer>
#include <QDir>
#include <QMap>
#include <QString>
#include <Ogre.h>

#include "ui_ProfilerWindow.h"

class Framework;
class ProfilerNodeTree;
class ProfilerNode;

/// Provides various profiling data, performance statistics and tools.
class TimeProfilerWindow : public QWidget
{
    Q_OBJECT

public:
    /// The ctor adds this window to scene, but does not show it.
    explicit TimeProfilerWindow(Framework *fw, QWidget *parent = 0);
    ~TimeProfilerWindow();

    // Graph updates.
    void ResizeFrameTimeHistoryGraph();
    void RedrawFrameTimeHistoryGraph(const std::vector<std::pair<u64, double> > &frameTimes);
    void RedrawFrameTimeHistoryGraphDelta(const std::vector<std::pair<u64, double> > &frameTimes);
    
    // Threshold logging
    void DoThresholdLogging();

public slots:
    /// Set visibility
    /** @note This does not modify widget visibility but and internal one. */
    void SetVisibility(bool visibility);
    
    /// Refresh currently visible view.
    void Refresh();

private slots:    
    // Refresh tab widgets.
    void RefreshProfilerTabWidget(int pageIndex = -1);
    void RefreshOgreTabWidget(int pageIndex = -1);

    // Top level pages.
    void RefreshTimingPage();
    void RefreshBulletPage();
    void RefreshScriptsPage();
    void RefreshAssetsPage();

    // Ogre pages.
    void RefreshOgreOverviewPage();
    void RefreshOgreSceneComplexityPage();
    void RefreshOgreSceneHierarchyPage();
    void RefreshOgreRenderTargetPage();

    // Log file dumping.
    void DumpOgreResourceStatsToFile();
    void DumpSceneComplexityToFile();   
    
    // UI action handlers.
    void ChangeLoggerThreshold();
    void OnTimingViewModeChanged();
    void OnCollapseAllPressed();
    void OnExpandAllPressed();
    void OnShowUnusedPressed();
    
    // Ogre data tree refresh.
    void ArrangeOgreDataTree();
    void RefreshOgreDataTree(Ogre::ResourceManager& manager, QTreeWidget* widget, QString drawType);
    
    // Assets.
    void ShowAsset(QTreeWidgetItem* item, int column);
    void CopyTextureAssetName();
    void CopyMeshAssetName();
    void CopyMaterialAssetName();

    // Main profiler block refresh internal.
    int ReadProfilingRefreshInterval();
    void OnTimingIntervalChanged();
    
    // Load previous settings from config.
    void LoadSettings();
    
    // Save current settings to a config.
    void SaveSettings();

signals:
    /// Emitted on show/hide.
    void Visible(bool visible);

protected:
    // QWidget overrides.
    void resizeEvent(QResizeEvent *event);
    void showEvent(QShowEvent *event);
    void hideEvent(QHideEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    
private:
    void FillThresholdLogger(QTextStream& out, const ProfilerNodeTree *profilerNode);
    void FillProfileTimingWindow(QTreeWidgetItem *qtNode, const ProfilerNodeTree *profilerNode, int numFrames, float frameTotalTimeSecs);

    void CollectProfilerNodes(ProfilerNodeTree *node, std::vector<const ProfilerNode *> &dst);
    void RefreshProfilingDataTree(float msecsOccurred);
    void RefreshProfilingDataList(float msecsOccurred);
    
    void FillOgreDataTreeItem(QTreeWidgetItem* item, const Ogre::ResourcePtr& resource, QString drawType);

    uint GetNumResources(Ogre::ResourceManager& manager);
    void GetVerticesAndTrianglesFromMesh(Ogre::Mesh* mesh, uint& vertices, uint& triangles);
    void GetMaterialsFromEntity(Ogre::Entity* entity, std::set<Ogre::Material*>& dest);
    void GetTexturesFromMaterials(const std::set<Ogre::Material*>& materials, std::set<Ogre::Texture*>& dest);
    
    QStringList TimingFilters();
    bool IsTimingItemFiltered(const QString &name, const QStringList &filters);

    /// Framework ptr.
    Framework *framework_;
    
    // Ui.
    Ui::ProfilerWindow ui_;

    /// Log file directory.
    QDir logDirectory_;

    // Right click context menu.
    QMenu *contextMenu_;

    // Main update timer.
    QTimer updateTimer_;

    // Is profiler currently visible (afaik can be false even if widget is showing).
    bool visibility_;
    
    float logThreshold_;
    int frameFimeUpdatePosX_;
};
