/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   AttributeChangeType.h
 *  @brief  Dummy class containing enumeration of attribute/component change types for replication.
 *          This is done in separate file in order to overcome cyclic inclusion dependency
 *          between IAttribute and IComponent.
 */

#ifndef incl_Interfaces_AttributeChangeType_h
#define incl_Interfaces_AttributeChangeType_h

#include <QObject>

//! Dummy class containing enumeration of attribute/component change types for replication.
class AttributeChange : public QObject
{
    Q_OBJECT
    Q_ENUMS(Type)

public:
    //! Enumeration of attribute/component change types for replication
    ///\todo These types will be changed to the following:
    /// Instead of marking the change from the "who did the last change"
    /// perspective, these will be changed to a "who should get notified about this change." perspective:
    /// - Default: Use the current sync method specified in the IComponent this attribute is part of.
    /// - Disconnected: The value will be changed, but no notifications will be sent (even locally). This
    ///   is useful when you are doing batch updates of several attributes at a time and want to minimize
    ///   the amount of re-processing that is done.
    /// - LocalOnly: The value change will be signalled locally immediately after the change occurs, but
    ///   it is not sent to the network.
    /// - Replicate: After changing the value, the change will be signalled locally and this change is
    ///   transmitted to the network as well.
    /// -jj.
    enum Type
    {
        //! No change: attribute/component is up to date
        None = 0,
        //! Local change that should be replicated to server
        Local,
        //! Local change that should not be replicated to server
        LocalOnly,
        //! Change that came from network
        Network
    };
};

#endif
