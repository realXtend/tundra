// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <QDomDocument>

/// Holds data that is transferred server->client immediately after the client has successfully connected to the server.
/** The contents can hold arbitrary data that is to be passed to the client to read. (max 64K bytes).
    Native clients will get the responseData XML encoded as a string, while web clients get responseDataJson encoded to JSON.
    This is to be unified in the future with both kinds of clients using JSON. */
struct UserConnectedResponseData
{
    QDomDocument responseData;
    QVariantMap responseDataJson;
};
Q_DECLARE_METATYPE(UserConnectedResponseData*)
