/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   DebugStats.cpp
 *  @brief  ScenePersistenceModule shows information about internal core data structures in separate windows.
 *          Useful for verifying and understanding the internal state of the application.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ScenePersistenceModule.h"
#include "ConsoleCommandServiceInterface.h"

#include "MemoryLeakCheck.h"

#include "Entity.h"
#include "Framework.h"
#include "SceneManager.h"

#include "sqlite3.h"
#include "kNet.h"

using namespace std;

const std::string ScenePersistenceModule::moduleName = std::string("ScenePersistence");

ScenePersistenceModule::ScenePersistenceModule()
:IModule(NameStatic()), 
db(0)
{
}

ScenePersistenceModule::~ScenePersistenceModule()
{
    ClosePersistingStorage();
}

void ScenePersistenceModule::PostInitialize()
{
    RegisterConsoleCommand(Console::CreateCommand("persist", 
        "Starts persistent storage",
        Console::Bind(this, &ScenePersistenceModule::StartPersistenceCommand)));

    /*
    framework_->Console()->RegisterCommand("prof", "Shows the profiling window.", this, SLOT(ShowProfilingWindow()));

    RegisterConsoleCommand(Console::CreateCommand("rin", 
        "Sends a random network message in.",
        Console::Bind(this, &ScenePersistenceModule::SendRandomNetworkInPacket)));

    RegisterConsoleCommand(Console::CreateCommand("rout", 
        "Sends a random network message out.",
        Console::Bind(this, &ScenePersistenceModule::SendRandomNetworkOutPacket)));
#endif

    RegisterConsoleCommand(Console::CreateCommand("Participant", 
        "Shows the participant window.",
        Console::Bind(this, &ScenePersistenceModule::ShowParticipantWindow)));

    RegisterConsoleCommand(Console::CreateCommand("iddqd",
        "Requests god-mode on from the server.",
        Console::Bind(this, &ScenePersistenceModule::RequestGodMode)));

    RegisterConsoleCommand(Console::CreateCommand("kick",
        "Kicks user out from the server. Usage: \"kick(fullname)\"",
        Console::Bind(this, &ScenePersistenceModule::KickUser)));

    RegisterConsoleCommand(Console::CreateCommand("dumptextures",
        "Dumps all currently existing J2K decoded textures as PNG files into the viewer working directory.",
        Console::Bind(this, &ScenePersistenceModule::DumpTextures)));
        
    RegisterConsoleCommand(Console::CreateCommand("exec",
        "Invokes action execution in entity",
        Console::Bind(this, &ScenePersistenceModule::Exec)));

    frameworkEventCategory_ = framework_->GetEventManager()->QueryEventCategory("Framework");


//#ifdef Q_WS_WIN
// 
//    PDH::PerformanceMonitor monitor;
//    int treads = monitor.GetThreadCount();
//#endif 

    AddProfilerWidgetToUi();
    */
}

Console::CommandResult ScenePersistenceModule::StartPersistenceCommand(const StringVector &params)
{
    StartPersistingStorage("world.db");

    const Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
    connect(scene.get(), SIGNAL(EntityCreated(Scene::Entity*, AttributeChange::Type)), this,
        SLOT(EntityCreated(Scene::Entity*, AttributeChange::Type)));

    connect(scene.get(), SIGNAL(EntityRemoved(Scene::Entity*, AttributeChange::Type)), this,
        SLOT(EntityRemoved(Scene::Entity*, AttributeChange::Type)));

    connect(scene.get(), SIGNAL(ComponentAdded(Scene::Entity*, IComponent*, AttributeChange::Type)), this,
        SLOT(ComponentAdded(Scene::Entity*, IComponent*, AttributeChange::Type)));

    connect(scene.get(), SIGNAL(ComponentRemoved(Scene::Entity*, IComponent*, AttributeChange::Type)), this,
        SLOT(ComponentRemoved(Scene::Entity*, IComponent*, AttributeChange::Type)));

    connect(scene.get(), SIGNAL(AttributeChanged(IComponent*, IAttribute*, AttributeChange::Type)), this,
        SLOT(AttributeChanged(IComponent*, IAttribute*, AttributeChange::Type)));

    return Console::ResultSuccess();
}

void ScenePersistenceModule::StartPersistingStorage(QString filename)
{
    if (sqlite3_open_v2(filename.toStdString().c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL) != SQLITE_OK)
        throw Exception("");
    if (sqlite3_extended_result_codes(db, true) != SQLITE_OK)
        throw Exception("");

    CreateTables();
    CreateStatements();
}

/// Closes the current storage database file and stops listening to any scene changes.
void ScenePersistenceModule::ClosePersistingStorage()
{
    ///\todo call sqlite3_finalize for all sqlite statements.

    sqlite3_close(db);
    db = 0;
}

void ScenePersistenceModule::CreateTables()
{
    assert(db);

    const char entities[] = 
        "CREATE TABLE IF NOT EXISTS entities (id INTEGER PRIMARY KEY)";
    
    if (sqlite3_exec(db, entities, NULL, NULL, NULL) != SQLITE_OK)
        throw Exception(entities);

    const char components[] = 
        "CREATE TABLE IF NOT EXISTS components ( \
            entityID INTEGER NOT NULL, \
            compTypename TEXT NOT NULL, \
            compName TEXT, \
            networkSyncEnabled INTEGER NOT NULL, \
            defaultChangeType INTEGER NOT NULL, \
            FOREIGN KEY (entityID) REFERENCES entities (id))";

    if (sqlite3_exec(db, components, NULL, NULL, NULL) != SQLITE_OK)
        throw Exception(components);

    const char attributes[] =
        "CREATE TABLE IF NOT EXISTS attributes ( \
            entityID INTEGER NOT NULL, \
            compTypename TEXT NOT NULL, \
            compName TEXT, \
            attrName TEXT NOT NULL, \
            attrType TEXT NOT NULL, \
            attrValue BLOB NOT NULL)";
//            FOREIGN KEY (compTypename) REFERENCES components (compTypename))";
            
    if (sqlite3_exec(db, attributes, NULL, NULL, NULL) != SQLITE_OK)
        throw Exception(attributes);
            
}

void ScenePersistenceModule::CreateStatements()
{
    const char insertEntity[] = "INSERT INTO entities (id) VALUES (?1)";
    const char removeEntity[] = "DELETE FROM entities WHERE id=?1";

    const char insertComponent[] = "INSERT INTO components "
        "(entityID, compTypename, compName, networkSyncEnabled, defaultChangeType) "
        "VALUES (?1, ?2, ?3, ?4, ?5)";

//    const char updateComponent[] = "UPDATE components SET"
//        "compTypename=?1, compName=?2, networkSyncEnabled=?3, defaultChangeType=?4 WHERE entityID=?5";

    const char removeComponent[] = "DELETE FROM components WHERE entityID=?1 AND compTypename=?2 AND compName=?3";

    const char insertAttribute[] = "INSERT INTO attributes "
        "(entityID, compTypename, compName, attrName, attrType, attrValue) VALUES (?1, ?2, ?3, ?4, ?5, ?6)";

    const char findAttribute[] = "SELECT * FROM attributes "
        "WHERE entityID=?1 AND compTypename=?2 AND compName=?3 AND attrName=?4";

    const char updateAttribute[] = "UPDATE attributes SET "
        "attrType=?1, attrValue=?2 WHERE entityID=?3 AND compTypename=?4 AND compName=?5 AND attrName=?6";

//    const char removeAttribute[] = "DELETE FROM attributes"

    if (sqlite3_prepare_v2(db, insertEntity, -1, &insertEntityStatement, NULL) != SQLITE_OK)
        throw Exception(insertEntity);
    if (sqlite3_prepare_v2(db, removeEntity, -1, &removeEntityStatement, NULL) != SQLITE_OK)
        throw Exception(removeEntity);
    if (sqlite3_prepare_v2(db, insertComponent, -1, &insertComponentStatement, NULL) != SQLITE_OK)
        throw Exception(insertComponent);
    if (sqlite3_prepare_v2(db, removeComponent, -1, &removeComponentStatement, NULL) != SQLITE_OK)
        throw Exception(removeComponent);
    if (sqlite3_prepare_v2(db, findAttribute, -1, &findAttributeStatement, NULL) != SQLITE_OK)
        throw Exception(findAttribute);
    if (sqlite3_prepare_v2(db, insertAttribute, -1, &insertAttributeStatement, NULL) != SQLITE_OK)
        throw Exception(insertAttribute);
    if (sqlite3_prepare_v2(db, updateAttribute, -1, &updateAttributeStatement, NULL) != SQLITE_OK)
        throw Exception(updateAttribute);
}

void ScenePersistenceModule::EntityCreated(Scene::Entity* entity, AttributeChange::Type change)
{
    if (!db || !insertEntityStatement)
        return;

    sqlite3_reset(insertEntityStatement);
    sqlite3_clear_bindings(insertEntityStatement);
    sqlite3_bind_int(insertEntityStatement, 1, entity->GetId());
    
    if (sqlite3_step(insertEntityStatement) != SQLITE_DONE)
        throw Exception(sqlite3_sql(insertEntityStatement));
}

void ScenePersistenceModule::EntityRemoved(Scene::Entity* entity, AttributeChange::Type change)
{
    if (!db)
        return;

    sqlite3_reset(removeEntityStatement);
    sqlite3_clear_bindings(removeEntityStatement);
    sqlite3_bind_int(removeEntityStatement, 1, entity->GetId());
    
    if (sqlite3_step(removeEntityStatement) != SQLITE_DONE)
        throw Exception(sqlite3_sql(removeEntityStatement));

    ///\todo Here check that all entries of the component and attribute table that refer to this entity
    /// as foreign key are dropped.
}

void ScenePersistenceModule::ComponentAdded(Scene::Entity* entity, IComponent* comp, AttributeChange::Type change)
{
    if (!db)
        return;

    sqlite3_reset(insertComponentStatement);
    sqlite3_clear_bindings(insertComponentStatement);
    sqlite3_bind_int(insertComponentStatement, 1, entity->GetId());
    sqlite3_bind_text(insertComponentStatement, 2, comp->TypeName().toStdString().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(insertComponentStatement, 3, comp->Name().toStdString().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(insertComponentStatement, 4, comp->GetNetworkSyncEnabled() ? 1 : 0);
    sqlite3_bind_int(insertComponentStatement, 5, 0); // defaultChangeType.
    
    if (sqlite3_step(insertComponentStatement) != SQLITE_DONE)
        throw Exception(sqlite3_sql(insertComponentStatement));
}

void ScenePersistenceModule::ComponentRemoved(Scene::Entity* entity, IComponent* comp, AttributeChange::Type change)
{
    if (!db)
        return;

    sqlite3_reset(removeComponentStatement);
    sqlite3_clear_bindings(removeComponentStatement);
    sqlite3_bind_int(removeComponentStatement, 1, entity->GetId());
    sqlite3_bind_text(removeComponentStatement, 2, comp->TypeName().toStdString().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(removeComponentStatement, 3, comp->Name().toStdString().c_str(), -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(removeComponentStatement) != SQLITE_DONE)
        throw Exception(sqlite3_sql(removeComponentStatement));
}

void ScenePersistenceModule::AttributeChanged(IComponent* comp, IAttribute* attribute, AttributeChange::Type change)
{
    if (!db)
        return;

    sqlite3_reset(findAttributeStatement);
    sqlite3_clear_bindings(findAttributeStatement);
    sqlite3_bind_int(findAttributeStatement, 1, comp->GetParentEntity()->GetId());
    sqlite3_bind_text(findAttributeStatement, 2, comp->TypeName().toStdString().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(findAttributeStatement, 3, comp->Name().toStdString().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(findAttributeStatement, 4, attribute->GetName(), -1, SQLITE_TRANSIENT);

    kNet::DataSerializer ds(2048); ///\todo Maintain proper size.
    attribute->ToBinary(ds);

    switch(sqlite3_step(findAttributeStatement))
    {
    case SQLITE_ROW:
        sqlite3_reset(updateAttributeStatement);
        sqlite3_clear_bindings(updateAttributeStatement);
        sqlite3_bind_text(updateAttributeStatement, 1, attribute->TypenameToString().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_blob(updateAttributeStatement, 2, ds.GetData(), ds.BytesFilled(), SQLITE_TRANSIENT);
        sqlite3_bind_int(updateAttributeStatement, 3, comp->GetParentEntity()->GetId());
        sqlite3_bind_text(updateAttributeStatement, 4, comp->TypeName().toStdString().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(updateAttributeStatement, 5, comp->Name().toStdString().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(updateAttributeStatement, 6, attribute->GetName(), -1, SQLITE_TRANSIENT);
       
        if (sqlite3_step(updateAttributeStatement) != SQLITE_DONE)
            throw Exception(sqlite3_sql(updateAttributeStatement));

        break;
    default:
        sqlite3_reset(insertAttributeStatement);
        sqlite3_clear_bindings(insertAttributeStatement);
        sqlite3_bind_int(insertAttributeStatement, 1, comp->GetParentEntity()->GetId());
        sqlite3_bind_text(insertAttributeStatement, 2, comp->TypeName().toStdString().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(insertAttributeStatement, 3, comp->Name().toStdString().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(insertAttributeStatement, 4, attribute->GetName(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(insertAttributeStatement, 5, attribute->TypenameToString().c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_blob(insertAttributeStatement, 6, ds.GetData(), ds.BytesFilled(), SQLITE_TRANSIENT);
       
        if (sqlite3_step(insertAttributeStatement) != SQLITE_DONE)
            throw Exception(sqlite3_sql(insertAttributeStatement));

        break;
    }
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

POCO_BEGIN_MANIFEST(IModule)
    POCO_EXPORT_CLASS(ScenePersistenceModule)
POCO_END_MANIFEST 
