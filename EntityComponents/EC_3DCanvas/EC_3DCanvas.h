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
    void SetSubmesh(uint submesh);
    void SetSubmeshes(const QList<uint> &submeshes);
    void SetEntity(Scene::Entity *entity);
    void Update();

private:
    explicit EC_3DCanvas(Foundation::ModuleInterface *module);
    void UpdateSubmeshes();

private:
    QWidget *widget_;
    QList<uint> submeshes_;

    Scene::Entity* entity_;
    Foundation::Framework *framework_;

    Ogre::MaterialPtr material_;
    Ogre::Texture *texture_;

    bool texture_set_;
};

#endif