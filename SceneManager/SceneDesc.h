/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneDesc.h
 *  @brief  Utility structures for describing scene and its contents.
 */

#ifndef incl_SceneManager_SceneDesc_h
#define incl_SceneManager_SceneDesc_h

/// Description of attribute.
struct AttributeDesc
{
    QString typeName; ///< Type name.
    QString name; ///< Name.
    QString value; ///< Value.

    /// Less than operator. Compares all values.
    bool operator <(const AttributeDesc &rhs) const
    {
        return typeName < rhs.typeName || name < rhs.name || value < rhs.value;
    }

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
    QList<EntityDesc> entities; ///< List of entities the entity has.

    /// Returns true if the scene description has no entities, false otherwise.
    bool IsEmpty() const { return entities.isEmpty(); }

    /// Equality operator. Returns true if all values match, false otherwise.
    bool operator ==(const SceneDesc &rhs) const
    {
        return name == rhs.name && viewEnabled == rhs.viewEnabled && entities == rhs.entities;
    }
};

#endif
