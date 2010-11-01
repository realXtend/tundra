/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   InvokeItem.h
 *  @brief  
 */

#ifndef incl_ECEditorModule_InvokeItem_h
#define incl_ECEditorModule_InvokeItem_h

#include "ECEditorModuleApi.h"
#include "EntityAction.h"

#include <QString>
#include <QPair>
#include <QList>
#include <QVariant>

///
enum InvokeItemType
{
    Unknown = 0, ///< Unknown/invalid.
    Action, ///< Entity Action
    Function ///< Function
};

/// Parameter is pair of type name as QString and value as QVariant.
typedef QPair<QString, QVariant> Parameter;

///
struct ECEDITOR_MODULE_API InvokeItem
{
    /// Less than operator. Compares entries by the MRU order number.
    bool operator <(const InvokeItem &rhs) const { return mruOrder < rhs.mruOrder; }

    /// Returns the information contained by this item as one string, e.g. 'Function: Foo::bar(0.123,abced,true);
    QString ToString() const;

    /// Returns this items information in a form that's suitable to be saved in a setting file.
    std::string ToSetting() const;

    /// Constructs item from string read from setting file.
    /** @param str String read from setting file.
    */
    void FromSetting(const std::string &str);

    EntityAction::ExecutionTypeField execTypes; ///< Execution type (entity actions only).
    InvokeItemType type; ///< Type of the item.
    QString name; ///< Name of the function or entity action.
    QList<Parameter> parameters; ///< "type name - value pair, e.g. "float"-0.123
    int mruOrder; ///< Most recently used order.
};

#endif
