#pragma once

// Login
const unsigned long cLoginMessage = 100;
const unsigned long cLoginReplyMessage = 101;
const unsigned long cClientJoinedMessage = 102;
const unsigned long cClientLeftMessage = 103;

// Scenesync
const unsigned long cCreateEntityMessage = 110;
const unsigned long cCreateComponentsMessage = 111;
const unsigned long cCreateAttributesMessage = 112;
const unsigned long cEditAttributesMessage = 113;
const unsigned long cRemoveAttributesMessage = 114;
const unsigned long cRemoveComponentsMessage = 115;
const unsigned long cRemoveEntityMessage = 116;
const unsigned long cCreateEntityMessageConvID = 117;
const unsigned long cCreateComponentsMessageConvID = 118;
const unsigned long cCreateAttributesMessageConvID = 119;

// Entity action
const unsigned long cEntityActionMessage = 120;

// Assets
const unsigned long cAssetDiscoveryMessage = 121;
const unsigned long cAssetDeletedMessage = 122;
