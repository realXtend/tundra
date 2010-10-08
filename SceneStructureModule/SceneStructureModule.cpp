/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneStructureModule.cpp
 *  @brief  
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "SceneStructureModule.h"
#include "SceneStructureWindow.h"

#include "Console.h"
#include "UiServiceInterface.h"
#include "Input.h"

#include "MemoryLeakCheck.h"

std::string SceneStructureModule::typeNameStatic = "SceneStructure";

SceneStructureModule::SceneStructureModule() :
    IModule(typeNameStatic),
    sceneWindow(0)
{
}

SceneStructureModule::~SceneStructureModule()
{
    SAFE_DELETE(sceneWindow);
}

void SceneStructureModule::PostInitialize()
{
    framework_->Console()->RegisterCommand("scenestruct", "Shows the Scene Structure window.", this, SLOT(ShowSceneStructureWindow()));
    inputContext = framework_->GetInput()->RegisterInputContext("SceneStructureInput", 90);
    //input->SetTakeKeyboardEventsOverQt(true);
    connect(inputContext.get(), SIGNAL(KeyPressed(KeyEvent *)), this, SLOT(HandleKeyPressed(KeyEvent *)));
}

void SceneStructureModule::ShowSceneStructureWindow()
{
    UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();
    if (!ui)
        return;

    if (sceneWindow)
    {
        ui->ShowWidget(sceneWindow);
        return;
    }

    sceneWindow = new SceneStructureWindow(framework_);
    sceneWindow->move(200,200);
    sceneWindow->SetScene(framework_->GetDefaultWorldScene());

    ui->AddWidgetToScene(sceneWindow);
    ui->ShowWidget(sceneWindow);
}

void SceneStructureModule::HandleKeyPressed(KeyEvent *e)
{
    if (e->eventType != KeyEvent::KeyPressed || e->keyPressCount > 1)
        return;

    Input &input = *framework_->GetInput();

    const QKeySequence showSceneStruct = input.KeyBinding("ShowSceneStructureWindow", QKeySequence(Qt::ShiftModifier + Qt::Key_S));
    if (QKeySequence(e->keyCode | e->modifiers) == showSceneStruct)
        ShowSceneStructureWindow();
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

POCO_BEGIN_MANIFEST(IModule)
   POCO_EXPORT_CLASS(SceneStructureModule)
POCO_END_MANIFEST
