/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ParticipantWindow.h
 *  @brief  ParticipantWindow shows list of users in the world.
 */

#ifndef incl_DebugStatsModule_ParticipantWindow_h
#define incl_DebugStatsModule_ParticipantWindow_h

#include "RexUUID.h"

#include <QWidget>
#include <QMap>

class QVBoxLayout;
class QLabel;

namespace Foundation
{
    class Framework;
}

namespace RexLogic
{
    class EC_OpenSimPresence;
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

        /// Destructor.
        ~ParticipantWindow();

public slots:
        ///
        void PopulateUsernameList();

        ///
        void AddUser(RexLogic::EC_OpenSimPresence *presence);

        ///
        void RemoveUser(RexLogic::EC_OpenSimPresence *presence);

    private:
        ///
        Foundation::Framework *framework_;

        ///
        QVBoxLayout *usernameLayout_;

        ///
        QMap<RexUUID, QLabel *> users_;
    };
}

#endif
