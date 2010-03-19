// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   OgreMaterialProperties.h
 *  @brief  Dynamically created QProperties for OGRE material scripts.
 */

#ifndef incl_OgreAssetEditorModule_OgreMaterialProperties_h
#define incl_OgreAssetEditorModule_OgreMaterialProperties_h

#include <boost/shared_ptr.hpp>

#include <OgreTexture.h>
#include <OgreGpuProgram.h>

#include <QObject>
#include <QMap>
#include <QString>

namespace Foundation
{
    class Framework;
    class AssetInterface;
    typedef boost::shared_ptr<AssetInterface> AssetPtr;
}

namespace OgreRenderer
{
    class OgreMaterialResource;
}

namespace Naali
{
    typedef QMap<QString, QVariant> PropertyMap;
    typedef QMap<QString, QVariant> TypeValuePair;
    typedef QMapIterator<QString, QVariant> PropertyMapIter;

    class OgreMaterialProperties : public QObject
    {
        Q_OBJECT

    public:
        /// Constructor.
        /// @param asset Asset pointer to the material binary data.
        explicit OgreMaterialProperties(const QString &name, Foundation::AssetPtr asset);

        /// Destructor.
        ~OgreMaterialProperties();

        /// Convenience function returning map of property names, their type and values.
        /// Provides easier access to the properties than iterating dynamic properties by hand.
        /// @return Property map QMap[QString(name), QVariant[QMap[QString(type), QVariant(value)]]].
        PropertyMap GetPropertyMap();

        /// @return Does this material have valid properties.
        bool HasProperties();

        /// @return Material script as an OgreMaterialPtr.
        Ogre::MaterialPtr ToOgreMaterial();

        /// @return Material script as a string.
        QString ToString();

        /// Utility function for converting Ogre::GpuConstantType enum to type string.
        /// @param type Ogre::GpuConstantType enum.
        /// @return Type as string.
        static QString GpuConstantTypeToString(const Ogre::GpuConstantType &type);

        /// Utility function for converting Ogre::GpuConstantType enum to type string.
        /// @param type Ogre::GpuConstantType enum.
        /// @return Type as string.
        static QString TextureTypeToString(const Ogre::TextureType &type);

    private:
        /// Creates the QProperties dynamically for this material.
        /// @True if creation was succesful, false otherwise.
        bool CreateProperties();

        /// Material resource pointer.
        OgreRenderer::OgreMaterialResource *material_;
    };
}

#endif
