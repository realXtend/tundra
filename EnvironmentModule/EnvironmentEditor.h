// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Environment_EnvironmentEditor_h
#define incl_Environment_EnvironmentEditor_h

#include "EC_OgreSky.h"

#include "Color.h"

#include "InputServiceInterface.h"

#include <QPair>
#include <QTimer>
#include <QVector>
#include <QWidget>
#include <QCheckBox>

class QImage;
class QColor;
class QMouseEvent;
class QColorDialog;
class QSlider;

class MouseEvent;

namespace Ogre
{
    class ManualObject;
    class SceneNode;
}

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

namespace Environment
{
    class EnvironmentModule;
    class Terrain;
    class EC_Terrain;
    class Water;
    class Sky;
    class Environment;

    typedef QPair<float, float> MinMaxValue;

    //! Environment editor window. Owned by EnvironmentModule. The editor is responsible for terrain, water, sky, fog and light editing.
    //! Terrain: Paint terrain heightmap, update terrain textures and height ranges.
    //! Water: Enable/Disable water geometry and change it height value.
    //! Sky: Choose between three sky types that are Sky_box, Sky_dome and Sky_plane, update sky parameters like distance and texture and enable/disable sky geometry.
    //!      Sky editing is only enabled when the Caelum is turned off.
    //! Fog: Change water/ground fog color and distances (start and end distances).
    //! Light: Change sunlight direction and color and change ambient light color.
    //! \ingroup EnvironmentModuleClient.
    class EnvironmentEditor: public QWidget
    {
        Q_OBJECT

    public:
        //! All modify land actions that we can send into the server.
        //! Note! Dont change the order of this list or server wont do right modify land actions.
        enum ModifyLandAction
        {
            Flatten = 0,
            Raise   = 1,
            Lower   = 2,
            Smooth  = 3,
            Roughen = 4,
            Revert  = 5
        };

        //! All brush sizes that we can use to modify our land.
        //! Note! Dont change the order of this list.
        enum BrushSize
        {
            //!Brush size is 3x3 vertices.
            Small   = 0,
            //!Brush Size is 5x5 vertices.
            Medium  = 1,
            //!Brush Size is 9x9 vertices.
            Large   = 2
        };

        enum TerrainPaintMode
        {
            ACTIVE,
            INACTIVE
        };

        //! Constuctor
        EnvironmentEditor(EnvironmentModule *environment_module);

        //! Destructor
        virtual ~EnvironmentEditor();

        //! Handle resource ready event that will return the texture that has been requested.
        void HandleResourceReady(Resource::Events::ResourceReady *res);

        //! Heightmap image width
        static const int cHeightmapImageWidth  = 256;

        //! Heightmap image height
        static const int cHeightmapImageHeight = 256;

        //! How many textures terrain is using
        static const int cNumberOfTerrainTextures = 4;
 
        void InitTerrainTabWindow();
        void InitTerrainTextureTabWindow();
        void InitWaterTabWindow();
        void InitSkyTabWindow();
        void InitFogTabWindow();
        void InitAmbientTabWindow();
    
        bool Showed() { return editor_widget_->isVisible(); }

    public slots:
        //! Toggle between Paint2D and Paint3D mode.
        void ToggleTerrainPaintMode();

        //! What paint mode is in use (Paint2D or Paint3D).
        TerrainPaintMode GetTerrainPaintMode() const;

        //! Will handle MOUSEDRAG and MOUSEDRAG_STOPPED events. When MOUSEDRAG event is called first time
        //! terrain painting mode is set to active and terrain_paint_timer is set running. 
        //! terrain painting mode is active until WorldInputModule will send a MOUSEDRAG_STOPPED event.
        //! @param event_id Mouse event id is used to identifie what mouse event just occured.
        //! @param data Mouse movement information, used for raycast.
        //! @return Should return true if the event was handled and is not to be propagated further
        bool HandleMouseDragEvent(event_id_t event_id, IEventData* data);

        //! Create new terrain heightmap texture using terrain height information.
        void UpdateTerrain();

        //! Get new height ranges for a each terrain texture.
        void UpdateTerrainTextureRanges();

        //! Get new terrain texture asset ids from the terrain object and send a asset request to the texture decoder.
        void UpdateTerrainTextures();

        //! Update water geometry if water geometry is not already created, create new water geometry and get new water height from editor window.
        //! @param state Editor window's checkbox state.
        void UpdateWaterGeometry(int state);

        //! Update water height value.
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

        void HandleKeyInputEvent(KeyEvent *key);

        void HandleMouseInputEvent(MouseEvent *mouse);

        //! Called when QMouseEvent event is generated inside the TerrainLabel.
        //! ev mouse event pointer.
//        void HandleMouseEvent(QMouseEvent *ev);

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

     
        //! Update ground fog.
        //! @param fogStart fog start distance
        //! @param fogEnd fog end distance
        //! @param color fog color
        void UpdateGroundFog(float fogStart, float fogEnd, const QVector<float>& color);

        //! Update water fog.
        //! @param fogStart fog start distance
        //! @param fogEnd fog end distance
        //! @param color fog color
        void UpdateWaterFog(float fogStart, float fogEnd, const QVector<float>& color);

        //! Set new ground fog values to Environment object.
        void SetGroundFog();

        //! Set new water fog values to Environment object.
        void SetWaterFog();

        //! Set new ground fog distance value to Environment object.
        void SetGroundFogDistance();

        //! Set new water fog distance value to Environment object.
        void SetWaterFogDistance();

        //! Toggle if fog color can be changed by client side.
        void ToggleFogOverride();

        //! Updates sun direction when user change some of the cordinate values from the editor widnow.
        void UpdateSunDirection(double value);

        //! Bring a external color picker dialog on the screen.
        void ShowColorPicker();

        //! Return the original color to environment if color picker got cancel signal.
        void ColorPickerRejected();

        //! Update sun light color value.
        //! @color new color value.
        void UpdateSunLightColor(const QColor& color);

        //! Update ambient light color.
        //! @param new color value.
        void UpdateAmbientLightColor(const QColor& color);

        void TimeOfDayOverrideChanged(int state);
        void TimeValueChanged(int new_value);

        void InitializeTabs();

    protected:
        /// QWidget override.
        void changeEvent(QEvent* e);

    private:
        Q_DISABLE_COPY(EnvironmentEditor);

       void CreateInputContext();

        //! Convert Degoded texture into QImage format. Note! This method is planned to be removed when QImage can directly created using raw data pointer.
        //! @Param tex Reference to texture resource.
        //! @Return converted QImage.
        QImage ConvertToQImage(Foundation::TextureInterface &tex);

        //! Create new terrain paint brush that will display where user is painting the terrain.
        //! Paint area color is calculated using distance between middle point and individual vertex.
        //! This method will destroy the previous mesh until it begin to create a new mesh.
        //! @todo It's heavier to recreate the mesh each time the paint area location has changed, so
        //! in a future this code should be replaced with one that will update already created mesh each
        //! time the bush has moved on the scene.
        //! @param x_pos X coordinate where paint mesh middle point is located.
        //! @param y_pos Y coordinate where paint mesh middle point is located.
        //! @param color Paint area color.
        //! @param gradient_size how much larger or smaller gradient will be. Gradient range can be [0 - 1].
        void CreatePaintAreaMesh(int x_pos, int y_pos, const Color &color = Color(1.0f, 0.0f, 0.0f), float gradient_size = 0.75f);

        //! Finds the position on the terrain that the given screen-space x,y coordinates correspond to.
        //! The clientX and clientY parameters are in the main window coordinate frame. This function will do a raycast
        //! to the scene to find the proper position in the 2D scene to show the notification mesh at.
        void RaycastScreenPosToTerrainPos(int clientX, int clientY, int &mapX, int &mapY);

        void Update3DPaintNotificationMeshPosition(int mapX, int mapY);

        //! Called when the user moves his mouse outside the terrain when painting the terrain.
        void HidePaintMeshOnScene();

        //! If paint area has been created, this method will release it from the scene.
        void ReleasePaintMeshOnScene();

        //! Create a window for terrain editor.
        void InitEditorWindow();

        //! Create a new heightmap image that will show heightmap values in grayscale.format.
        void CreateHeightmapImage();

        //! Create red cube inside the heightmap image. Cube is used to show where mouse cursor is curently located 
        //! @param x_pos x position of mouse cursor.
        //! @param y_pos y position of mouse cursor.
        void UpdateHeightmapImagePaintArea(uint x_pos, uint y_pos);

        //! Clear old sky properties and create a new one for spesific sky type.
        //! @Param sky_type is used to tell what type of sky is in use, so right properties will be created.
        void CreateSkyProperties(OgreRenderer::SkyType sky_type);

        //! Clean all option widgets on the option scroll area.
        void CleanSkyProperties();

        //! Ask texture decoder for a texture resource.
        //! @Param index Index will tell us what terrain texture we are requesting range [0 - 3] where 0 is the lowest terrain texture and 3 is the highest.
        //! @Return request tag for the texture.
        request_tag_t RequestTerrainTexture(uint index);

        QWidget* GetPage(const QString& name);
        QWidget* GetCurrentPage();

        //! Asset_tags for terrain texture requests.
        std::vector<request_tag_t> terrain_texture_requests_;

        //! Terrain texture asset id list.
        std::vector<std::string> terrain_texture_id_list_;

        //! Return heightmap smallest and largest value (first = min and second = max).
        MinMaxValue GetMinMaxHeightmapValue(const EC_Terrain &terrain) const;

        //! Pointer for environment module.
        EnvironmentModule *environment_module_;

        //! Main widget for editor
        QWidget *editor_widget_;
        //Editor* editor_widget_;

        //! Brush size (small, medium and large).
        BrushSize brush_size_;

        //! Terrain actions (Flatten, Raise, Lower, Smooth, Roughen and Revert).
        ModifyLandAction action_;

        QCheckBox *daytime_override_checkbox_;
        QSlider *timeof_day_slider_;

        QColorDialog* sun_color_picker_;
        QVector<float> sun_color_;

        QColorDialog* ambient_color_picker_;
        QVector<float> ambient_color_;
        bool ambient_;

        //! sky type in use.
        OgreRenderer::SkyType sky_type_;

        InputContextPtr terrainPaintInputContext;
        //! Timer for terrain painting so that terrain is painted certain time.
        QTimer terrain_paint_timer_;

        //! Only used when terrain paint is active. Updates parameter when move event occurs.
        int mouse_position_[2];

        //! Is a user currently painting the terrain.
        bool edit_terrain_active_;

        //! Terrain paint mode is used to tell the editor when user can paint in 3d.
        TerrainPaintMode terrainPaintMode_;

        //! This object is created each time the CreatePaintAreaMesh is called.
        Ogre::ManualObject *manual_paint_object_;

        //! Scene node that will hold spesific paint area mesh object.
        Ogre::SceneNode *manual_paint_node_;
    };
}

#endif
