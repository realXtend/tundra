/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   G3dwhModule.cpp
 *  @brief  Brief description of the module goes here
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "G3dwhModule.h"
#include "G3dwhDialog.h"

#include "EventManager.h"
#include "MemoryLeakCheck.h"

#include <QMessageBox>

#include "IModule.h"
#include "IComponent.h"
#include "UiAPI.h"
#include "UiMainWindow.h"

#include "Framework.h"
#include "AssetAPI.h"
#include "IAssetStorage.h"

#include "QDebug"

#include "LoggingFunctions.h"

DEFINE_POCO_LOGGING_FUNCTIONS("G3dwh");

std::string G3dwhModule::type_name_static_ = "G3dwh";

G3dwhModule::G3dwhModule() :
    IModule(type_name_static_),warehouse_(0)
{
}

G3dwhModule::~G3dwhModule()
{
    delete warehouse_;
}

void G3dwhModule::PreInitialize()
{
}

void G3dwhModule::Initialize()
{
    // Register all services the module offers here
}

void G3dwhModule::PostInitialize()
{
    // Set event categories, i.e:
    // framework_category_ = framework_->GetEventManager()->QueryEventCategory("Framework");


}

bool G3dwhModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
{
    // 
    return false;
}

void G3dwhModule::Uninitialize()
{
    // Unregister all services the module offers here
}

void G3dwhModule::Update(f64 frametime)
{
}

void G3dwhModule::ShowG3dwhWindow()
{
    if(warehouse_.isNull())
    {
        std::string modelPath=GetFramework()->GetPlatform()->GetApplicationDataDirectory();

        warehouse_ = new G3dwhDialog(framework_, modelPath, GetFramework()->Ui()->MainWindow());
        connect(warehouse_,SIGNAL(finished(int)),this,SLOT(nullifyDialog()));
        warehouse_->show();

        std::vector<AssetStoragePtr> storages = framework_->Asset()->GetAssetStorages();

        for(size_t i = 0; i < storages.size(); ++i)
        {
            QString storage = storages[i]->ToString();
            qDebug()<<storage;

            if(storage.contains("Scene ",Qt::CaseSensitive))
            {
                QString scenePathEdit = storage.section("(",1,1);
                QString scenePath = scenePathEdit.replace(")","/",Qt::CaseSensitive);
                LogInfo( scenePath.toStdString());
                warehouse_->setScenePath(scenePath);
                warehouse_->disableButtons(false);

                return;
            }
        }
        LogInfo("No default storage added, models can't be added\nAdd new storage to start creating scene");
        warehouse_->disableButtons(true);
    }
    else
    {
        warehouse_->close();
        warehouse_=NULL;
    }

}

void G3dwhModule::nullifyDialog()
{
    warehouse_=NULL;
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

POCO_BEGIN_MANIFEST(IModule)
POCO_EXPORT_CLASS(G3dwhModule)
POCO_END_MANIFEST

