// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Environment_EnvironmentEditor_h
#define incl_Environment_EnvironmentEditor_h

#include <Foundation.h>
#include "UICanvas.h"

#include <QObject>
#include <QPair>

class QImage;

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
    class Environment;

    typedef QPair<float, float> MinMaxValue;

    class EnvironmentEditor: public QObject
    {
        Q_OBJECT

    public:
        // All modify land actions that we can send to server.
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

        //! Handle resource ready event.
        void HandleResourceReady(Resource::Events::ResourceReady *res);

        void SetTerrainTextureID();

        static const int cHeightmapImageWidth  = 256;
        static const int cHeightmapImageHeight = 256;
        static const int cNumberOfTerrainTextures = 4;

    public slots:
        //! Get a new terrain texture from rexlogic.
        void UpdateTerrain();

        void UpdateWaterGeometry(int state);
        void UpdateWaterHeight();

        void UpdateTerrainTextureRanges();

        //! Called when qMouseEvent event is generated.
        void HandleMouseEvent(QMouseEvent *ev);

        //! Called when brush size option has changed.
        void BrushSizeChanged();

        //! Called when terrain action option has changed.
        void PaintActionChanged();

        //! Called when tab window state has changed.
        void TabWidgetChanged(int index);

        //! Called when user has pressed enter or return on editor's LineEdit widget.
        void LineEditReturnPressed();

        //! Called when apply button have been pressed
        void ApplyButtonPressed();

        //! Called when user change some of the height values.
        void HeightValueChanged(double height);

        //! Adjust Water check button.
        void ToggleWaterCheckButton();


        void UpdateGroundFog(float fogStart, float fogEnd, const QVector<float>& color);
        void UpdateWaterFog(float fogStart, float fogEnd, const QVector<float>& color);
        void SetGroundFog();
        void SetWaterFog();
        void SetGroundFogDistance();
        void SetWaterFogDistance();
        void ToggleFogOverride();

        void UpdateAmbient();

    private:
        Q_DISABLE_COPY(EnvironmentEditor);

        //! Convert Degoded texture into QImage format. Note! This method is planned to be removed when QImage can directly created using raw data pointer.
        //! @Param tex Reference to texture resource.
        //! @Return converted QImage.
        QImage ConvertToQImage(Foundation::TextureInterface &tex);

        //! Create a window for terrain editor.
        void InitEditorWindow();

        //! Create a new heightmap image that will show heightmap values in grayscale.format
        void CreateHeightmapImage();

        //! Ask texture decoder for a texture resource.
        //! @Param index for terrain_texture_id_list_ that holds the uuid that we want to use to request the resource that we need. Range should be [0 - 3].
        //! @Return request tag for the texture.
        request_tag_t RequestTerrainTexture(uint index);

        //! Asset_tags for terrain texture requests.
        std::vector<request_tag_t> terrain_texture_requests_;

        //! Terrain texture id list.
        std::vector<std::string> terrain_texture_id_list_;

        //! Return heightmap smallest and largest value (first = min and second = max).
        MinMaxValue GetMinMaxHeightmapValue(EC_Terrain &terrain) const;

        //! Pointer for environment module.
        EnvironmentModule *environment_module_;

        //! Main widget for editor
        QWidget *editor_widget_;

        //! Terrain geometry information.
        boost::shared_ptr<Terrain> terrain_;

        //! Water information (geometry etc.)
        boost::shared_ptr<Water> water_;

        //! Environment information.
        boost::shared_ptr<Environment> environment_;

        //! Brush size (small, medium and large).
        BrushSize brush_size_;

        //! Terrain actions (Flatten, Raise, Lower, Smooth, Roughen and Revert).
        ModifyLandAction action_;

        /// Proxy Widget for ui
        UiServices::UiProxyWidget *EnvironmentEditorProxyWidget_;

        //! Mouse press flags
        //u8 mouse_press_flag_;

        /// 
        //Real start_height_;
    };
}

#endif