#include <QWidget>
#include "CoreTypes.h"
namespace Scene
{
    class SceneManager;
}



namespace Environment
{
    class EnvironmentModule;

    class TerrainWeightEditor : public QWidget
    {
        Q_OBJECT

    private:
        EnvironmentModule* env_module_;
        QWidget* editor_widget_;
        Scene::SceneManager* scene_manager_;
        Scene::SceneManager* GetSceneManager();
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


    public:
        TerrainWeightEditor(EnvironmentModule* env_module);
        ~TerrainWeightEditor();
    };

}