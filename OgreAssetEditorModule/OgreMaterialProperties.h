// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   OgreMaterialProperties.h
 *  @brief 
 */

#ifndef incl_InventoryModule_OgreMaterialProperties_h
#define incl_InventoryModule_OgreMaterialProperties_h

#include <QObject>
//#include <QMap>
//#include <QString>
//#include <QVariant>

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
        explicit OgreMaterialProperties(OgreRenderer::OgreMaterialResource *material);
        ~OgreMaterialProperties();
    private:
        bool CreateProperties(OgreRenderer::OgreMaterialResource *material);
//        QMap<QString, QVariant> properties_;
    };
}

#endif
