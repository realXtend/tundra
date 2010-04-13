/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ParticipantWindow.h
 *  @brief  ParticipantWindow
*/

#ifndef incl_DebugStatsModule_ParticipantWindow_h
#define incl_DebugStatsModule_ParticipantWindow_h

#include <QWidget>

namespace DebugStats
{
    class ParticipantWindow : public QWidget
    {
        Q_OBJECT

    public:
        ParticipantWindow(QWidget *parent = 0);
        ~ParticipantWindow();
    };
}

#endif
