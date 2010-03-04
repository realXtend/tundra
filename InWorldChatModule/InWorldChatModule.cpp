/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   InWorldChatModule.cpp
 *  @brief  Simple OpenSim world chat module. Listens for ChatFromSimulator packets and shows the chat on the UI.
 *          Outgoing chat sent using ChatFromViewer packets. Manages EC_ChatBubbles
 */

#include "StableHeaders.h"
#include "InWorldChatModule.h"

#include <EC_ChatBubble.h>
#include <EventManager.h>
#include <ModuleManager.h>
#include <RealXtend/RexProtocolMsgIDs.h>
#include <NetworkMessages/NetInMessage.h>
#include <CoreStringUtils.h>
#include <UiModule.h>


namespace
{
/// Test code for getting pixmap with chat bubble image and text
/// @param filename Filename for the image used as the basis for the chat bubble.
/// @param text Text to be shown in the chat bubble.
/// @return the rendered pixmap with image and embedded text or null if something goes wrong.
QPixmap GetPixmap(const QString &image_name, const QString &text)
{
    int wanted_width = 400;
    int wanted_height = 200;
    QRect wanted_rect(0,0,wanted_width,wanted_height);

    if (!QFile::exists(image_name))
        return 0;

    QPixmap pixmap;
    pixmap.load(image_name);
    pixmap = pixmap.scaled(wanted_rect.size());

    QPainter painter(&pixmap);

    // Draw rounded rect
/*
    QRectF rectangle(10.0, 20.0, 80.0, 60.0);
    painter.setBrush(Qt::SolidPattern);
    painter.drawRoundedRect(rectangle, 20.0, 15.0);
*/

    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 24));
    // draw text
    painter.drawText(wanted_rect, Qt::AlignCenter | Qt::TextWordWrap, text);

    return pixmap;
}

}

namespace Naali
{

InWorldChatModule::InWorldChatModule() :
    ModuleInterfaceImpl(NameStatic()),
    networkStateEventCategory_(0),
    networkInEventCategory_(0),
    frameworkEventCategory_(0)
{
}

InWorldChatModule::~InWorldChatModule()
{
}

void InWorldChatModule::Load()
{
    DECLARE_MODULE_EC(EC_ChatBubble);

    LogInfo(Name() + " loaded.");
}

void InWorldChatModule::Unload()
{
    LogInfo(Name() + " unloaded.");
}

void InWorldChatModule::Initialize()
{
    LogInfo(Name() + " initialized.");
}

void InWorldChatModule::PostInitialize()
{
    frameworkEventCategory_ = framework_->GetEventManager()->QueryEventCategory("Framework");
    if (frameworkEventCategory_ == 0)
        LogError("Failed to query \"Framework\" event category");

    uiModule_ = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices);
}

void InWorldChatModule::Uninitialize()
{
    LogInfo(Name() + " uninitialized.");
}

void InWorldChatModule::Update(f64 frametime)
{
}

bool InWorldChatModule::HandleEvent(
    event_category_id_t category_id,
    event_id_t event_id,
    Foundation::EventDataInterface *data)
{
    if (category_id == frameworkEventCategory_)
    {
        if (event_id == Foundation::NETWORKING_REGISTERED)
        {
            ProtocolUtilities::NetworkingRegisteredEvent *event_data = dynamic_cast<ProtocolUtilities::NetworkingRegisteredEvent *>(data);
            if (event_data)
                SubscribeToNetworkEvents(event_data->currentProtocolModule);
            return false;
        }

        if(event_id == Foundation::WORLD_STREAM_READY)
        {
            ProtocolUtilities::WorldStreamReadyEvent *event_data = dynamic_cast<ProtocolUtilities::WorldStreamReadyEvent *>(data);
            if (event_data)
                currentWorldStream_ = event_data->WorldStream;

            //if (chatWindow_)
            //    chatWindow_->SetWorldStreamPtr(current_world_stream_);

            networkInEventCategory_ = framework_->GetEventManager()->QueryEventCategory("NetworkIn");
            if (networkInEventCategory_ == 0)
                LogError("Failed to query \"NetworkIn\" event category");

            return false;
        }
    }

    if (category_id == networkStateEventCategory_)
    {
        // Connected to server. Create chat UI.
        if (event_id == ProtocolUtilities::Events::EVENT_SERVER_CONNECTED)
        {
            if (!uiModule_.expired())
            {
                //uiModule_.lock()->
            }
        }
        // Disconnected from server. Delete chat UI.
        if (event_id == ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED)
        {
            if (!uiModule_.expired())
            {
                //uiModule_.lock()->
            }
        }
    }

    if (category_id == networkInEventCategory_)
    {
        if(event_id == RexNetMsgChatFromSimulator)
        {
            ProtocolUtilities::NetworkEventInboundData *netdata = checked_static_cast<ProtocolUtilities::NetworkEventInboundData *>(data);
            assert(netdata);
            if (!netdata)
                return false;

            std::stringstream ss;

            ProtocolUtilities::NetInMessage &msg = *netdata->message;
            msg.ResetReading();

            std::string name = msg.ReadString();
            msg.SkipToFirstVariableByName("Message");
            std::string message = msg.ReadString();
            if (message.size() < 1)
                return false;

            ss << "[" << GetLocalTimeString() << "] " << name << ": " << message << std::endl;

            LogInfo(ss.str());

            QPixmap pixmap = GetPixmap("./media/textures/ChatBubble.png", message.c_str());
            if (!pixmap)
                return false;

            QLabel *w = new QLabel;
            w->setPixmap(pixmap);
            w->setAttribute(Qt::WA_DeleteOnClose);
            w->show();

            // Connect chat ui to this modules ChatReceived
            // emit ChatReceived()
            //if (chatWindow_)
            //    chatWindow_->CharReceived(name, msg);

            return false;
        }
    }

    return false;
}

void InWorldChatModule::SubscribeToNetworkEvents(ProtocolUtilities::ProtocolWeakPtr currentProtocolModule)
{
    networkStateEventCategory_ = framework_->GetEventManager()->QueryEventCategory("NetworkState");
    if (networkStateEventCategory_ == 0)
        LogError("Failed to query \"NetworkState\" event category");

    networkInEventCategory_ = framework_->GetEventManager()->QueryEventCategory("NetworkIn");
    if (networkInEventCategory_ == 0)
        LogError("Failed to query \"NetworkIn\" event category");
}

const std::string InWorldChatModule::moduleName = std::string("InWorldChatModule");

const std::string &InWorldChatModule::NameStatic()
{
    return moduleName;
}

void InWorldChatModule::SendChatFromViewer(const QString &msg)
{
    if (currentWorldStream_)
        currentWorldStream_->SendChatFromViewerPacket(msg.toStdString());
}

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
