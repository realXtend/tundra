// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"

#include <QString>
#include <QByteArray>
#include <QVariant>

namespace TundraJson
{
    /// Parse JSON data from a file.
    /** This also function provides ignoring comment lines which can be useful in config files.
        If you are expecting that the file content has comment lines starting with '//' and/or '#' set removeCommentLines to true.
        @see CoreStringUtils::RemoveLines.
        @param File path.
        @param Boolean that will be set to true if processing succeeded, false on failure.
        @param If comment lines should be removed before parsing. 
        @return Data as a QVariant. Use its functions to convert to the correct type eg. toInt(), toBool(), toList() and toMap(). */
    QVariant TUNDRACORE_API ParseFile(const QString &filePath, bool removeCommentLines = false, bool *ok = 0);

    /** @overload
        @param File path.
        @return Data as a QVariant. Use its functions to convert to the correct type eg. toInt(), toBool(), toList() and toMap(). */
    QVariant TUNDRACORE_API ParseFile(const QString &filePath);

    /// Parse JSON data.
    /** @note This requires strict JSON formatting. If you have a file with comment lines use ParseFile with removeCommentLines set to true.
        @param Data string.
        @param Boolean that will be set to true if processing succeeded, false on failure.
        @return Data as a QVariant. Use its functions to convert to the correct type eg. toInt(), toBool(), toList() and toMap(). */
    QVariant TUNDRACORE_API Parse(const QByteArray &data, bool *ok = 0);

    /// Returns values from input data by keys.
    /** @param Data object.
        @param List of keys that you want the values for, if empty all values are returned.
        @param Case sensitivity for matching keys (if provided).
        @return Values as a QVariant list. Use QVariant functions to convert to the correct type eg. toInt(), toBool(), toList() and toMap(). */
    QVariantList TUNDRACORE_API Values(const QVariantMap &data, const QStringList &keys = QStringList(), Qt::CaseSensitivity = Qt::CaseSensitive);

    /// Returns value from input data by key. The match is case-sensitive.
    /** @param Data object.
        @param Values key.
        @param Default value that will be returned if key cannot be found.
        @return Value as a QVariant. Use its functions to convert to the correct type eg. toInt(), toBool(), toList() and toMap(). */
    QVariant TUNDRACORE_API Value(const QVariantMap &data, const QString &key, const QVariant &defaultValue = QVariant());
}
