// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_AssetReference_h
#define incl_Interfaces_AssetReference_h

#include <QString>
#include <QMetaType>

/// Reference to an asset
struct AssetReference
{
    /// Default constructor.
    AssetReference() {}

    /// Constructs asset refence with spesific ID and type.
    /** @param i ID of the reference
        @param t Type of the asset reference.
    */
    AssetReference(const QString &i, const QString &t) : id(i), type(t) {}

    /// Returns true if other asset reference is equal to this asset reference.
    bool operator == (const AssetReference &rhs) const { return ((this->id == rhs.id) && (this->type == rhs.type)); }

    /// Returns true if other asset reference is inequal to this asset reference.
    bool operator != (const AssetReference &rhs) const { return !(*this == rhs); }

    /// Asset ID
    QString id;

    /// Asset type
    QString type;
};

Q_DECLARE_METATYPE(AssetReference)

#endif
