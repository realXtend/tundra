/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   AttributeChangeType.h
 *  @brief  Dummy class containing enumeration of attribute/component change types for replication.
 *          This is done in separate file in order to overcome cyclic inclusion dependency
 *          between AttributeInterface and ComponentInterface.
 */

#ifndef incl_Interfaces_AttributeChangeType_h
#define incl_Interfaces_AttributeChangeType_h

#include <QObject>

//! Dummy class containing enumeration of attribute/component change types for replication.
class AttributeChange : public QObject
{
    Q_OBJECT
    Q_ENUMS(ChangeType)

public:
    //! Enumeration of attribute/component change types for replication
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
