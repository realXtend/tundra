// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

class Framework;

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
    class MumbleClient;
    class Channel;
    class User;
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
