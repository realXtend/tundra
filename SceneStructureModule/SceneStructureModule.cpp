/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneStructureModule.cpp
 *  @brief  Provides Scene Structure window and raycast drag-and-drop import of
 *          mesh, .scene, .xml and .nbf files to the main window.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "SceneStructureModule.h"
#include "SceneStructureWindow.h"

#include "Console.h"
#include "UiServiceInterface.h"
#include "Input.h"
#include "RenderServiceInterface.h"
#include "SceneImporter.h"
#include "EC_OgreCamera.h"
#include "EC_Placeable.h"
#include "NaaliUi.h"
#include "NaaliGraphicsView.h"
#include "LoggingFunctions.h"

DEFINE_POCO_LOGGING_FUNCTIONS("SceneStructure");

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

    connect(framework_->Ui()->GraphicsView(), SIGNAL(DragEnterEvent(QDragEnterEvent *)), SLOT(HandleDragEnterEvent(QDragEnterEvent *)));
    connect(framework_->Ui()->GraphicsView(), SIGNAL(DragMoveEvent(QDragMoveEvent *)), SLOT(HandleDragMoveEvent(QDragMoveEvent *)));
    connect(framework_->Ui()->GraphicsView(), SIGNAL(DropEvent(QDropEvent *)), SLOT(HandleDropEvent(QDropEvent *)));
}

void SceneStructureModule::InstantiateContent(const QString &filename, Vector3df &worldPos, bool clearScene)
{
    const Scene::ScenePtr &scene = framework_->GetDefaultWorldScene();
    if (!scene)
        return;

    // If zero vector position, query position from the user.
    if (worldPos == Vector3df())
    {
        bool ok;
        QString posString = QInputDialog::getText(0, tr("Position"), tr("position (x;y;z):"), QLineEdit::Normal, "20.00;20.00;20.00", &ok);
        if (!ok || posString.isEmpty())
            return;

        posString.replace(',', '.');
        QStringList pos = posString.split(';');
        if (pos.size() > 0)
            worldPos.x = pos[0].toFloat();
        if (pos.size() > 1)
            worldPos.y = pos[1].toFloat();
        if (pos.size() > 2)
            worldPos.z = pos[2].toFloat();
    }

    if (filename.toLower().indexOf(".scene") != -1)
    {
        boost::filesystem::path path(filename.toStdString());
        std::string dirname = path.branch_path().string();

        TundraLogic::SceneImporter importer(framework_);
        ///\todo Take into account asset sources.
        importer.Import(scene, filename.toStdString(), dirname, "./data/assets",
            Transform(worldPos, Vector3df(0,0,0), Vector3df(1,1,1)), AttributeChange::Default, clearScene, true, false);
    }
    else if (filename.toLower().indexOf(".mesh") != -1)
    {
        boost::filesystem::path path(filename.toStdString());
        std::string dirname = path.branch_path().string();

        TundraLogic::SceneImporter importer(framework_);
        Scene::EntityPtr entity = importer.ImportMesh(scene, filename.toStdString(), dirname, "./data/assets",
            Transform(worldPos, Vector3df(0,0,0), Vector3df(1,1,1)), std::string(), AttributeChange::Default, true);
        if (entity)
            scene->EmitEntityCreated(entity, AttributeChange::Default);
    }
    else if (filename.toLower().indexOf(".xml") != -1)
    {
        scene->LoadSceneXML(filename.toStdString(), clearScene, AttributeChange::Replicate);
    }
    else if (filename.toLower().indexOf(".nbf") != -1)
    {
        scene->CreateContentFromBinary(filename, true, AttributeChange::Replicate);
    }
    else
    {
        LogError("Unsupported file extension: " + filename.toStdString());
    }
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

void SceneStructureModule::HandleDragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
        e->accept();
}

void SceneStructureModule::HandleDragMoveEvent(QDragMoveEvent *e)
{
    if (e->mimeData()->hasUrls())
        e->accept();
}

void SceneStructureModule::HandleDropEvent(QDropEvent *e)
{
    if (e->mimeData()->hasUrls())
    {
        Foundation::RenderServiceInterface *renderer = framework_->GetService<Foundation::RenderServiceInterface>();
        if (!renderer)
            return;

        Vector3df worldPos;
        Foundation::RaycastResult res = renderer->Raycast(e->pos().x(), e->pos().y());
        if (!res.entity_)
        {
            // No entity hit, use camera's position with hard-coded offset.
            const Scene::ScenePtr &scene = framework_->GetDefaultWorldScene();
            if (!scene)
                return;

            Scene::EntityList cameras = scene->GetEntitiesWithComponent(EC_OgreCamera::TypeNameStatic());
            if (cameras.empty())
                return;

            foreach(Scene::EntityPtr cam, cameras)
                if (cam->GetComponent<EC_OgreCamera>()->IsActive())
                {
                    EC_Placeable *placeable = cam->GetComponent<EC_Placeable>().get();
                    if (placeable)
                    {
                        Vector3df offset = placeable->transform.Get().rotation;
                        offset.normalize();
                        offset *= 20;
                        worldPos = placeable->transform.Get().position + offset;
                        break;
                    }
                }
        }
        else
            worldPos = res.pos_;

        foreach (QUrl url, e->mimeData()->urls())
        {
            QString filename = url.path();
#ifdef _WINDOWS
            // We have '/' as the first char on windows and the filename
            // is not identified as a file properly. But on other platforms the '/' is valid/required.
            filename = filename.mid(1);
#endif
            InstantiateContent(filename, worldPos, false);
        }

        e->acceptProposedAction();
    }
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

POCO_BEGIN_MANIFEST(IModule)
   POCO_EXPORT_CLASS(SceneStructureModule)
POCO_END_MANIFEST
