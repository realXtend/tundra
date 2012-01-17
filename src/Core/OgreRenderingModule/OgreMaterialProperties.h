/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   OgreMaterialProperties.h
 *  @brief  Dynamically created QProperties for OGRE material scripts.
 */

#pragma once

#include "OgreModuleApi.h"

#include <boost/shared_ptr.hpp>

#include <OgreTexture.h>
#include <OgreGpuProgram.h>

#include <QObject>
#include <QMap>
#include <QString>

typedef QMap<QString, QVariant> PropertyMap;
typedef QMap<QString, QVariant> TypeValuePair;
typedef QMapIterator<QString, QVariant> PropertyMapIter;

/// Gathers name-value map of shader parameters of Ogre material.
/** Vertex shader, fragment/pixel shader and texture unit names are appended with " VP", " FP" and " TU" respectively.
    @param material Material to be inspected.
    @return Property map QMap[QString(name), QVariant[QMap[QString(type), QVariant(value)]]]. */
PropertyMap OGRE_MODULE_API GatherShaderParameters(const Ogre::MaterialPtr &material, bool includeTextureUnits = false);

/// Dynamically created QProperties for OGRE material scripts.
///\todo Cleanup/merge with OgreMaterialUtils.h
class OGRE_MODULE_API OgreMaterialProperties : public QObject
{
    Q_OBJECT

public:
    /// Constructor.
    /// @param asset Asset pointer to the material binary data.
    explicit OgreMaterialProperties(const QString &name, void *asset); ///\todo Regression. Reimplement using the new Asset API. -jj.
//    explicit OgreMaterialProperties(const QString &name, AssetInterfacePtr asset);

    ~OgreMaterialProperties();

    /// Convenience function returning map of property names, their type and values.
    /** Provides easier access to the properties than iterating dynamic properties by hand.
        @return Property map QMap[QString(name), QVariant[QMap[QString(type), QVariant(value)]]]. */
    PropertyMap GetPropertyMap() const;

    /// @return Does this material have valid properties.
    bool HasProperties() const;

    /// @return Material script as an OgreMaterialPtr.
    Ogre::MaterialPtr ToOgreMaterial();

    /// @return Material script as a string.
    QString ToString();

    /// Utility function for converting Ogre::GpuConstantType enum to type string.
    static QString GpuConstantTypeToString(Ogre::GpuConstantType type);

    /// Utility function for converting Ogre::GpuConstantType enum to type string.
    static QString TextureTypeToString(Ogre::TextureType type);

private:
    /// Creates the QProperties dynamically for this material.
    /** @return True if creation was succesful, false otherwise. */
    bool CreateProperties();

    /// Material resource pointer.
    ///\todo Regression. Reimplement using the new Asset API. -jj.
//    OgreRenderer::OgreMaterialResource *material_;
};
