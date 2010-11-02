/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   InvokeItem.h
 *  @brief  Struct used to storage information about invoked Entity Action or Function call.
 */

#ifndef incl_ECEditorModule_InvokeItem_h
#define incl_ECEditorModule_InvokeItem_h

#include "ECEditorModuleApi.h"
#include "EntityAction.h"

/// Struct used to storage information about invokable Entity Action or Function call.
/** @todo Describe syntax of InvokeItem in the setting file
    @todo Consider storing entity actions as Entity::Exec() function calls.
*/
struct ECEDITOR_MODULE_API InvokeItem
{
    /// Default constructor. Sets type and execTypes member variables to Unknown/Invalid.
    InvokeItem();

    /// Constructor which creates InvokeItem from string read from setting file.
    /** @param settingStr String read from setting file.
    */
    explicit InvokeItem(const std::string &settingStr);

    /// Equality operator. Compares items by objectName, name and parameters.
    bool operator ==(const InvokeItem &rhs) const
    {
        return objectName == rhs.objectName && name == rhs.name && parameters == rhs.parameters;
    }

    /// Inequality operator. Compares items by objectName, name and parameters.
    bool operator !=(const InvokeItem &rhs) const { return !(*this == rhs); }

    /// Less than operator. Compares items by the MRU order number.
    bool operator <(const InvokeItem &rhs) const { return mruOrder < rhs.mruOrder; }

    /// Greater than operator. Compares items by the MRU order number.
    bool operator >(const InvokeItem &rhs) const { return mruOrder > rhs.mruOrder; }

    /// Returns the information contained by this item as one string, e.g. 'void Foo::bar(0.123,abced,true);
    QString ToString() const;

    /// Returns this items information in a form that's suitable to be saved in a setting file.
    std::string ToSetting() const;

    /// Constructs item from string read from setting file.
    /** @param str String read from setting file.
    */
    void FromSetting(const std::string &str);

    /// Type enumeration.
    enum Type
    {
        Unknown = 0, ///< Unknown/invalid.
        Action, ///< Entity Action
        Function ///< Function
    };

    Type type; ///< Type of the item.
    QString objectName; ///< Class name of the object. Always Scene::Entity for entity actions.
    QString name; ///< Name of the function or entity action.
    QString returnType; ///< Return type (functions only)
    EntityAction::ExecutionTypeField execTypes; ///< Execution type (entity actions only).
    QVariantList parameters; ///< "type name - value pair, e.g. "float"-0.123
    int mruOrder; ///< Most recently used order.
};

#endif
