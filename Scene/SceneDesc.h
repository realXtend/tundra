/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneDesc.h
 *  @brief  Light-weigth structures for describing scene and its contents.
 */

#pragma once

#include "SceneFwd.h"

#include <QMap>
#include <QPair>

/// Description of a scene (Scene).
/** A source-agnostic scene graph description of a Tundra scene.
    A Tunda scene consist of entities, components, attributes and assets references.
    @sa EntityDesc, ComponentDesc, AttributeDesc and AssetDesc */
struct SceneDesc
{
    typedef QPair<QString, QString> AssetMapKey; ///< source-subname pair used to idenfity assets.
    typedef QMap<AssetMapKey, AssetDesc> AssetMap; ///< Map of assets.

    QString filename; ///< Name of the file from which the description was created.
    QString name; ///< Name.
    bool viewEnabled; ///< Is scene view enabled (ie. rendering-related components actually create stuff)
    QList<EntityDesc> entities; ///< List of entities the scene has.
    AssetMap assets; ///< Map of unique assets.

    /// Returns true if the scene description has no entities, false otherwise.
    bool IsEmpty() const { return entities.isEmpty(); }

    /// Equality operator. Returns true if all values match, false otherwise.
    bool operator ==(const SceneDesc &rhs) const
    {
        return name == rhs.name && viewEnabled == rhs.viewEnabled && entities == rhs.entities;
    }
};

/// Description of an entity (Entity).
struct EntityDesc
{
    QString id; ///< ID (if applicable).
    QString name; ///< Name.
    bool local; ///< Is entity local.
    bool temporary; ///< Is entity temporary.
    QList<ComponentDesc> components; ///< List of components the entity has.

    /// Default constructor.
    EntityDesc()
    {
        local = false;
        temporary = false;
    }
    
    /// Constructor with full input param list.
    EntityDesc(QString id_, QString name_ = QString(), bool local_ = false, bool temporary_ = false)
    {
        id = id_;
        name = name_;
        local = local_;
        temporary = temporary_;
    }

    /// Equality operator. Returns true if ID and name match, false otherwise.
    bool operator ==(const EntityDesc &rhs) const
    {
        return id == rhs.id && name == rhs.name /*&& local == rhs.local && temporary == rhs.temporary && components == rhs.components*/;
    }
};

/// Description of an entity-component (IComponent).
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

/// Description of an attribute (IAttribute).
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

/// Description of an asset (IAsset) or an asset reference (AssetReference).
struct AssetDesc
{
    QString source; ///< Specifies the source filename for the location of this asset.
    QByteArray data; ///< Specifies in-memory content for the asset data.

    /// If true, the data for this asset is loaded in memory, and specified by the member field 'data'. Otherwise,
    /// the data is loaded from disk, specified by the filename 'source'.
    bool dataInMemory;

    QString subname; ///< If the source filename is a container for multiple files, subname represents name within the file.
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
