// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_TerrainEditor_h
#define incl_RexLogic_TerrainEditor_h

#include <Foundation.h>
#include "UICanvas.h"

#include <QObject>
#include <QPair>

class QImage;

namespace RexLogic
{
    class RexLogicModule;
    class Terrain;
    class EC_Terrain;

    typedef QPair<float, float> MinMaxValue;

    class TerrainEditor: public QObject
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
        TerrainEditor(RexLogicModule *rexlogicmodule);

        //! Destructor
        ~TerrainEditor();

        //! Change editor window visibility.
        void Toggle();

        static const int cHeightmapImageWidth  = 256;
        static const int cHeightmapImageHeight = 256;

    public slots:
        //! Send a terrain paint message to server.
        void SendModifyLandMessage();

        //! Get a new terrain texture from rexlogic.
        void UpdateTerrain();

        //! Called when qMouseEvent event is generated.
        void HandleMouseEvent(QMouseEvent *ev);

        //! Called when brush size option has changed.
        void BrushSizeChanged();

        //! Called when terrain action option has changed.
        void PaintActionChanged();

    private:
        //! Create a window for terrain editor.
        void InitEditorWindow();

        //! Create a new heightmap image that will show heightmap values in grayscale.format
        void CreateHeightmapImage();

        //! Return heightmap smallest and largest value (first = min and second = max).
        MinMaxValue GetMinMaxHeightmapValue(EC_Terrain &terrain);

        //! Pointer for rexLogicModule.
        RexLogicModule *rexlogicmodule_;

        //! Canvas for terrain editor window
        boost::shared_ptr<QtUI::UICanvas> canvas_;

        //! Main widget for editor
        QWidget *editor_widget_;

        //! Terrain geometry information.
        boost::shared_ptr<Terrain> terrain_;

        //! Brush size (small, medium and large).
        BrushSize brush_size_;

        //! Terrain actions (Flatten, Raise, Lower, Smooth, Roughen and Revert).
        ModifyLandAction action_;

        //! Mouse press flags
        //Core::u8 mouse_press_flag_;

        //! 
        //Core::Real start_height_;
    };
}

#endif