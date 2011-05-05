// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetReference_h
#define incl_Asset_AssetReference_h

#include <QString>
#include <QMetaType>
#include <QVariantList>

#include <cassert>

/// Represents a reference to an asset. This structure can be used as a parameter type to an EC attribute.
struct AssetReference
{
public:
    /// Default constructor
    AssetReference() {}

    /// Constructs an asset reference pointing to the given asset.
    /// @param reference The URL of the asset to point to, e.g. "local://myasset.mesh", or "http://www.website.com/texture.png".
    explicit AssetReference(const QString &reference) : ref(reference) {}

    /// @note This form of a ctor should not be used, since asset references can contain unicode characters, which a std::string cannot represent.
    explicit AssetReference(const std::string &reference) : ref(QString::fromStdString(reference)) {}

    /// @note This form of a ctor should not be used, since asset references can contain unicode characters, which a std::string cannot represent.
    explicit AssetReference(const char *reference) : ref(reference) {}

    AssetReference(const QString &reference, const QString &type_) : ref(reference), type(type_) {}

    bool operator ==(const AssetReference &rhs) const { return this->ref == rhs.ref; }

    bool operator !=(const AssetReference &rhs) const { return !(*this == rhs); }

    bool operator <(const AssetReference &rhs) const { return ref < rhs.ref; }

    /// Specifies the URL of the asset that is being pointed to.
    QString ref;
    /// Specifies the type of the asset to load from that URL. If "", the type is interpreted directly from the ref string.
    /// Not all asset types can support this kind of interpretation. For example, avatar assets are of type .xml, which can
    /// only be distinguished from generic xml files by explicitly specifying the type here.
    QString type;
};

/// Represents list of asset references.
struct AssetReferenceList
{
    /// Default constructor.
    AssetReferenceList() {}

    /// Removes the last item in the list.
    /** The list must not be empty. If the list can be empty, call IsEmpty() before calling this function.
    */
    void RemoveLast() { refs.removeLast(); }

    /// Return size of the list.
    int Size() const { return refs.size(); }

    /// Inserts @ref at the end of the list.
    void Append(const AssetReference &ref) { refs.append(QVariant::fromValue(ref)); }

    /// Returns true if the list contains no items, false otherwise.
    bool IsEmpty() const { return refs.isEmpty(); }

    /// Subscript operator. If index @c i is invalid and empty AssetReference is returned.
    AssetReference operator[] (int i)
    {
        assert(i >= 0 && i < refs.size());
        if (i < 0 || i >= refs.size())
            return AssetReference();
        else
            return refs[i].value<AssetReference>();
    }

    /// This is an overloaded function.
    AssetReference operator[] (int i) const
    {
        assert(i >= 0 && i < refs.size());
        if (i < 0 || i >= refs.size())
            return AssetReference();
        else
            return refs[i].value<AssetReference>();
    }

    /// Returns true if @c rhs is equal to this list, otherwise false.
    bool operator ==(const AssetReferenceList &rhs) const 
    {
        if (this->refs.size() != rhs.refs.size())
            return false;
        for(uint i = 0; i < (uint)this->refs.size(); ++i)
            if (this->refs[i].value<AssetReference>().ref != rhs.refs[i].value<AssetReference>().ref)
                return false;
        return true;
    }

    /// Returns true if @c rhs is not equal to this list, otherwise false.
    bool operator !=(const AssetReferenceList &rhs) const { return !(*this == rhs); }

    /// List of asset references.
    QVariantList refs;
};

Q_DECLARE_METATYPE(AssetReference)
Q_DECLARE_METATYPE(AssetReferenceList)

#endif
