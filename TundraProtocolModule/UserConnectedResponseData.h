#pragma once

#include <QDomDocument>

/// This structure holds data that is transferred server->client immediately after the client has successfully connected to the server.
/// The contents can hold arbitrary data that is to be passed to the client to read. (max 64K bytes).
struct UserConnectedResponseData
{
    QDomDocument responseData;
};
