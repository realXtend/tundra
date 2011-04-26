// For conditions of distribution and use, see copyright notice in license.txt

#include "ForwardDefines.h"
#include "CoreTypes.h"

#include <QWidget>

class SceneManager;

namespace Environment
{
    class TerrainWeightEditor : public QWidget
    {
        Q_OBJECT

    public:
        explicit TerrainWeightEditor(Framework *fw);
        ~TerrainWeightEditor();

    public slots:
        void RecreateBrush();
        void LoadWeightmapFromFile();
        void Initialize();
        void ApplyWeightTexture();
        void SetUseWeights(bool val);
        void BrushSizeChanged(int val);
        void BrushFalloffChanged(double val);
        void BrushModifierChanged(int val);
        void HandleMouseEventFromCanvas(QMouseEvent *ev, QString name);

    private:
        Framework *fw_;
        QWidget* editor_widget_;
        SceneManager* scene_manager_;
        SceneManager* GetSceneManager();
        int brush_size_;
        int brush_modifier_;
        f32 falloff_percentage_;
        int neutral_color_;
        int brush_size_max_;

        void DecomposeImageToCanvases(const QImage& image);
        QImage CreateImageFromCanvases();

        void InitializeCanvases();
        void InitializeConnections();

    signals:
        void BrushValueChanged();
    };
}
