// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QVariant>

namespace TundraJson
{
    /// Indentation mode for serialization.
    /**
    IndentNone
    @code
    { "foo" : 0, "foo1" : 1, "foo2" : [ { "foo3" : 3, "foo4" : 4 } ] }
    @endcode

    IndentCompact
    @code
    {"foo":0,"foo1":1,"foo2":[{"foo3":3,"foo4":4}]}
    @endcode

    IndentMinimum
    @code
    { "foo" : 0, "foo1" : 1, "foo2" : [
      { "foo3" : 3, "foo4" : 4 }
    ] }
    @endcode

    IndentMedium
    @code
    {
     "foo" : 0, "foo1" : 1, "foo2" : [
      {
       "foo3" : 3, "foo4" : 4
      }
     ]
    }
    @endcode

    IndentFull
    @code
    {
     "foo" : 0,
     "foo1" : 1,
     "foo2" : [
      {
       "foo3" : 3,
       "foo4" : 4
      }
     ]
    }
    @endcode */
    enum IndentMode
    {
        IndentNone,
        IndentCompact,
        IndentMinimum,
        IndentMedium,
        IndentFull
    };

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

    /// Serializes QVariant to JSON and store it to file.
    /** @param File to store the serialization results.
        @param QVariant to serialize.
        @param Indentation mode.
        @param Boolean that will be set to true if serialization succeeded, otherwise false.
        @return False if file could not be written to or serialization failed, otherwise true. */
    bool TUNDRACORE_API SerializeToFile(const QString &filePath, const QVariant &variant, IndentMode indentMode = IndentCompact);

    /// Serializes QVariant to JSON.
    /** @param QVariant to serialize.
        @param Indentation mode.
        @param Boolean that will be set to true if serialization succeeded, otherwise false.
        @return JSON string data. If .isEmpty() the input was empty, if .isNull() there was a parsing error. */
    QByteArray TUNDRACORE_API Serialize(const QVariant &variant, IndentMode indentMode = IndentCompact, bool *ok = 0);

    /// Serializes QObject to JSON.
    /** @param QObject to serialize.
        @param QObject property names that should be ignored in serialization. eg. "objectName".
        @param Indentation mode.
        @param Boolean that will be set to true if serialization succeeded, otherwise false.
        @return JSON string data. If .isEmpty() the input was empty, if .isNull() there was a parsing error. */
    QByteArray TUNDRACORE_API Serialize(const QObject *object, const QStringList &ignoredPropertyNames, IndentMode indentMode = IndentCompact, bool *ok = 0);

    /// Converts QVariantMap data to QObject properties.
    /** @param QVariant input data.
        @param Target QObject ptr. */
    void TUNDRACORE_API ToObject(const QVariantMap &variant, QObject *target);

    /// Converts QObject properties to QVariantMap.
    /** @param QObject to convert.
        @param QObject property names that should be ignored in serialization. eg. "objectName".
        @return Data map. */
    QVariantMap TUNDRACORE_API ToVariant(const QObject *object, const QStringList &ignoredPropertyNames);
}
