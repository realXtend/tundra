// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"

#include <QString>
#include <QMetaType>
#include <QVariantList>

#include <cassert>

/// Represents a reference to an asset.
/** This structure can be used as a parameter type to an EC attribute. */
struct TUNDRACORE_API AssetReference
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

    /// Perform assignment
    /** @note This will not modify the type if already set. Set the type explicitly if required. */
    AssetReference& operator = (const AssetReference &rhs) { ref = rhs.ref; if (type.isEmpty()) type = rhs.type; return *this; }
    
    bool operator ==(const AssetReference &rhs) const { return this->ref == rhs.ref; }

    bool operator !=(const AssetReference &rhs) const { return !(*this == rhs); }

    bool operator <(const AssetReference &rhs) const { return ref < rhs.ref; }

    /// Specifies the URL of the asset that is being pointed to.
    QString ref;

    /// Specifies the type of the asset to load from that URL. If "", the type is interpreted directly from the ref string.
    /** Not all asset types can support this kind of interpretation. For example, avatar assets are of type .xml, which can
        only be distinguished from generic xml files by explicitly specifying the type here.

        @sa AssetAPI::GetResourceTypeFromAssetRef() */
    QString type;
};

/// Represents list of asset references.
/** This structure can be used as a parameter type to an EC attribute. */
struct TUNDRACORE_API AssetReferenceList
{
    /// Default constructor.
    AssetReferenceList() {}

    /// Constructor taking preferred asset type.
    /** @param type Preferred asset type for the list. */
    AssetReferenceList(const QString &preferredType) { type = preferredType; }

    /// Removes the last item in the list.
    /** The list must not be empty. If the list can be empty, call IsEmpty() before calling this function. */
    void RemoveLast() { refs.removeLast(); }

    /// Perform assignment.
    /** @note This will not modify the type if already set. Set the type explicitly if required. */
    AssetReferenceList& operator = (const AssetReferenceList &rhs) { refs = rhs.refs; if (type.isEmpty()) type = rhs.type; return *this; }

    /// Removes empty items
    void RemoveEmpty()
    {
        unsigned size = refs.size();
        for(unsigned i = size - 1; i < size; --i)
        {
            if (refs[i].value<AssetReference>().ref.trimmed().isEmpty())
                refs.erase(refs.begin() + i);
        }
    }

    /// Return size of the list.
    int Size() const { return refs.size(); }

    /// Inserts @ref at the end of the list.
    void Append(const AssetReference &ref) { refs.append(QVariant::fromValue(ref)); }

    /// Returns true if the list contains no items, false otherwise.
    bool IsEmpty() const { return refs.isEmpty(); }

    /// Sets new value in the list.
    /** @param i Index.
        @ref New asset reference value. */
    void Set(int i, AssetReference ref)
    {
        assert(i >= 0 && i < refs.size());
        if (i >= 0 && i < refs.size())
            refs[i] = QVariant::fromValue(ref);
    }

    /// Subscript operator. If index @c i is invalid and empty AssetReference is returned.
    /** @note Doesn't return reference for script-compatibility/safety.*/
    const AssetReference operator[] (int i)
    {
        assert(i >= 0 && i < refs.size());
        if (i < 0 || i >= refs.size())
            return AssetReference();
        else
            return refs[i].value<AssetReference>();
    }

    /// @overload
    /** @note Doesn't return reference for script-compatibility/safety. */
    const AssetReference operator[] (int i) const
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

    /// Preferred type for asset refs in the list
    /** @sa AssetReference::type;
        @sa AssetAPI::GetResourceTypeFromAssetRef() */
    QString type;
};

Q_DECLARE_METATYPE(AssetReference)
Q_DECLARE_METATYPE(AssetReferenceList)
