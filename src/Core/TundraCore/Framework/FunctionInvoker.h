/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   FunctionInvoker.h
    @brief  Utility class which wraps QMetaObject::invokeMethod() functionality with more user-friendly API. */

#pragma once

#include "TundraCoreApi.h"

class IArgumentType;

/// Utility class which wraps QMetaObject::invokeMethod() functionality with more user-friendly API.
class TUNDRACORE_API FunctionInvoker
{
public:
    /// Invokes function using QMetaObject::invokeMethod().
    /** @param obj Object for which the function is invoked.
        @param function Function name, e.g. "SetName"
        @param params List of parameters, maximum of ten.
        @param ret [out] Return value.
        @param errorMsg [out] Pointer to error message. */
    static void Invoke(QObject *obj, const QString &function, const QVariantList &params, QVariant *ret = 0, QString *errorMsg = 0);
    static void Invoke(QObject *obj, const QString &function, const QStringList &params, QVariant *ret = 0, QString *errorMsg = 0); ///< @overload

    /// Creates argument type list for function of object @c obj with the signature @c signature.
    /** @param obj Object.
        @param signature of the function, e.g. "SetName(QString)". */
    static QList<IArgumentType *> CreateArgumentList(const QObject *obj, const QString &signature);

    /// Returns number of arguments for function of object @c obj with the signature @c signature.
    /** @param obj Object.
        @param signature of the function, e.g. "SetName(QString)".
        @return -1 if fuction with signature was not found, otherwise the number of required arguments. */
    static int NumArgsForFunction(const QObject *obj, const QString &signature);

private:
    /// Returns Argument type object for spesific parameter type name.
    /** @param type Type name of the function parameter.
        @return Argument type, or 0 if invalid type name was given. */
    static IArgumentType *CreateArgumentType(const QString &type);

    /// Creates return value argument type for function @c function of object.@c obj.
    /** @param obj Object.
        @param function Name of the function, e.g. "SetName". */
    static IArgumentType *CreateReturnValueArgument(const QObject *obj, const QString &function);

    static void Invoke(QObject *obj, const QString &function, QList<IArgumentType *> &args, QVariant *ret, QString *errorMsg);
};
