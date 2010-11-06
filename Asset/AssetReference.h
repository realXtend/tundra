// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_AssetReference_h
#define incl_Interfaces_AssetReference_h

#include <QString>
#include <QMetaType>

/// Represents a reference to an asset. This structure can be used as a parameter type to an EC attribute.
struct AssetReference
{
    AssetReference() {}

    /// Constructs an asset reference pointing to the given asset.
    /** @param ref_ The URL of the asset to point to, e.g. "file://myasset.mesh", or "http://www.website.com/texture.png".
        @param type_ The type of the asset that is being pointed to. If the file name suffix of the URL uniquely identifies the asset type,
            this field may be left blank. */
    AssetReference(const QString &ref_, const QString &type_)
    :ref(ref_), type(type_) {}

    bool operator == (const AssetReference &rhs) const { return (this->ref == rhs.ref && this->type == rhs.type); }

    bool operator != (const AssetReference &rhs) const { return !(*this == rhs); }

    /// Specifies the URL of the asset that is being pointed to.
    QString ref;

    /// Specifies the data type of the asset, like "Mesh", "Texture", or "Script". If the URL specifies the type, this field may be left blank.
    QString type;
};

Q_DECLARE_METATYPE(AssetReference)

#endif
