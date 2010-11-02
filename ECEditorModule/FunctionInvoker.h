/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   FunctionInvoker.h
 *  @brief  
 */

#ifndef incl_ECEditorModule_FunctionInvoker_h
#define incl_ECEditorModule_FunctionInvoker_h

#include "ECEditorModuleApi.h"

class IArgumentType;

/// 
class ECEDITOR_MODULE_API FunctionInvoker
{
public:
    /// Default constructor.
    FunctionInvoker() {}

    /// Invokes function using QMetaObject::invokeMethod().
    /** @param obj Object for which the function is invoked.
        @param function Function name, e.g. "SetName"
        @param ret [out] Return value.
        @param params List of parameters, maximum of ten.
        @param errorMsg [out] Pointer to error message.
    */
    void Invoke(QObject *obj, const QString &function, QVariant *ret, const QVariantList &params, QString *errorMsg = 0);

    /// This is an overloaded function. Use for void functions.
    /** @param obj Object for which the function is invoked.
        @param function Function name, e.g. "SetName"
        @param params List of parameters, maximum of ten.
        @param errorMsg [out] Pointer to error message.
    */
    void Invoke(QObject *obj, const QString &function, const QVariantList &params, QString *errorMsg = 0);

    /// Creates argument type list for function @c function of object.@c obj.
    /** @param obj Object.
        @param function Name of the function, e.g. "SetName".
    */
    QList<IArgumentType *> CreateArgumentList(const QObject *obj, const QString &function);

private:
    /// Returns Argument type object for spesific parameter type name.
    /** @param type Type name of the function parameter.
        @return Argument type, or 0 if invalid type name was given.
    */
    IArgumentType *CreateArgumentType(const QString &type);

    /// Creates return value argument type for function @c function of object.@c obj.
    /** @param obj Object.
        @param function Name of the function, e.g. "SetName".
    */
    IArgumentType *CreateReturnValueArgument(const QObject *obj, const QString &function);
};

#endif
