// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Environment_EnvironmentEditor_h
#define incl_Environment_EnvironmentEditor_h

#include <Foundation.h>
#include "EC_OgreSky.h"

#include <QObject>
#include <QPair>
#include <QTimer>

class QImage;
class QColor;
class QMouseEvent;
class QColorDialog;

namespace Resource
{
    namespace Events
    {
        class ResourceReady;
    }
}

namespace Foundation
{
    class TextureInterface;
}

namespace UiServices
{
    class UiProxyWidget;
}

namespace Environment
{
    class EnvironmentModule;
    class Terrain;
    class EC_Terrain;
    class Water;
    class Sky;
    class Environment;

    typedef QPair<float, float> MinMaxValue;

    class EnvironmentEditor: public QObject
    {
        Q_OBJECT

    public:
        // All modify land actions that we can send into the server.
        // Note! Dont change the order of this list or server wont do right modify land actions.
        enum ModifyLandAction
        {
            Flatten = 0,
            Raise   = 1,
            Lower   = 2,
            Smooth  = 3,
            Roughen = 4,
            Revert  = 5
        };

        // All brush sizes that we can use to modify our land.
        // Note! Dont change the order of this list.
        enum BrushSize
        {
            Small   = 0,
            Medium  = 1,
            Large   = 2
        };

        /*enum MousePressEvents
        {
            no_button       = 0,
            left_button     = 1 << 0,
            right_button    = 1 << 1,
            middle_button   = 1 << 2,
            button_mask     = 0 + left_button + right_button + middle_button
        };*/

        //! Constuctor
        EnvironmentEditor(EnvironmentModule *environment_module);

        //! Destructor
        virtual ~EnvironmentEditor();

        //! Handle resource ready event that will return the texture that has been requested.
        void HandleResourceReady(Resource::Events::ResourceReady *res);

        static const int cHeightmapImageWidth  = 256;
        static const int cHeightmapImageHeight = 256;
        static const int cNumberOfTerrainTextures = 4;

    public slots:
        //! Create new terrain heightmap texture using terrain height information.
        void UpdateTerrain();

        //! Get new height ranges for a each terrain texture.
        void UpdateTerrainTextureRanges();

        //! Get new terrain texture asset ids from the terrain object and send a asset request to the texture decoder.
        void UpdateTerrainTextures();

        void UpdateWaterGeometry(int state);
        void UpdateWaterHeight();

        //! Called when sky type has been changed.
        void SkyTypeChanged(int index);

        //! Enable/Disable sky.
        void UpdateSkyState(int state);

        //! Set combobox pointing to right sky type when sky type has changed by server side.
        void UpdateSkyType();

        //! Reads new texture names from line-edit fields and change those textures in EC_OgreSky if they have changed.
        void ChangeSkyTextures();

        //! Get new sky texture names from sky object and insert them into texture lineedit fields.
        void UpdateSkyTextureNames();

        //! Reads sky properties values from QSpinboxes and send them to EC_Sky object.
        void ChangeSkyProperties();

        //! Change state of sky enable check box.
        void ToggleSkyCheckButton(bool enabled);

        //! Called when QMouseEvent event is generated inside the terrain map image label.
        void HandleMouseEvent(QMouseEvent *ev);

        //! When called modify land message is sended into the server. This method is connected with mouse click event and is active,
        //! when MouseLeftPress event is active (Take a look at HandleMouseEvent-method).
        //! @todo add parameter that will tell how much time has past when this method was last time called (right now, always using 250ms time.).
        void TerrainEditTimerTick();

        //! Called when brush size option has changed. Supported sizes are Small, Medium and Large.
        void BrushSizeChanged();

        //! Called when terrain action option has changed. Supported actions are Flatten, Raise, Lower, Smooth, Roughen and Revert.
        void PaintActionChanged();

        //! Called when editor window's active tab has been changed.
        void TabWidgetChanged(int index);

        //! Called when user has pressed enter or return on editor's LineEdit-widget on terrain texture tab window.
        void LineEditReturnPressed();

        //! Called when apply button have been pressed on terrain texture tab window.
        void ChangeTerrainTexture();

        //! Adjust Water check button.
        void ToggleWaterCheckButton();

        void UpdateGroundFog(float fogStart, float fogEnd, const QVector<float>& color);

        void UpdateWaterFog(float fogStart, float fogEnd, const QVector<float>& color);
        void SetGroundFog();
        void SetWaterFog();
        void SetGroundFogDistance();
        void SetWaterFogDistance();
        void ToggleFogOverride();

        void UpdateSunDirection(double value);
        void ShowColorPicker();
        void UpdateSunLightColor(const QColor& color);
        void UpdateAmbientLightColor(const QColor& color);

    private:
        Q_DISABLE_COPY(EnvironmentEditor);

        //! Convert Degoded texture into QImage format. Note! This method is planned to be removed when QImage can directly created using raw data pointer.
        //! @Param tex Reference to texture resource.
        //! @Return converted QImage.
        QImage ConvertToQImage(Foundation::TextureInterface &tex);

        //! Create a window for terrain editor.
        void InitEditorWindow();
        void InitTerrainTabWindow();
        void InitTerrainTextureTabWindow();
        void InitWaterTabWindow();
        void InitSkyTabWindow();
        void InitFogTabWindow();
        void InitAmbientTabWindow();

        //! Create a new heightmap image that will show heightmap values in grayscale.format.
        void CreateHeightmapImage();

        //! Clear old sky properties and create a new one for spesific sky type.
        //! @Param sky_type is used to tell what type of sky is in use, so right properties will be created.
        void CreateSkyProperties(OgreRenderer::SkyType sky_type);

        //! Clean all option widgets on the option scroll area.
        void CleanSkyProperties();

        //! Ask texture decoder for a texture resource.
        //! @Param index for terrain_texture_id_list_ that holds the uuid that we want to use to request the resource that we need. Range should be [0 - 3].
        //! @Return request tag for the texture.
        request_tag_t RequestTerrainTexture(uint index);

        QWidget* GetPage(const QString& name);
        QWidget* GetCurrentPage();

        //! Asset_tags for terrain texture requests.
        std::vector<request_tag_t> terrain_texture_requests_;

        //! Terrain texture asset id list.
        std::vector<std::string> terrain_texture_id_list_;

        //! Return heightmap smallest and largest value (first = min and second = max).
        MinMaxValue GetMinMaxHeightmapValue(EC_Terrain &terrain) const;

        //! Pointer for environment module.
        EnvironmentModule *environment_module_;

        //! Main widget for editor
        QWidget *editor_widget_;

        //! Brush size (small, medium and large).
        BrushSize brush_size_;

        //! Terrain actions (Flatten, Raise, Lower, Smooth, Roughen and Revert).
        ModifyLandAction action_;

        //! Proxy Widget for ui
        UiServices::UiProxyWidget *EnvironmentEditorProxyWidget_;

        QColorDialog* sun_color_picker_;
        QColorDialog* ambient_color_picker_;
        bool ambient_;

        //! sky type in use.
        OgreRenderer::SkyType sky_type_;

        //! Timer for terrain painting so that terrain is painted certain time.
        QTimer terrain_paint_timer_;
        //! Mouse press flags
        //u8 mouse_press_flag_;
        
        //! Only use with terrain paint to keep track where mouse is locating.
        int mouse_position_[2];

        bool edit_terrain_;

        /// 
        //Real start_height_;
    };
}

#endif
