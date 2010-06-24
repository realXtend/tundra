/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   InWorldChatModule.cpp
 *  @brief  Simple OpenSim world chat module. Listens for ChatFromSimulator packets and shows the chat on the UI.
 *          Outgoing chat sent using ChatFromViewer packets. Manages EC_ChatBubbles, EC_Billboards, chat logging etc.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "InWorldChatModule.h"
//#include "ChatWidget.h"

#include "EC_ChatBubble.h"
#include "EC_Billboard.h"

#include "ConsoleCommandServiceInterface.h"
#include "WorldStream.h"
#include "SceneManager.h"
#include "EventManager.h"
#include "ModuleManager.h"
#include "RealXtend/RexProtocolMsgIDs.h"
#include "NetworkMessages/NetInMessage.h"
#include "CoreStringUtils.h"
#include "GenericMessageUtils.h"
#include "UiModule.h"
#include "ConfigurationManager.h"
#include "EC_OpenSimPresence.h"
#include "EC_OpenSimPrim.h"

#include <QColor>
#include "MemoryLeakCheck.h"

namespace Naali
{

const std::string InWorldChatModule::moduleName_ = std::string("InWorldChatModule");

InWorldChatModule::InWorldChatModule() :
    ModuleInterface(moduleName_),
    networkStateEventCategory_(0),
    networkInEventCategory_(0),
    frameworkEventCategory_(0),
    showChatBubbles_(true),
    logging_(false),
    logFile_(0)
    //chatWidget_(0)
{
}

InWorldChatModule::~InWorldChatModule()
{
    SAFE_DELETE(logFile_);
}

void InWorldChatModule::Load()
{
    DECLARE_MODULE_EC(EC_Billboard);
    DECLARE_MODULE_EC(EC_ChatBubble);
}

void InWorldChatModule::PostInitialize()
{
    frameworkEventCategory_ = framework_->GetEventManager()->QueryEventCategory("Framework");

    uiModule_ = framework_->GetModuleManager()->GetModule<UiServices::UiModule>();

    RegisterConsoleCommand(Console::CreateCommand("bbtest",
        "Adds a billboard to each entity in the scene.",
        Console::Bind(this, &InWorldChatModule::TestAddBillboard)));

    RegisterConsoleCommand(Console::CreateCommand("chat",
        "Sends a chat message. Usage: \"chat(message)\"",
        Console::Bind(this, &InWorldChatModule::ConsoleChat)));

    showChatBubbles_ = framework_->GetDefaultConfig().DeclareSetting("InWorldChatModule", "ShowChatBubbles", true);
    logging_ = framework_->GetDefaultConfig().DeclareSetting("InWorldChatModule", "Logging", false);
}

void InWorldChatModule::Update(f64 frametime)
{
}

bool InWorldChatModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface *data)
{
    if (category_id == frameworkEventCategory_)
    {
        if (event_id == Foundation::NETWORKING_REGISTERED)
        {
            ProtocolUtilities::NetworkingRegisteredEvent *event_data = checked_static_cast<ProtocolUtilities::NetworkingRegisteredEvent *>(data);
            if (event_data)
            {
                networkStateEventCategory_ = framework_->GetEventManager()->QueryEventCategory("NetworkState");
                networkInEventCategory_ = framework_->GetEventManager()->QueryEventCategory("NetworkIn");

                return false;
            }
        }

        if(event_id == Foundation::WORLD_STREAM_READY)
        {
            ProtocolUtilities::WorldStreamReadyEvent *event_data = checked_static_cast<ProtocolUtilities::WorldStreamReadyEvent *>(data);
            if (event_data)
                currentWorldStream_ = event_data->WorldStream;

            networkInEventCategory_ = framework_->GetEventManager()->QueryEventCategory("NetworkIn");

            return false;
        }
    }

    if (category_id == networkStateEventCategory_)
    {
        if (event_id == ProtocolUtilities::Events::EVENT_SERVER_CONNECTED)
        {
            // Get settings.
            showChatBubbles_ = framework_->GetDefaultConfig().GetSetting<bool>("InWorldChatModule", "ShowChatBubbles");
            logging_ = framework_->GetDefaultConfig().GetSetting<bool>("InWorldChatModule", "Logging");
        }
        else if (event_id == ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED)
        {
            //SAFE_DELETE(chatWidget_);

            // Save settings.
            framework_->GetDefaultConfig().SetSetting<bool>("InWorldChatModule", "ShowChatBubbles", showChatBubbles_);
            framework_->GetDefaultConfig().SetSetting<bool>("InWorldChatModule", "Logging", logging_);
        }
    }

    if (category_id == networkInEventCategory_)
    {
        if (event_id != RexNetMsgGenericMessage && event_id != RexNetMsgChatFromSimulator)
            return false;

        using namespace ProtocolUtilities;
        NetworkEventInboundData *netdata = checked_static_cast<NetworkEventInboundData *>(data);
        assert(netdata);
        if (!netdata)
            return false;
        ProtocolUtilities::NetInMessage &msg = *netdata->message;

        switch(event_id)
        {
        case RexNetMsgGenericMessage:
            if (ParseGenericMessageMethod(msg) == "RexEmotionIcon")
            {
                StringVector sv = ParseGenericMessageParameters(msg);
                HandleRexEmotionIconMessage(sv);
            }
            return false;
        case RexNetMsgChatFromSimulator:
            HandleChatFromSimulatorMessage(msg);
            return false;
        default:
            return false;
        }
    }

    return false;
}

void InWorldChatModule::SendChatFromViewer(const QString &msg)
{
    if (currentWorldStream_)
        currentWorldStream_->SendChatFromViewerPacket(std::string(msg.toUtf8()));
}

void InWorldChatModule::ShowUserVoipActivityIcon(const RexUUID &id, const bool visibility)
{
    Scene::Entity *entity = GetEntityWithId(id);
    if (!entity)
        return;

    if (!entity->HasComponent("EC_Billboard", "VoipIndicator"))
    {
        Foundation::ComponentPtr component = framework_->GetComponentManager()->CreateComponent("EC_Billboard", "VoipIndicator");
        assert(component.get());
        entity->AddComponent(component);
        entity->GetComponent<EC_Billboard>()->SetDimensions(10, 10);
    }

    boost::shared_ptr<EC_Billboard> billboard = entity->GetComponent<EC_Billboard>("VoipIndicator");
    assert(billboard.get());
    if (visibility)
        billboard->Show("SpeakerIcon.png");
    else
        billboard->Hide();
}

Console::CommandResult InWorldChatModule::TestAddBillboard(const StringVector &params)
{
    Scene::ScenePtr scene = GetFramework()->GetDefaultWorldScene();
    Scene::EntityList prims = scene->GetEntitiesWithComponent(EC_OpenSimPrim::TypeNameStatic());
    Scene::EntityListIterator it = prims.begin();
    while(it != prims.end())
    {
        Scene::EntityPtr entity = *it;
        entity->AddComponent(framework_->GetComponentManager()->CreateComponent("EC_Billboard"));
        EC_Billboard *billboard = entity->GetComponent<EC_Billboard>().get();
        assert(billboard);
        billboard->Show("smoke.png", 4);
        ++it;
    }

    return Console::ResultSuccess();
}

Console::CommandResult InWorldChatModule::ConsoleChat(const StringVector &params)
{
    if (params.size() == 0)
        return Console::ResultFailure("Can't send empty chat message!");

    SendChatFromViewer(params[0].c_str());
    return Console::ResultSuccess();
}

void InWorldChatModule::ApplyDefaultChatBubble(Scene::Entity &entity, const QString &message)
{
    Foundation::ComponentInterfacePtr component = entity.GetOrCreateComponent(EC_ChatBubble::TypeNameStatic());
    assert(component.get());
    EC_ChatBubble &chatBubble = *(checked_static_cast<EC_ChatBubble *>(component.get()));
    chatBubble.ShowMessage(message);
}

void InWorldChatModule::ApplyBillboard(Scene::Entity &entity, const std::string &texture, float timeToShow)
{
    boost::shared_ptr<EC_Billboard> ec_bb = entity.GetComponent<EC_Billboard>();

    // If we didn't have the billboard component yet, create one now.
    if (!ec_bb)
    {
        entity.AddComponent(framework_->GetComponentManager()->CreateComponent("EC_Billboard"));
        ec_bb = entity.GetComponent<EC_Billboard>();
        assert(ec_bb.get());
    }

    ec_bb->Show(texture, timeToShow);
}

Scene::Entity *InWorldChatModule::GetEntityWithId(const RexUUID &id)
{
    Scene::ScenePtr scene = GetFramework()->GetDefaultWorldScene();
    for(Scene::SceneManager::iterator iter = scene->begin(); iter != scene->end(); ++iter)
    {
        Scene::Entity &entity = **iter;

        boost::shared_ptr<EC_OpenSimPresence> ec_presence = entity.GetComponent<EC_OpenSimPresence>();
        boost::shared_ptr<EC_OpenSimPrim> ec_prim = entity.GetComponent<EC_OpenSimPrim>();

        if (ec_presence)
        {
            if (ec_presence->agentId == id)
                return &entity;
        }
        else if (ec_prim)
        {
            if (ec_prim->FullId == id)
                return &entity;
        }
    }

    return 0;
}

void InWorldChatModule::HandleRexEmotionIconMessage(StringVector &params)
{
    // Param 0: avatar UUID
    // Param 1: texture ID
    // Param 2: timeout (remember to replace any , with . before parsing)
    if (params.size() < 3)
        throw Exception("Failed to parse RexEmotionIcon message!");

    LogInfo("Received RexEmotionIcon: " + params[0] + " " + params[1] + " " + params[2]);

    ReplaceCharInplace(params[2], ',', '.');

    if (!RexUUID::IsValid(params[0]))
        throw Exception("Invalid Entity UUID passed in RexEmotionIcon message!");

    RexUUID entityUUID(params[0]);
    if (entityUUID.IsNull())
        throw Exception("Null Entity UUID passed in RexEmotionIcon message!");

    Scene::Entity *entity = GetEntityWithId(entityUUID);
    if (!entity)
        throw Exception("Received RexEmotionIcon message for a nonexisting entity!");

    // timeToShow: value of [-1, 0[ denotes "infinite".
    //             a positive value between [0, 86400] denotes the number of seconds to show. (max roughly one day)
    float timeToShow = atof(params[2].c_str());
    if (!(timeToShow >= -2.f && timeToShow <= 86401.f)) // Checking through negation due to possible NaNs and infs. (being lax and also allowing off-by-one)
        throw Exception("Invalid time-to-show passed in RexEmotionIcon message!");

    if (RexUUID::IsValid(params[1]))
    {
        RexUUID textureID(params[1]);
        // We've been passed a texture UUID to show on the billboard.
        ///\todo request the asset and show that on the billboard.
    }
    else // We've been passed a string URL or a filename on the local computer.
    {
        ///\todo Request a download from that URL and show the resulting image on the billboard.

        // Now assuming that the textureID points to a local file, just to get a proof-of-concept something showing in the UI.
        ApplyBillboard(*entity, params[1], timeToShow);
    }
}

void InWorldChatModule::HandleChatFromSimulatorMessage(ProtocolUtilities::NetInMessage &msg)
{
    msg.ResetReading();

    std::string fromName = msg.ReadString();
    RexUUID sourceId = msg.ReadUUID();

    msg.SkipToFirstVariableByName("Message");
    std::string message = msg.ReadString();
    if (message.size() < 1)
        return;

    if (logging_)
    {
        if (!logFile_)
            CreateLogFile();

        if (logFile_)
        {
            std::stringstream ss;
            ss << "[" << GetLocalTimeString() << "] " << fromName << ": " << message;

            QTextStream out(logFile_);
            out << ss.str().c_str() << "\n";
        }
    }

    if (showChatBubbles_)
    {
        Scene::Entity *entity = GetEntityWithId(sourceId);
        if (entity)
            ApplyDefaultChatBubble(*entity, QString::fromUtf8(message.c_str()));
    }

    // Connect chat ui to this modules ChatReceived
    // emit ChatReceived()
    //if (chatWindow_)
    //    chatWindow_->ChatReceived(name, msg);
}

bool InWorldChatModule::CreateLogFile()
{
    // Create filename. Format: "<server_name>_yyyy_dd_MM_<counter>.log"
    // Use QSettings for getting the application settings home dir
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "chatlogs/");
    QString path = settings.fileName();
    path.replace(".ini", "");
    QDir dir(path);
    if (!dir.exists())
        dir.mkpath(path);

    QString filename = path;
    QString server = currentWorldStream_->GetSimName().c_str();

    // Protection against invalid characters and possible evil filename injections
    server.replace('.', '_');
    server.replace(' ', '_');
    server.replace('\\', '_');
    server.replace('/', '_');
    server.replace(':', '_');
    server.replace('*', '_');
    server.replace('?', '_');
    server.replace('\"', '_');
    server.replace('<', '_');
    server.replace('>', '_');
    server.replace('|', '_');

    filename.append(server);
    filename.append('_');
    filename.append(QDate::currentDate().toString("yyyy-MM-dd"));

    // Create file
    int fileSuffixCounter = 1;
    logFile_ = new QFile;
    while(!logFile_->isOpen() && fileSuffixCounter < 100)
    {
        QString file = filename + "_" + QString("%1.log").arg(fileSuffixCounter++);
        if (!QFile::exists(file))
        {
            SAFE_DELETE(logFile_);
            logFile_ = new QFile(file);
            logFile_->open(QIODevice::WriteOnly | QIODevice::Text);
        }
    }

    if (!logFile_->isOpen())
    {
        LogError("Could not create log file for chat logging.");
        SAFE_DELETE(logFile_);
        return false;
    }

    QTextStream log(logFile_);
    QString entry = tr("Chat log created ") + QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss");
    log << entry << "\n";
    LogDebug(entry.toStdString());
    return true;
}

/*
void InWorldChatModule::RepositionChatWidget(const QRectF &rect)
{
    if (rect.isValid() && chatProxy_ && chatWidget_)
        chatProxy_->setPos(rect.width()-chatWidget_->size().width(), rect.height()-chatWidget_->size().height());
}
*/

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

}

using namespace Naali;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(InWorldChatModule)
POCO_END_MANIFEST
