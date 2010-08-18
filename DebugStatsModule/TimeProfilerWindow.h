// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_DebugStats_TimeProfilerWindow_h
#define incl_DebugStats_TimeProfilerWindow_h

#include "CoreTypes.h"

#include <boost/cstdint.hpp>

#include <QWidget>
#include <QTimer>
#include <QDir>
#include <QMap>
#include <QString>
#include <Ogre.h>

class QTreeWidget;
class QComboBox;
class QTabWidget;
class QLabel;
class QTreeWidget;
class QTreeWidgetItem;
class QTextEdit;
class QPushButton;
class QTextStream;

namespace ProtocolUtilities
{
    class NetInMessage;
    class NetOutMessage;
    class WorldStream;
    typedef boost::shared_ptr<WorldStream> WorldStreamPtr;
}

namespace Foundation
{
    class Framework;
    class ProfilerNodeTree;
    class ProfilerNode;
}

namespace DebugStats
{
    /// Provides various profiling data, performance statistics and tools for Naali.
    class TimeProfilerWindow : public QWidget
    {
        Q_OBJECT

    public:
        /// The ctor adds this window to scene, but does not show it.
        explicit TimeProfilerWindow(Foundation::Framework *fw);
        void RedrawFrameTimeHistoryGraph(const std::vector<std::pair<boost::uint64_t, double> > &frameTimes);
        void RedrawFrameTimeHistoryGraphDelta(const std::vector<std::pair<boost::uint64_t, double> > &frameTimes);
        void SetWorldStreamPtr(ProtocolUtilities::WorldStreamPtr worldStream);
        void RefreshSimStatsData(ProtocolUtilities::NetInMessage *simStats);
        void DoThresholdLogging();

        /// Writes brief log entry about network message to log file, if logging is enabled.
        /// @param msg Inbound SLUDP message
        void LogNetInMessage(const ProtocolUtilities::NetInMessage *msg);

        /// Writes brief log entry about network message to log file, if logging is enabled.
        /// @param msg Outbound SLUDP message
        void LogNetOutMessage(const ProtocolUtilities::NetOutMessage *msg);

    public slots:
        void RefreshProfilingData();
        void OnProfilerWindowTabChanged(int newPage);
        void RefreshOgreProfilingWindow();
        void RefreshNetworkProfilingData();
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
        /// Sets network logging on/off
        void SetNetworkLogging(int value);

    protected:
        void resizeEvent(QResizeEvent *event);

    private:
        void FillThresholdLogger(QTextStream& out, const Foundation::ProfilerNodeTree *profilerNode);
        void FillProfileTimingWindow(QTreeWidgetItem *qtNode, const Foundation::ProfilerNodeTree *profilerNode);
        int ReadProfilingRefreshInterval();
        void RefreshProfilingDataTree();
        void RefreshProfilingDataList();
        void CollectProfilerNodes(Foundation::ProfilerNodeTree *node, std::vector<const Foundation::ProfilerNode *> &dst);
        void FillItem(QTreeWidgetItem* item, const Ogre::ResourcePtr& resource);
        void RefreshAssetData(Ogre::ResourceManager& manager, QTreeWidget* widget);
        uint GetNumResources(Ogre::ResourceManager& manager);
        void GetVerticesAndTrianglesFromMesh(Ogre::Mesh* mesh, uint& vertices, uint& triangles);
        void GetMaterialsFromEntity(Ogre::Entity* entity, std::set<Ogre::Material*>& dest);
        void GetTexturesFromMaterials(const std::set<Ogre::Material*>& materials, std::set<Ogre::Texture*>& dest);
        
        void DumpNetworkSummary(QTextStream* log = 0);
        
        struct NetworkLogData
         {
            NetworkLogData() : bytes(0), packages(0) {}

            unsigned int bytes;
            unsigned int packages;
        };
        
        QMap<QString, NetworkLogData > mapNetInData_;
        QMap<QString, NetworkLogData > mapNetOutData_;


        Foundation::Framework *framework_;
        int frame_time_update_x_pos_;
        /// If true, profiling data is shown in a tree, otherwise using a flat list.
        bool show_profiler_tree_;
        bool show_unused_;
        bool visibility_;
        QTimer profiler_update_timer_;
        ProtocolUtilities::WorldStreamPtr world_stream_;
        float logThreshold_;
        /// Directory.for log files.
        QDir logDirectory_;

        QTreeWidget *tree_profiling_data_;
        QComboBox *combo_timing_refresh_interval_;
        QTabWidget *tab_widget_;
        QWidget *contents_widget_;
        QLabel *label_frame_time_history_;
        QLabel *label_top_frame_time_;
        QLabel *label_time_per_frame_;
        QLabel *label_region_map_coords_;
        QLabel *label_region_object_capacity_;
        QLabel *label_pid_stat_;
        QTreeWidget *tree_sim_stats_;
        QPushButton *push_button_toggle_tree_;
        QPushButton *push_button_collapse_all_;
        QPushButton *push_button_expand_all_;
        QPushButton *push_button_show_unused_;
        QTreeWidget *tree_asset_cache_;
        QTreeWidget *tree_asset_transfers_;
        QTreeWidget *tree_rendertargets_;
        QTreeWidget* tree_texture_assets_;
        QTreeWidget* tree_mesh_assets_; 
        QTreeWidget* tree_material_assets_;
        QTreeWidget* tree_skeleton_assets_;
        QTreeWidget* tree_compositor_assets_;
        QTreeWidget* tree_gpu_assets_;
        QTreeWidget* tree_font_assets_;
        QTextEdit* text_scenecomplexity_;
    };
}

#endif
