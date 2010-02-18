// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_DebugStats_TimeProfilerWindow_h
#define incl_DebugStats_TimeProfilerWindow_h

#include <QTreeWidget>
#include <QTimer>
#include <QComboBox>
#include <QTabWidget>
#include <QLabel>
#include <QTreeWidget>
#include <QPushButton>

#include "Framework.h"
#include "UiModule.h"
#include "WorldStream.h"
#include "NetworkMessages/NetInMessage.h"
#include <boost/cstdint.hpp>

class TimeProfilerWindow : public QWidget
{
    Q_OBJECT

    Foundation::Framework *framework_;

    QTreeWidget *treeProfilingData_;
    QComboBox *comboTimingRefreshInterval_;
    QTabWidget *tabWidget_;
    QWidget *contentsWidget_;
    QLabel *labelFrameTimeHistory_;
    QLabel *labelTopFrameTime_;
    QLabel *labelTimePerFrame_;
    QLabel *labelRegionMapCoords_;
    QLabel *labelRegionObjectCapacity_;
    QLabel *labelPidStat_;
    QTreeWidget *treeSimStats_;
    QPushButton *pushButtonToggleTree_;
    QPushButton *pushButtonCollapseAll_;
    QPushButton *pushButtonExpandAll_;
    QPushButton *pushButtonShowUnused_;

    int frameTimeUpdateXPos;

    // If true, profiling data is shown in a tree, otherwise using a flat list.
    bool showProfilerTree;

    bool showUnused;

    QTimer profilerUpdateTimer_;

    ProtocolUtilities::WorldStreamPtr currentWorldStream_;

    void FillProfileTimingWindow(QTreeWidgetItem *qtNode, const Foundation::ProfilerNodeTree *profilerNode);

    int ReadProfilingRefreshInterval();
    void RefreshProfilingDataTree();
    void RefreshProfilingDataList();
    void CollectProfilerNodes(Foundation::ProfilerNodeTree *node, std::vector<const Foundation::ProfilerNode *> &dst);
    void resizeEvent(QResizeEvent *event);

public:
    /// The ctor adds this window to scene, but does not show it.
    explicit TimeProfilerWindow(UiServices::UiModule *uiModule, Foundation::Framework *framework);
    void RedrawFrameTimeHistoryGraph(const std::vector<std::pair<boost::uint64_t, double> > &frameTimes);
    void RedrawFrameTimeHistoryGraphDelta(const std::vector<std::pair<boost::uint64_t, double> > &frameTimes);
    void SetWorldStreamPtr(ProtocolUtilities::WorldStreamPtr worldStream);
    void RefreshSimStatsData(ProtocolUtilities::NetInMessage *simStats);

public slots:
    void RefreshProfilingData();
    void OnProfilerWindowTabChanged(int newPage);
    void RefreshOgreProfilingWindow();
    void RefreshNetworkProfilingData();
    void ToggleTreeButtonPressed();
    void CollapseAllButtonPressed();
    void ExpandAllButtonPressed();
    void ShowUnusedButtonPressed();
};

#endif
