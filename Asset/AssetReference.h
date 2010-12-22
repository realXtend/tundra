// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetReference_h
#define incl_Asset_AssetReference_h

#include <QString>
#include <QMetaType>
#include <QVariantList>

/// Represents a reference to an asset. This structure can be used as a parameter type to an EC attribute.
struct AssetReference
{
public:
    /// Default constructor
    AssetReference() {}

    /// Constructs an asset reference pointing to the given asset.
    /// @param reference The URL of the asset to point to, e.g. "local://myasset.mesh", or "http://www.website.com/texture.png".
    AssetReference(const QString &reference) : ref(reference) {}

    bool operator ==(const AssetReference &rhs) const { return this->ref == rhs.ref; }

    bool operator !=(const AssetReference &rhs) const { return !(*this == rhs); }

    bool operator <(const AssetReference &rhs) const { return ref < rhs.ref; }

    /// Specifies the URL of the asset that is being pointed to.
    QString ref;
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
        assert(i > 0 || i < refs.size());
        if (i < 0 || i > refs.size())
            return AssetReference();
        else
            return refs[i].value<AssetReference>();
    }

    /// This is an overloaded function.
    AssetReference operator[] (int i) const
    {
        assert(i > 0 || i < refs.size());
        if (i < 0 || i > refs.size())
            return AssetReference();
        else
            return refs[i].value<AssetReference>();
    }

    /// Returns true if @c rhs is equal to this list, otherwise false.
    bool operator ==(const AssetReferenceList &rhs) const 
    {
        if (this->refs.size() != rhs.refs.size())
            return false;
        for(unsigned int i = 0; i < this->refs.size(); ++i)
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
