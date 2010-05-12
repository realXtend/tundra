// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_3DCanvas_EC_3DCanvas_h
#define incl_EC_3DCanvas_EC_3DCanvas_h

#include "ComponentInterface.h"
#include "Declare_EC.h"

#include <OgreMaterial.h>
#include <OgreTexture.h>

#include <QObject>
#include <QList>

class QWidget;
class QTimer;

namespace Scene      { class Entity; }
namespace Foundation { class Framework; }

class EC_3DCanvas : public Foundation::ComponentInterface
{
    Q_OBJECT
    DECLARE_EC(EC_3DCanvas);

public:
    ~EC_3DCanvas();

public slots:
    void SetWidget(QWidget *widget);
    void SetRefreshRate(int refresh_per_second);
    void SetSubmesh(uint submesh);
    void SetSubmeshes(const QList<uint> &submeshes);
    void SetEntity(Scene::Entity *entity);
    
    void Start();

private:
    explicit EC_3DCanvas(Foundation::ModuleInterface *module);
    void UpdateSubmeshes();

private slots:
    void Update();
private:
    QWidget *widget_;
    QList<uint> submeshes_;
    QTimer *refresh_timer_;

    Scene::Entity* entity_;
    Foundation::Framework *framework_;

    Ogre::TextureManager &texture_manager_;
    Ogre::MaterialManager &material_manager_;
    std::string material_name_;
    std::string texture_name_;

    int update_interval_msec_;
    bool update_internals_;
    bool paint;
};

#endif