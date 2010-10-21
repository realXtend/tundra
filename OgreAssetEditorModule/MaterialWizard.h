//$ HEADER_MOD_FILE $
/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   MaterialWizard.h
 *  @brief  Utitility tool for choosing right material script for your purpose from
 *          the Naali material script template library.
 */

#ifndef incl_OgreAssetEditorModule_MaterialWizard_h
#define incl_OgreAssetEditorModule_MaterialWizard_h

#include <QWidget>
#include <QString>

namespace Foundation
{
    class Framework;
}

namespace Inventory
{
    class InventoryUploadEventData;
}

class QVBoxLayout;

class MaterialWizard : public QWidget
{
    Q_OBJECT

public:
    /// Enumeration of the Material Wizard options and possible combinations.
    enum MaterialWizardOption
    {
        // Diffuse options
        MWO_DiffuseSolid                    = 1 << 1,
        MWO_DiffuseMap                      = 1 << 2,
        ///\todo MWO_VertexColor

        // Specular options
        MWO_SpecularNone                    = 1 << 3,
        MWO_SpecularSolid                   = 1 << 4,
        MWO_SpecularMap                     = 1 << 5,

        // Alpha options 
        MWO_AlphaNone                       = 1 << 6,
        MWO_AlphaSolid                      = 1 << 7,
        MWO_Opacity                         = 1 << 8,
        MWO_DiffuseAlpha                    = 1 << 9,
        MWO_AlphaMasking                    = 1 << 10,
        ///\todo MWO_TwoPassAlpha

        // Misc. options
        MWO_ReceivesShadows                 = 1 << 11,
        MWO_NormalMap                       = 1 << 12,
        MWO_LuminanceMap                    = 1 << 13,
        MWO_LightMap                        = 1 << 14,
        MWO_ReflectionMap                   = 1 << 15,
        MWO_Animation                       = 1 << 16,

        // Currently available combinations
        Material_None                           = 0,
        Material_Diff                           = Material_None | MWO_DiffuseMap | MWO_SpecularNone | MWO_AlphaNone,
        Material_Diffa                          = Material_None | MWO_DiffuseMap | MWO_SpecularNone | MWO_DiffuseAlpha,
        Material_DiffAlphamask                  = Material_Diff | MWO_AlphaMasking,
        Material_DiffAnim                       = Material_Diff | MWO_Animation,
        Material_DiffNormal                     = Material_Diff | MWO_NormalMap,
        Material_DiffNormalLightmap             = Material_DiffNormal | MWO_LightMap,
        Material_DiffNormalShadow               = Material_DiffNormal | MWO_ReceivesShadows,
        Material_DiffNormalShadowLightmap       = Material_DiffNormalShadow | MWO_LightMap,
        Material_DiffReflAlpha                  = Material_None | MWO_DiffuseMap | MWO_SpecularNone |MWO_ReflectionMap | MWO_AlphaSolid,
        Material_DiffShadow                     = Material_Diff | MWO_ReceivesShadows,
        Material_DiffShadowLightmap             = Material_DiffShadow | MWO_LightMap,
        Material_DiffSpecflatShadow             = Material_None | MWO_DiffuseMap | MWO_AlphaNone | MWO_SpecularSolid | MWO_ReceivesShadows,
        Material_DiffSpecflatNormalShadowLum    = Material_DiffSpecflatShadow | MWO_NormalMap | MWO_LuminanceMap,
        Material_DiffSpecmap                    = Material_None | MWO_DiffuseMap | MWO_AlphaNone | MWO_SpecularMap,
        Material_DiffSpecmapNormal              = Material_DiffSpecmap | MWO_NormalMap,
        Material_DiffSpecmapNormalOpa           = Material_None | MWO_DiffuseMap | MWO_SpecularMap | MWO_Opacity | MWO_NormalMap,
        Material_DiffSpecmapNormalShadow        = Material_DiffSpecmapNormal | MWO_ReceivesShadows,
        Material_DiffSpecmapNormalShadowLum     = Material_DiffSpecmapNormal | MWO_ReceivesShadows | MWO_LuminanceMap,
        Material_DiffSpecmapNormalShadowLumOpa  = Material_DiffSpecmapNormalOpa | MWO_ReceivesShadows | MWO_LuminanceMap,
        Material_DiffSpecmapRefl                = Material_DiffSpecmap | MWO_ReflectionMap,
        Material_DiffSpecmapShadow              = Material_DiffSpecmap | MWO_ReceivesShadows,
        Material_DiffSpecmapShadowLum           = Material_DiffSpecmapShadow | MWO_LuminanceMap,
        Material_DiffSpecmapShadowLumOpa        = Material_None | MWO_DiffuseMap | MWO_SpecularMap | MWO_Opacity | MWO_ReceivesShadows | MWO_LuminanceMap,
        Material_DiffSpecmapShadowOpa           = Material_None | MWO_DiffuseMap | MWO_SpecularMap | MWO_Opacity | MWO_ReceivesShadows,
        Material_DiffSpecmapShadowRefl          = Material_DiffSpecmapShadow | MWO_ReflectionMap,
        ///\todo Material_DiffTwoPassAlpha
    };
    Q_DECLARE_FLAGS(MaterialWizardOptions, MaterialWizardOption)
//$ BEGIN_MOD $   
    /// Consturctor.
    /// @param parent Parent widget.
	explicit MaterialWizard(QWidget *parent = 0, Foundation::Framework *framework = 0);
//$ END_MOD $  
    /// Destructor.
    ~MaterialWizard();

public slots:
    /// Closes the material wizard.
    void Close();

signals:
    /// Emitted when new material is succesfully chosen and ready for upload.
    /// @param event_data Inventory upload event data. If null v
    void NewMaterial(Inventory::InventoryUploadEventData *event_data);

private slots:
    /// Chooses the right material script file and uploads it.
    void Create();

    /// Activates and deactivates widgets according to the current selection.
    void RefreshWidgets();

    /// Unchecks every checkbox and radiobutton.
    void ClearSelections();

    /// Checks that name name isn't null.
    void ValidateScriptName(const QString &name);

private:
    /// @return Name of the material script template according to the current parameters,
    /// or null string if no matches for current parameters.
    QString GetCurrentMaterialFilename() const;

    /// Framework pointer.
    Foundation::Framework *framework_;

    /// Main widget loaded from .ui file.
    QWidget *mainWidget_;

    /// Bit mask of current material configuration options.
    MaterialWizardOptions currentOptions_;

    /// Name of the script.
    QString scriptName_;
};

#endif
