// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

// Login
const unsigned long cLoginMessage = 100;
const unsigned long cLoginReplyMessage = 101;
const unsigned long cClientJoinedMessage = 102;
const unsigned long cClientLeftMessage = 103;

// Camera
const unsigned long cCameraOrientationUpdate = 104;
const unsigned long cCameraOrientationRequest = 105;

// Scenesync
// IMPORTANT! At least two numbers from the range [106, 108] must be reserved for scene creation and removal messages!
//const unsigned long cCreateSceneMessage = 107;
//const unsigned long cRemoveSceneMessage = 108;
const unsigned long cEditEntityPropertiesMessage = 109;
const unsigned long cCreateEntityMessage = 110;
const unsigned long cCreateComponentsMessage = 111;
const unsigned long cCreateAttributesMessage = 112;
const unsigned long cEditAttributesMessage = 113;
const unsigned long cRemoveAttributesMessage = 114;
const unsigned long cRemoveComponentsMessage = 115;
const unsigned long cRemoveEntityMessage = 116;
const unsigned long cCreateEntityReplyMessage = 117; // Server->client only
const unsigned long cCreateComponentsReplyMessage = 118; // Server->client only
const unsigned long cRigidBodyUpdateMessage = 119;

// Entity action
const unsigned long cEntityActionMessage = 120;

// Assets
const unsigned long cAssetDiscoveryMessage = 121;
const unsigned long cAssetDeletedMessage = 122;

// Custom component types
const unsigned long cRegisterComponentTypeMessage = 123;

// Entity parenting
const unsigned long cSetEntityParentMessage = 124;

// In case of network message structs are regenerated and descriptions get deleted., saving their descriptions here.
// MsgAssetDeleted: Network message informing that asset has been deleted from storage.
// MsgAssetDiscovery: Network message informing that new asset has been discovered in storage.
// MsgClientJoined: Network message informing that client has joined the server.
// MsgClientLeft: Network message informing that client has left the server.
// MsgEntityAction: Network message for entity-action replication.
// MsgLogin: Network message for login request.
// MsgLoginReply: Network message for login reply.
