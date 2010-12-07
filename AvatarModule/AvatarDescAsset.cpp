#include "StableHeaders.h"
#include "AvatarDescAsset.h"

#include <QDomDocument>

bool AvatarDescAsset::LoadFromFileInMemory(const u8 *data, size_t numBytes)
{
    // Just store the raw downloaded Avatar Appearance XML data for now..    
    QByteArray bytes((const char *)data, numBytes);
    avatarAppearanceXML = QString(bytes);

    // .. but do parse the asset dependencies immediately.

    return true;
}

void AvatarDescAsset::ParseReferences()
{
//    QDomDocument avatar_doc("Avatar");
//    avatar_doc.setContent(avatarAppearanceXML);

    ///\todo Implement.
}
