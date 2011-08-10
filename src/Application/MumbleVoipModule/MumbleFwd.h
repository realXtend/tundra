// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include <boost/shared_ptr.hpp>

class Framework;

namespace TundraLogic
{
    class TundraLogicModule;
}

namespace MumbleLib
{
    class User;
    class Connection;
    class MumbleClient;
    class Channel;
    class MumbleLibrary;
    class MumbleMainLoopThread;
}

namespace MumbleClient
{
    class Channel;
}

namespace MumbleVoip
{
    class Participant;
    class Settings;
    class Session;
    class Provider;

    class ServerInfo;
    class PCMAudioFrame;
    class SettingsWidget;
}

class QSignalMapper;
class QNetworkReply;
class QNetworkAccessManager;

struct CELTMode;
struct CELTEncoder;
struct CELTDecoder;
