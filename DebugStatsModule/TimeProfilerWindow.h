// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_DebugStats_TimeProfilerWindow_h
#define incl_DebugStats_TimeProfilerWindow_h

#include "CoreTypes.h"

#include <boost/cstdint.hpp>

#include <QWidget>
#include <QTimer>

class QTreeWidget;
class QComboBox;
class QTabWidget;
class QLabel;
class QTreeWidget;
class QTreeWidgetItem;
class QPushButton;

namespace ProtocolUtilities
{
    class WorldStream;
    typedef boost::shared_ptr<WorldStream> WorldStreamPtr;
}

namespace ProtocolUtilities
{
    class NetInMessage;
}

namespace UiServices
{
    class UiModule;
}

namespace DebugStats
{
    class DebugStatsModule;
}

namespace Foundation
{
    class Framework;
    class ProfilerNodeTree;
    class ProfilerNode;
}

namespace DebugStats
{
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

    public slots:
        void RefreshProfilingData();
        void OnProfilerWindowTabChanged(int newPage);
        void RefreshOgreProfilingWindow();
        void RefreshNetworkProfilingData();
        void RefressTextureProfilingData();
        void ToggleTreeButtonPressed();
        void CollapseAllButtonPressed();
        void ExpandAllButtonPressed();
        void ShowUnusedButtonPressed();
        void RefreshAssetProfilingData();
        void DumpOgreResourceStatsToFile();

    protected:
        void resizeEvent(QResizeEvent *event);

    private:
        Foundation::Framework *framework_;
        DebugStats::DebugStatsModule *owner_;

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

        int frame_time_update_x_pos_;

        // If true, profiling data is shown in a tree, otherwise using a flat list.
        bool show_profiler_tree_;

        bool show_unused_;

        QTimer profiler_update_timer_;

        ProtocolUtilities::WorldStreamPtr current_world_stream_;

        void FillProfileTimingWindow(QTreeWidgetItem *qtNode, const Foundation::ProfilerNodeTree *profilerNode);

        int ReadProfilingRefreshInterval();
        void RefreshProfilingDataTree();
        void RefreshProfilingDataList();
        void CollectProfilerNodes(Foundation::ProfilerNodeTree *node, std::vector<const Foundation::ProfilerNode *> &dst);

        QTreeWidget *tree_texture_assets_;
    };
}

#endif
