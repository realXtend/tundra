/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   Action.h
 *  @brief  Represent an executable command on an Entity.
 *
 *          Components (or other instances) can register to these actions by using Entity::ConnectAction().
 *          Actions allow more complicated in-world logic to be built in slightly more data-driven fashion.
 *          Actions cannot be created directly, they're created by Entity::RegisterAction().
 */

#ifndef incl_SceneManager_Action_h
#define incl_SceneManager_Action_h

#include "CoreTypes.h"

#include <QObject>

namespace Scene
{
    class Entity;

    /** Represent an executable command on an Entity.

        Components (or other instances) can register to these actions by using Entity::ConnectAction().
        Actions allow more complicated in-world logic to be built in slightly more data-driven fashion.
        Actions cannot be created directly, they're created by Entity::RegisterAction().

        @ingroup Scene_group
    */
    class Action : public QObject
    {
        Q_OBJECT

        friend class Entity;

    public:
        /// Destructor.
        ~Action() {}

        /// Returns name of the action.
        QString Name() const { return name_; }

    signals:
        /** Emitted when action is triggered.
            @param param1 1st parameter for the action, if applicable.
            @param param2 2nd parameter for the action, if applicable.
            @param param3 3rd parameter for the action, if applicable.
            @param params Rest of the parameters, if applicable.
        */
        void Triggered(const QString &param1, const QString &param2, const QString &param3, const QStringVector &params);

    private:
        /** Constructor.
            @param name Name of the action.
        */
        explicit Action(const QString &name ) : name_(name) {}

        /** Triggers this action i.e. emits the Triggered signal.
            @param param1 1st parameter for the action, if applicable.
            @param param2 2nd parameter for the action, if applicable.
            @param param3 3rd parameter for the action, if applicable.
            @param params Rest of the parameters, if applicable.
        */
        void Trigger(const QString &param1 = "", const QString &param2 = "", const QString &param3 = "", const QStringVector &params = QStringVector())
        {
            emit Triggered(param1, param2, param3, params);
        }

        /// Name of the action.
        QString name_;
    };
}

#endif
