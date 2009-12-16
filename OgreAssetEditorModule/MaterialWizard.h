// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   MaterialWizard.h
 *  @brief  Utitility tool for choosing right material script for your purpose from
 *          the Naali material script template library.
 */

#ifndef incl_OgreAssetEditorModule_MaterialWizard_h
#define incl_OgreAssetEditorModule_MaterialWizard_h

#include <QObject>

namespace Foundation
{
    class Framework;
}

namespace UiServices
{
    class UiProxyWidget;
}

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

namespace OgreAssetEditor
{
    /// Enumeration of the Material Wizard options and possible combinations.
    enum MaterialWizardOptions
    {
        // Diffuse options
        MWO_DiffuseSolid                    = 1 << 1,
        MWO_DiffuseMap                      = 1 << 2,
        ///\todo MWO_VertexColor

        // Specular options
        MWO_SpecularSolid                   = 1 << 3,
        MWO_SpecularMap                     = 1 << 4,

        // Alpha options 
        MWO_AlphaSolid                      = 1 << 5,
        MWO_Opacity                         = 1 << 6,
        MWO_DiffuseAlpha                    = 1 << 8,
        MWO_AlphaMasking                    = 1 << 9,
        ///\todo MWO_TwoPassAlpha

        // Misc. options
        MWO_ReceivesShadows                 = 1 << 10,
        MWO_NormalMap                       = 1 << 11,
        MWO_LuminanceMap                    = 1 << 12,
        MWO_LightMap                        = 1 << 13,
        MWO_ReflectionMap                   = 1 << 14,
        MWO_Animation                       = 1 << 15,

        // Combinations
        Material_None                           = 0,
        Material_Diff                           = Material_None | MWO_DiffuseMap,
        Material_Diffa                          = Material_Diff | MWO_DiffuseAlpha,
        Material_DiffAlphamask                  = Material_Diff | MWO_AlphaMasking,
        Material_DiffAnim                       = Material_Diff | MWO_Animation,
        Material_DiffNormal                     = Material_Diff | MWO_NormalMap,
        Material_DiffNormalLightmap             = Material_DiffNormal | MWO_LightMap,
        Material_DiffNormalShadow               = Material_DiffNormal | MWO_ReceivesShadows,
        Material_DiffNormalShadowLightmap       = Material_DiffNormalShadow | MWO_LightMap,
        Material_DiffReflAlpha                  = Material_Diff | MWO_ReflectionMap | MWO_AlphaSolid,
        Material_DiffShadow                     = Material_Diff | MWO_ReceivesShadows,
        Material_DiffShadowLightmap             = Material_DiffShadow | MWO_LightMap,
        Material_DiffSpecflatNormalShadowLum    = Material_Diff | MWO_SpecularSolid | MWO_NormalMap | MWO_ReceivesShadows | MWO_LuminanceMap,
        Material_DiffSpecflatShadow             = Material_Diff | MWO_SpecularSolid | MWO_ReceivesShadows,
        Material_DiffSpecmap                    = Material_Diff | MWO_SpecularMap,
        Material_DiffSpecmapNormal              = Material_DiffSpecmap | MWO_NormalMap,
        Material_DiffSpecmapNormalOpa           = Material_DiffSpecmapNormal | MWO_Opacity,
        Material_DiffSpecmapNormalShadow        = Material_DiffSpecmapNormal | MWO_ReceivesShadows,
        Material_DiffSpecmapNormalShadowLum     = Material_DiffSpecmap | MWO_ReceivesShadows | MWO_LuminanceMap,
        Material_DiffSpecmapNormalShadowLumOpa  = Material_DiffSpecmap | MWO_NormalMap | MWO_ReceivesShadows | MWO_LuminanceMap | MWO_Opacity,
        Material_SpecmapRefl                    = Material_DiffSpecmap | MWO_ReflectionMap,
        Material_DiffSpecmapShadow              = Material_DiffSpecmap | MWO_ReceivesShadows,
        Material_DiffSpecmapShadowLum           = Material_DiffSpecmapShadow | MWO_LuminanceMap,
        Material_DiffSpecmapShadowLumOpa        = Material_DiffSpecmapShadowLum | MWO_Opacity,
        Material_DiffSpecmapShadowOpa           = Material_DiffSpecmapShadow | MWO_Opacity,
        Material_DiffSpecmapShadowRefl          = Material_DiffSpecmapShadow | MWO_ReflectionMap,
        ///\todo SSDiffTwoPassAlpha
        //Material_DiffTwoPassAlpha               = 0 | MWO_DiffuseMap | 
    };

    class MaterialWizard : public QObject
    {
        Q_OBJECT

    public:
        /// Consturctor.
        /// @param framework Framework pointer.
        MaterialWizard(Foundation::Framework *framework);

        /// Destructor.
        ~MaterialWizard();

    private slots:
        /// Chooses the right material script file and uploads it.
        void Create();

        /// Closes the material wizard.
        void Cancel();

        /// Activates and deactivates widgets according to the current selection.
        void RefreshWidgets();

    private:
        /// Initializes the UI.
        void InitWindow();

        /// Framework pointer.
        Foundation::Framework *framework_;

        /// Proxy widget for the UI.
        UiServices::UiProxyWidget *proxyWidget_;

        /// The window main widget.
        QWidget *mainWidget_;
    };
}

#endif
