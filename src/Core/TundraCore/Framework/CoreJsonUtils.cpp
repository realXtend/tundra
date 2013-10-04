// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "CoreJsonUtils.h"
#include "CoreStringUtils.h"
#include "LoggingFunctions.h"

#include <qjson/parser.h>
#include <qjson/serializer.h>
#include <qjson/qobjecthelper.h>

#include <QFile>

namespace TundraJson
{  
    QVariant ParseFile(const QString &filePath)
    {
        bool ok = false;
        return ParseFile(filePath, false, &ok);
    }

    QVariant ParseFile(const QString &filePath, bool removeCommentLines, bool *ok)
    {
        QFile file(filePath);
        if (!file.open(QFile::ReadOnly))
            return QVariant();
        QByteArray data = file.readAll();
        file.close();
        if (removeCommentLines)
        {
            // If we are removing lines, we have to parse here to report correct error line!
            // If this wont be reported right, startup and rendering config errors in console
            // are going to be very confusing to people as the line numbers won't match the file content.
            uint removedLines = 0;
            data = RemoveLines(data, QStringList() << "//" << "#", &removedLines);
            QJson::Parser parser;
            QVariant result = parser.parse(data, ok);
            if (ok != 0 && *ok == false)
                LogError(QString("TundraJson::Parse: Error on line %1: %2")
                    .arg(parser.errorLine()+removedLines).arg(parser.errorString()));
            return result;
        }
        return Parse(data, ok);
    }

    QVariant Parse(const QByteArray &data, bool *ok)
    {
        QJson::Parser parser;
        QVariant result = parser.parse(data, ok);
        if (ok != 0 && *ok == false)
            LogError(QString("TundraJson::Parse: Error on line %1: %2")
                .arg(parser.errorLine()).arg(parser.errorString()));
        return result;
    }

    QVariantList Values(const QVariantMap &data, const QStringList &keys, Qt::CaseSensitivity sensitivity)
    {
        QVariantList values;
        foreach(const QString &key, data.keys())
        {
            if (keys.isEmpty() || keys.contains(key, sensitivity))
                values << data[key];
        }
        return values;
    }

    QVariant Value(const QVariantMap &data, const QString &key, const QVariant &defaultValue)
    {
        return data.value(key, defaultValue);
    }

    bool SerializeToFile(const QString &filePath, const QVariant &variant, IndentMode indentMode)
    {
        bool ok = false;
        QByteArray data = Serialize(variant, indentMode, &ok);
        if (!ok)
            return false;

        QFile file(filePath);
        if (file.open(QFile::WriteOnly))
        {
            file.write(data);
            file.close();
            return true;
        }
        return false;
    }

    QByteArray Serialize(const QVariant &variant, IndentMode indentMode, bool *ok)
    {
        QJson::Serializer serializer;
        serializer.setIndentMode(static_cast<QJson::IndentMode>(indentMode));
        QByteArray data = serializer.serialize(variant, ok);
        if (ok != 0 && *ok == false)
            LogError(QString("TundraJson::Serialize: %1").arg(serializer.errorMessage()));
        return data;
    }

    QByteArray Serialize(const QObject *object, const QStringList &ignoredPropertyNames, IndentMode indentMode, bool *ok)
    {
        if (object)
            return Serialize(ToVariant(object, ignoredPropertyNames), indentMode, ok);
        return QByteArray();
    }

    void ToObject(const QVariantMap &variant, QObject *target)
    {
        if (target)
            QJson::QObjectHelper::qvariant2qobject(variant, target);
    }

    QVariantMap ToVariant(const QObject *object, const QStringList &ignoredPropertyNames)
    {
        if (object)
            return QJson::QObjectHelper::qobject2qvariant(object, ignoredPropertyNames);
        return QVariantMap();
    }

    bool IsList(const QVariant &value)
    {
        QVariant::Type t = value.type();
        return (t == QVariant::StringList || t == QVariant::List);
    }

    bool IsMap(const QVariant &value)
    {
        QVariant::Type t = value.type();
        return (t == QVariant::Map || t == QVariant::Hash);
    }

    bool IsNumber(const QVariant &value)
    {
        QVariant::Type t = value.type();
        return (t == QVariant::Int || t == QVariant::UInt ||
                t == QVariant::LongLong || t == QVariant::ULongLong ||
                t == QVariant::Double);
    }

    QVariant ValueForAnyKey(const QVariantMap &map, const QStringList &acceptedKeys, const QVariant &defaultValue)
    {
        foreach(const QString &key, acceptedKeys)
            if (map.contains(key))
                return map[key];
        return defaultValue;
    }
}
