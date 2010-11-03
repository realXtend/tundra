/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   InvokeItem.cpp
 *  @brief  Struct used to storage information about invoked Entity Action or Function call.
 */

#include "StableHeaders.h"
#include "InvokeItem.h"

InvokeItem::InvokeItem() : type(Unknown), execTypes(EntityAction::Invalid), mruOrder(0)
{
}

InvokeItem::InvokeItem(const std::string &settingStr)
{
    FromSetting(settingStr);
}

QString InvokeItem::ToString() const
{
    QString str;

    if (type == Function)
        str.append(returnType + ' ');
    str.append(name);
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
    return str;
}

std::string InvokeItem::ToSetting() const
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

    return str.toStdString();
}

void InvokeItem::FromSetting(const std::string &str)
{
    QStringList params = QString(str.c_str()).split('|');
    if (params.size() < 3)
        return;

    int idx = 0;
    type = (Type)params[idx++].toInt();
    if (type == Action)
        execTypes = (EntityAction::ExecutionTypeField)params[idx++].toInt();
    if (type == Function)
        returnType = params[idx++];

    objectName = params[idx++];
    name = params[idx++];

    for(idx; idx < params.size(); idx+=2)
    {
        QString type = params[idx];
        QVariant value = params[idx+1];
        bool converted = value.convert((QVariant::Type)type.toInt());
        if (!converted)
        {
            QString err("Failed to parse invoke history: could not convert" + value.toString() + "to QVariant::Type " + type);
            throw Exception(err.toStdString().c_str());
        }
        parameters.push_back(value);
    }
}
