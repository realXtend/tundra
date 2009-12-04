// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   OgreMaterialProperties.h
 *  @brief  Dynamically created QProperties for OGRE material scripts.
 */

#ifndef incl_InventoryModule_OgreMaterialProperties_h
#define incl_InventoryModule_OgreMaterialProperties_h

#include <QObject>
//#include <QMap>
//#include <QString>
//#include <QVariant>

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
        explicit OgreMaterialProperties(OgreRenderer::OgreMaterialResource *material);

        /// Destructor.
        ~OgreMaterialProperties();

        ///@ return OgreMaterialPtr
        Ogre::MaterialPtr ToOgreMaterial();

    private:
        /// Creates the QProperties for this material.
        /// @True If succesful, false otherwise.
        bool CreateProperties(OgreRenderer::OgreMaterialResource *material);

        /// Material resource pointer.
        OgreRenderer::OgreMaterialResource *material_;

//        QMap<QString, QVariant> properties_;
    };
}

#endif
