// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   OgreMaterialProperties.h
 *  @brief  Dynamically created QProperties for OGRE material scripts.
 */

#ifndef incl_OgreAssetEditorModule_OgreMaterialProperties_h
#define incl_OgreAssetEditorModule_OgreMaterialProperties_h

#include <QObject>
#include <QMap>
#include <QString>
#include <QVariant>

namespace Ogre
{
    class MaterialPtr;
}

namespace OgreRenderer
{
    class OgreMaterialResource;
}

namespace OgreAssetEditor
{
    class OgreMaterialProperties : public QObject
    {
        Q_OBJECT
    public:
        /// Constructor.
        /// @param material OgreMaterialResource pointer.
        explicit OgreMaterialProperties(const QString &name, OgreRenderer::OgreMaterialResource *material);

        /// Destructor.
        ~OgreMaterialProperties();

        typedef QMap<QString, QVariant> PropertyMap;
        typedef QMapIterator<QString, QVariant> PropertyMapIter;

        /// @return Property map (QString name, QVariant value).
        PropertyMap GetPropertyMap();

        /// @return Does this material have valid properties.
        bool HasProperties();

        /// Returns value for requested property.
        /// param name Property name.
//        template <class T> GetProperty(const QString &name);
//        QVariant GetProperty(const QString &name);

        /// @return OgreMaterialPtr
        Ogre::MaterialPtr ToOgreMaterial();

        /// @return Material script as a string.
        QString ToString();

    private:
        /// Creates the QProperties dynamically for this material.
        /// @True if creation was succesful, false otherwise.
        bool CreateProperties();

        /// Material resource pointer.
        OgreRenderer::OgreMaterialResource *material_;
    };
}

#endif
