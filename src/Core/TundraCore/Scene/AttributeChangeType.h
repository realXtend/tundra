/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   AttributeChangeType.h
    @brief  Dummy class containing enumeration of attribute/component change types for replication.
            This is done in separate file in order to overcome cyclic inclusion dependency
            between IAttribute and IComponent. */

#pragma once

#include "TundraCoreApi.h"

#include <QObject>

/// Dummy class containing enumeration of attribute/component change types for replication.
class TUNDRACORE_API AttributeChange : public QObject
{
    Q_OBJECT
    Q_ENUMS(Type)

public:
    /// Enumeration of attribute/component change types for replication
    enum Type
    {
        /// Use the current sync method specified in the IComponent this attribute is part of
        Default = 0,
        /// The value will be changed, but no notifications will be sent (even locally). This
        /// is useful when you are doing batch updates of several attributes at a time and want to minimize
        /// the amount of re-processing that is done.
        Disconnected,
        /// The value change will be signalled locally immediately after the change occurs, but
        /// it is not sent to the network.
        LocalOnly,
        /// Replicate: After changing the value, the change will be signalled locally and this change is
        /// transmitted to the network as well.
        Replicate
    };
};
