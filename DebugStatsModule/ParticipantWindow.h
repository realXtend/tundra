/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ParticipantWindow.h
 *  @brief  ParticipantWindow Shows list of users in the world.
 */

#ifndef incl_DebugStatsModule_ParticipantWindow_h
#define incl_DebugStatsModule_ParticipantWindow_h

#include <QWidget>

class QVBoxLayout;

namespace Foundation
{
    class Framework;
}

namespace DebugStats
{
    class ParticipantWindow : public QWidget
    {
        Q_OBJECT

    public:
        /// Constructor
        /// @param fw Framework.
        /// @param parent Parent widget.
        ParticipantWindow(Foundation::Framework *fw, QWidget *parent = 0);

        ///
        ~ParticipantWindow();

        ///
        void PopulateUsernameList();

    private:
        ///
        Foundation::Framework *framework_;

        ///
        QVBoxLayout *usernameLayout_;
    };
}

#endif
