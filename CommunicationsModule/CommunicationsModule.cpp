// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h" 

#include "CommunicationsModule.h"
#include "Service.h"

namespace CommunicationsService
{
    std::string CommunicationsModule::type_name_static_ = "Communications";

    CommunicationsModule::CommunicationsModule()
        : ModuleInterface(type_name_static_)
    {
    }

    CommunicationsModule::~CommunicationsModule()
    {
    }

    void CommunicationsModule::Load()
    {
    }

    void CommunicationsModule::Unload()
    {
    }

    void CommunicationsModule::Initialize() 
    {
        communications_service_ = Service::IntancePtr();
        if (!communications_service_.get())
        {
            QString message("Cannot get communications service intance.");
            LogError(message.toStdString());
            return;
        }

        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_Communications, communications_service_);
        QString message("Communications service registered.");
        LogInfo(message.toStdString());
    }

    void CommunicationsModule::PostInitialize()
    {
//        CommunicationsWidget = new CommunicationsWidget();
        // Create Communication widgets
        //boost::shared_ptr<UiServices::UiModule> ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>().lock();
        //if (ui_module.get())
        {
            //UiServices::UiWidgetProperties widget_properties(name, UiServices::SceneWidget);
            //im_ui_proxy_widget_ = ui_module->GetInworldSceneController()->AddWidgetToScene(im_ui_, widget_properties);
            //if (im_ui_proxy_widget_)
            //    ui_module->GetInworldSceneController()->SetImWidget(im_ui_proxy_widget_);
        }
    }

    void CommunicationsModule::Uninitialize()
    {
        if (communications_service_.get())
        {
            framework_->GetServiceManager()->UnregisterService(communications_service_);
            QString message("Communications service unregistered.");
            LogInfo(message.toStdString());
        }
    }

    void CommunicationsModule::Update(f64 frametime)
    {
    }

    bool CommunicationsModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
    {
        return false;
    }

} // CommunicationsService

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace CommunicationsService;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
POCO_EXPORT_CLASS(CommunicationsModule)
POCO_END_MANIFEST
