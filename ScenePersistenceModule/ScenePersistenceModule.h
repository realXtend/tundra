/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ScenePersistenceModule.h
 *  @brief  Implements a persisting storage for the scene.
 */

#ifndef incl_ScenePersistenceModule_h
#define incl_ScenePersistenceModule_h

#include "ScenePersistenceModuleApi.h"
#include "IModule.h"
#include "ModuleLoggingFunctions.h"
#include "RexTypes.h"
#include "AttributeChangeType.h"

#include <QObject>
#include <QPointer>

struct sqlite3;
struct sqlite3_stmt;

class SCENEPERSISTENCE_MODULE_API ScenePersistenceModule : public QObject, public IModule
{
    Q_OBJECT

public:
    ScenePersistenceModule();
    virtual ~ScenePersistenceModule();

    void PostInitialize();

    MODULE_LOGGING_FUNCTIONS

    /// Returns name of this module. Needed for logging.
    static const std::string &NameStatic() { return moduleName; }

    /// Name of this module.
    static const std::string moduleName;

    Console::CommandResult StartPersistenceCommand(const StringVector &params);

public slots:

    /// Closes the current storage database file, opens a new one, and immediately stores all the entities
    /// in the scene to that database. After returning, this module actively listens to changes to the
    /// scene and immediately persists those changes to the given filename.
    void StartPersistingStorage(QString filename);

    /// Closes the current storage database file and stops listening to any scene changes.
    void ClosePersistingStorage();

    void EntityCreated(Scene::Entity* entity, AttributeChange::Type change);
    void EntityRemoved(Scene::Entity* entity, AttributeChange::Type change);
    void ComponentAdded(Scene::Entity* entity, IComponent* comp, AttributeChange::Type change);
    void ComponentRemoved(Scene::Entity* entity, IComponent* comp, AttributeChange::Type change);
    void AttributeChanged(IComponent* comp, IAttribute* attribute, AttributeChange::Type change);

private:
    Q_DISABLE_COPY(ScenePersistenceModule);

    sqlite3 *db;

    sqlite3_stmt *insertEntityStatement;
    sqlite3_stmt *removeEntityStatement;
    sqlite3_stmt *insertComponentStatement;
    sqlite3_stmt *removeComponentStatement;
    sqlite3_stmt *findAttributeStatement;
    sqlite3_stmt *updateAttributeStatement;
    sqlite3_stmt *insertAttributeStatement;

    void CreateTables();
    void CreateStatements();
};

#endif
