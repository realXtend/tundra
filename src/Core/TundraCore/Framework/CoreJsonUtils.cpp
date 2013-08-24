// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "CoreJsonUtils.h"
#include "CoreStringUtils.h"
#include "LoggingFunctions.h"

#include "QJson/Parser"

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
            data = RemoveLines(data, QStringList() << "//" << "#");
        return Parse(data, ok);
    }

    QVariant Parse(const QByteArray &data, bool *ok)
    {
        QJson::Parser parser;
        QVariant result = parser.parse(data, ok);
        if (ok != 0 && *ok == false)
            LogError(QString("TundraJson::Parse: Error on line %1: %2").arg(parser.errorLine()).arg(parser.errorString()));
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
}
