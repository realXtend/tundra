/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   InvokeItem.cpp
    @brief  Struct used to store information about invoked Entity Action or Function call. */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "InvokeItem.h"

#include "CoreException.h"

#include "MemoryLeakCheck.h"

InvokeItem::InvokeItem() :
    type(Unknown),
    execTypes(EntityAction::Invalid),
    mruOrder(0)
{
}

InvokeItem::InvokeItem(const QString &settingStr) :
    type(Unknown),
    execTypes(EntityAction::Invalid),
    mruOrder(0)
{
    FromSetting(settingStr);
}

QString InvokeItem::ToString() const
{
    QString str(name);
    if (type == Action)
        str.prepend('"');
    str.append('(');
    for(int i = 0; i < parameters.size(); ++i)
    {
        // Decorate string values with ".
        if (parameters[i].type() == QVariant::String)
            str.append('"' + parameters[i].toString() + '"');
        else
            str.append(parameters[i].toString());

        if (i < parameters.size() - 1)
            str.append(',');
    }

    str.append(')');
    if (type == Action)
        str.append('"');
    return str;
}

QString InvokeItem::ToSetting() const
{
    QString str;
    str.append(QString::number((int)type));
    if (type == Action)
        str.append('|' + QString::number((int)execTypes));
    if (type == Function)
        str.append('|' + returnType);
    str.append('|'+ objectName);
    str.append('|' + name);
    foreach(QVariant p, parameters)
        str.append('|' + QString::number((int)p.type()) + '|' + p.toString());

    return str;
}

void InvokeItem::FromSetting(const QString &str)
{
    QStringList params = str.split('|');
    if (params.size() < 3)
        return;

    int idx = 0;
    type = (Type)params[idx++].toInt();
    if (type == Action)
        execTypes = (EntityAction::ExecTypeField)params[idx++].toInt();
    if (type == Function)
        returnType = params[idx++];

    objectName = params[idx++];
    name = params[idx++];

    for(; idx < params.size()-1; idx+=2)
    {
        QString type = params[idx];
        QVariant value = params[idx+1];
        bool converted = value.convert((QVariant::Type)type.toInt());
        if (!converted)
        {
            QString err("Failed to parse invoke history: could not convert " + value.toString() + " to QVariant::Type " + type);
            throw Exception(err.toStdString().c_str()); //\ todo: is it really necessary to crash the client if history cannot be parsed?
        }
        parameters.push_back(value);
    }
}
