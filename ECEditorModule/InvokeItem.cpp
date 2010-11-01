/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   InvokeItem.h
 *  @brief  
 */

#include "StableHeaders.h"
#include "InvokeItem.h"

QString InvokeItem::ToString() const
{
    QString str;
    if (type == Function)
        str.append("Function: ");
    else if(type == Action)
        str.append("Action: ");
    else
        str.append("Unknown: ");

    str.append(name);
    str.append('(');

    for(int i = 0; i < parameters.size(); ++i)
    {
        // Decorate string values with ".
        if (parameters[i].second.type() == QVariant::String)
            str.append('"' + parameters[i].second.toString() + '"');
        else
            str.append(parameters[i].second.toString());

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
    str.append('|');
    str.append(name);
    foreach(Parameter p, parameters)
        str.append('|' + p.first + '|' + p.second.toString());

    return str.toStdString();
}

void InvokeItem::FromSetting(const std::string &str)
{
    QStringList params = QString(str.c_str()).split('|');
    if (params.size() < 2)
        return;

    int idx = 0;
    type = (Type)params[idx++].toInt();
    if (type == Action)
        execTypes = (EntityAction::ExecutionTypeField)params[idx++].toInt();
    name = params[idx++];

    for(idx; idx < params.size(); ++idx)
        if (idx+1 < params.size())
            parameters.push_back(qMakePair(params[idx], QVariant(params[idx+1])));
//        else
//            parameters.push_back(qMakePair(params[idx], QVariant()));
}
