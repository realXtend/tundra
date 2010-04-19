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

class QLabel;

class EC_OpenSimPresence;

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

        /// Destructor.
        ~ParticipantWindow();

        /// Adds user entry to the list.
        /// @param presence EC_OpenSimPresence component
        void AddUserEntry(EC_OpenSimPresence *presence);

        /// Removes user entry from the list.
        /// @param presence EC_OpenSimPresence component
        void RemoveUserEntry(EC_OpenSimPresence *presence);

        /// Adds new custom info field to the list.
        /// @param user_id User ID of the user the info field
        /// @param widget Info widget
        void AddInfoField(const RexUUID &user_id, QWidget *widget);

    private:
        /// Populates the user list when this widget is created
        void PopulateUserList();

        /// Framework pointer.
        Foundation::Framework *framework_;

        /// List of user entry widgets.
        QMap<RexUUID, QWidget *> entries_;

    };
}

#endif
