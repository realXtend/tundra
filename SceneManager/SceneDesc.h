/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneDesc.h
 *  @brief  Utility structures for describing scene and its contents.
 */

#ifndef incl_SceneManager_SceneDesc_h
#define incl_SceneManager_SceneDesc_h

#include <QMap>

/// Description of asset.
struct AssetDesc
{
    QString source; ///< Specifies the source filename for the location of this asset.
    QByteArray data; ///< Specifies in-memory content for the asset data.

    /// If true, the data for this asset is loaded in memory, and specified by the member field 'data'. Otherwise,
    /// the data is loaded from disk, specified by the filename 'source'.
    bool dataInMemory;

    QString subname; ///< If the source filename is a container for multiple files, subname represents name withing the file.
    QString typeName; ///< Type name of the asset.
    QString destinationName; ///< Name for the asset in the destination asset storage.

#define LEX_CMP(a, b) if ((a) < (b)) return true; else if ((a) > (b)) return false;

    /// Less than operator. Compares source and subname only.
    bool operator <(const AssetDesc &rhs) const
    {
        LEX_CMP(source, rhs.source)
        LEX_CMP(subname, rhs.subname)
        return false;
    }

#undef LEX_CMP

    /// Equality operator. Returns true if filenames match, false otherwise.
    bool operator ==(const AssetDesc &rhs) const { return source == rhs.source; }
};

/// Description of attribute.
struct AttributeDesc
{
    QString typeName; ///< Type name.
    QString name; ///< Name.
    QString value; ///< Value.

#define LEX_CMP(a, b) if ((a) < (b)) return true; else if ((a) > (b)) return false;

    /// Less than operator. Compares all values.
    bool operator <(const AttributeDesc &rhs) const
    {
        LEX_CMP(typeName, rhs.typeName);
        LEX_CMP(name, rhs.name);
        LEX_CMP(value, rhs.value);
        return false;
    }

#undef LEX_CMP

    /// Equality operator. Returns true if all values match, false otherwise.
    bool operator ==(const AttributeDesc &rhs) const
    {
        return typeName == rhs.typeName && name == rhs.name && value  == rhs.value;
    }
};

/// Description of scene.
struct ComponentDesc
{
    QString typeName; ///< Type name.
    QString name; ///< Name (if applicable).
    QString sync; ///< Synchronize component.
    QList<AttributeDesc> attributes; ///< List of attributes the component has.

    /// Equality operator. Returns true if all values match, false otherwise.
    bool operator ==(const ComponentDesc &rhs) const
    {
        return typeName == rhs.typeName && name == rhs.name && attributes == rhs.attributes;
    }
};

/// Description of entity.
struct EntityDesc
{
    QString id; ///< ID (if applicable).
    QString name; ///< Name.
    bool local; ///< Is entity local.
    bool temporary; ///< Is entity temporary.
    QList<ComponentDesc> components; ///< List of components the entity has.

    /// Equality operator. Returns true if ID and name match, false otherwise.
    bool operator ==(const EntityDesc &rhs) const
    {
        return id == rhs.id && name == rhs.name /*&& local == rhs.local && temporary == rhs.temporary && components == rhs.components*/;
    }
};

/// Description of scene.
struct SceneDesc
{
    /// Origin file type
    enum Type
    {
        Naali, ///< Naali XML or binary scene
        OgreScene, ///< OGRE .scene
        OgreMesh, ///< OGRE .mesh
        OpenAsset ///< OpenAsset
    };

    QString filename; ///< Name of the file from which the description was created.
    Type type; ///< Type
    QString name; ///< Name.
    bool viewEnabled; ///< Is scene view enabled (ie. rendering-related components actually create stuff)
    QList<EntityDesc> entities; ///< List of entities the scene has.
    QMap<QString, AssetDesc> assets; ///< Map of of assets the scene refers to, AssetDesc::source used as key.

    /// Returns true if the scene description has no entities, false otherwise.
    bool IsEmpty() const { return entities.isEmpty(); }

    /// Equality operator. Returns true if all values match, false otherwise.
    bool operator ==(const SceneDesc &rhs) const
    {
        return name == rhs.name && viewEnabled == rhs.viewEnabled && entities == rhs.entities;
    }
};

#endif
