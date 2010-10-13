// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Physics_PhysicsModule_h
#define incl_Physics_PhysicsModule_h

#include "PhysicsModuleApi.h"
#include "IModule.h"
#include "ModuleLoggingFunctions.h"
#include "RexTypes.h"
#include "SceneManager.h"

#include <QObject>

namespace Physics
{

class PhysicsWorld;

class PHYSICS_MODULE_API PhysicsModule : public QObject, public IModule
{
    Q_OBJECT

public:
    PhysicsModule();
    virtual ~PhysicsModule();

    MODULE_LOGGING_FUNCTIONS

    //! IModule override.
    void PostInitialize();

    //! IModule override.
    void Update(f64 frametime);

    //! Returns name of this module. Needed for logging.
    static const std::string &NameStatic() { return moduleName; }

    //! Name of this module.
    static const std::string moduleName;
    
public slots:
    //! Create a physics world for a scene
    PhysicsWorld* CreatePhysicsWorldForScene(Scene::ScenePtr scene);
    
    //! Return the physics world for a scene if it exists
    PhysicsWorld* GetPhysicsWorldForScene(Scene::ScenePtr scene);
    
private slots:
    //! Scene has been removed, so delete also the physics world (if exists)
    void OnSceneRemoved(QObject* scene);
    
private:
    typedef std::map<Scene::SceneManager*, boost::shared_ptr<PhysicsWorld> > PhysicsWorldMap;
    
    //! Map of physics worlds assigned to scenes
    PhysicsWorldMap physicsWorlds_;
};

}

#endif
