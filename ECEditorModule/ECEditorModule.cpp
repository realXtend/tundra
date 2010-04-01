// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ECEditorModule.h"
#include "ECEditorWindow.h"

namespace ECEditor
{
    std::string ECEditorModule::name_static_ = "ECEditorModule";
    
    ECEditorModule::ECEditorModule() :
        ModuleInterfaceImpl(name_static_),
        editor_window_(0)
    {
    }
    
    ECEditorModule::~ECEditorModule()
    {
    }
    
    void ECEditorModule::Load()
    {
    }

    void ECEditorModule::Initialize()
    {
        editor_window_ = new ECEditorWindow(GetFramework());
    }

    void ECEditorModule::PostInitialize()
    {
        event_manager_ = framework_->GetEventManager();
    }

    void ECEditorModule::Uninitialize()
    {
        if (editor_window_)
        {
            editor_window_->deleteLater();
            editor_window_ = 0;
        }
    }

    void ECEditorModule::Update(f64 frametime)
    {
        RESETPROFILER;
    }

    bool ECEditorModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
    {
        return false;
    }

}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace ECEditor;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(ECEditorModule)
POCO_END_MANIFEST 

