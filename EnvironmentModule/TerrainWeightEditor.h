#include <QWidget>

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
        void DecomposeImageToCanvases(const QImage& image);
        QImage CreateImageFromCanvases();

        void InitializeCanvases();


        void InitializeConnections();
        
    public slots:
        void LoadWeightmapFromFile();
        void Initialize();
        void ApplyWeightTexture();
        void SetUseWeights(bool val);



    public:
        TerrainWeightEditor(EnvironmentModule* env_module);
        ~TerrainWeightEditor();
    };

}