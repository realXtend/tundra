/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   DebugStats.cpp
 *  @brief  DebugStatsModule shows information about internal core data structures in separate windows.
 *          Useful for verifying and understanding the internal state of the application.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "DebugStats.h"
#include "TimeProfilerWindow.h"

#include "Framework.h"
#include "UiAPI.h"
#include "ModuleManager.h"
#include "ConsoleCommandUtils.h"

#include "SceneAPI.h"
#include "SceneManager.h"
#include "Entity.h"
#include "Renderer.h"
#include "UiProxyWidget.h"
#include "ConsoleAPI.h"
#include "InputAPI.h"
#include "UiAPI.h"
#include "UiMainWindow.h"

#include <utility>
#include <QDebug>

#include <QCryptographicHash>

#include "MemoryLeakCheck.h"

using namespace std;

namespace DebugStats
{

const std::string DebugStatsModule::moduleName = std::string("DebugStats");

DebugStatsModule::DebugStatsModule() :
    IModule(NameStatic()),
    profilerWindow_(0)
{
}

DebugStatsModule::~DebugStatsModule()
{
    SAFE_DELETE(profilerWindow_);
}

void DebugStatsModule::PostInitialize()
{
#ifdef _WINDOWS
    QueryPerformanceCounter(&lastCallTime);
#endif

#ifdef PROFILING
    framework_->Console()->RegisterCommand("prof", "Shows the profiling window.", this, SLOT(ShowProfilingWindow()));

#endif

    framework_->Console()->RegisterCommand(CreateConsoleCommand("exec",
        "Invokes action execution in entity",
        ConsoleBind(this, &DebugStatsModule::Exec)));

    inputContext = framework_->Input()->RegisterInputContext("DebugStatsInput", 90);
    connect(inputContext.get(), SIGNAL(KeyPressed(KeyEvent *)), this, SLOT(HandleKeyPressed(KeyEvent *)));


//#ifdef Q_WS_WIN
// 
//    PDH::PerformanceMonitor monitor;
//    int treads = monitor.GetThreadCount();
//#endif 

    AddProfilerWidgetToUi();
}

void DebugStatsModule::HandleKeyPressed(KeyEvent *e)
{
    if (e->eventType != KeyEvent::KeyPressed || e->keyPressCount > 1)
        return;

    const QKeySequence showProfiler = framework_->Input()->KeyBinding("ShowProfilerWindow", QKeySequence(Qt::ShiftModifier + Qt::Key_P));
    if (QKeySequence(e->keyCode | e->modifiers) == showProfiler)
        ShowProfilingWindow();
}

void DebugStatsModule::AddProfilerWidgetToUi()
{
    if (profilerWindow_)
    {
        profilerWindow_->setVisible(!(profilerWindow_->isVisible()));
        return;
    }

    profilerWindow_ = new TimeProfilerWindow(framework_);
    profilerWindow_->setParent(framework_->Ui()->MainWindow());
    profilerWindow_->setWindowFlags(Qt::Tool);
    //profilerWindow_->move(100, 100);
    profilerWindow_->resize(650, 530);
    //UiProxyWidget *proxy = ui->AddWidgetToScene(profilerWindow_);
    connect(profilerWindow_, SIGNAL(Visible(bool)), SLOT(StartProfiling(bool)));

    //ui->AddWidgetToMenu(profilerWindow_, tr("Profiler"), tr("Developer Tools"), Application::InstallationDirectory() + "data/ui/images/menus/edbutton_MATWIZ_hover.png");
}

void DebugStatsModule::StartProfiling(bool visible)
{
    profilerWindow_->SetVisibility(visible);
    // -1 means start updating currently selected tab
    if (visible)
        profilerWindow_->OnProfilerWindowTabChanged(-1); 
}

ConsoleCommandResult DebugStatsModule::ShowProfilingWindow()
{
    // If the window is already created, bring it to front.
    if (profilerWindow_)
    {
        framework_->Ui()->BringWidgetToFront(profilerWindow_);
        return ConsoleResultSuccess();
    }
    else
        return ConsoleResultFailure("Profiler window has not been initialized, something went wrong on startup!");
}

void DebugStatsModule::Update(f64 frametime)
{
    RESETPROFILER;

#ifdef _WINDOWS
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    double timeSpent = Foundation::ProfilerBlock::ElapsedTimeSeconds(lastCallTime.QuadPart, now.QuadPart);
    lastCallTime = now;

    frameTimes.push_back(make_pair(*(boost::uint64_t*)&now, timeSpent));
    if (frameTimes.size() > 2048) // Maintain an upper bound in the frame history.
        frameTimes.erase(frameTimes.begin());

    if (profilerWindow_)
    {
        if (!profilerWindow_->isVisible())
            return;
        profilerWindow_->RedrawFrameTimeHistoryGraph(frameTimes);
        profilerWindow_->DoThresholdLogging();
    }

#endif
}

ConsoleCommandResult DebugStatsModule::Exec(const StringVector &params)
{
    if (params.size() < 2)
        return ConsoleResultFailure("Not enough parameters.");

    int id = ParseString<int>(params[0], 0);
    if (id == 0)
        return ConsoleResultFailure("Invalid value for entity ID. The ID must be an integer and unequal to zero.");

    Scene::ScenePtr scene = GetFramework()->Scene()->GetDefaultScene();
    if (!scene)
        return ConsoleResultFailure("No active scene.");

    EntityPtr entity = scene->GetEntity(id);
    if (!entity)
        return ConsoleResultFailure("No entity found for entity ID " + params[0]);

    QStringList execParameters;
    
    if (params.size() >= 3)
    {
        int type = ParseString<int>(params[2], 0);
        for(size_t i = 3; i < params.size(); ++i)
            execParameters << params[i].c_str();
        
        if (id != 0)
            entity->Exec((EntityAction::ExecutionType)type, params[1].c_str(), execParameters);
    }
    else
        entity->Exec(EntityAction::Local, params[1].c_str(), execParameters);

    return ConsoleResultSuccess();
}

}

void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

extern "C"
{
__declspec(dllexport) void TundraPluginMain(Foundation::Framework *fw)
{
    IModule *module = new DebugStats::DebugStatsModule();
    fw->GetModuleManager()->DeclareStaticModule(module);
}
}
