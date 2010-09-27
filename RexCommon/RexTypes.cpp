/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   RexTypes.h
 *  @brief  RealXtend-spesific typedefs, bit flags, enums etc. and utility functions for them.
 */

#include "RexTypes.h"
#include "RexUUID.h"

#include <boost/algorithm/string.hpp>

namespace RexTypes
{

asset_type_t GetAssetTypeFromTypeName(const std::string& asset_type)
{
    if (asset_type == ASSETTYPENAME_TEXTURE)
        return RexAT_Texture;
    if (asset_type == ASSETTYPENAME_IMAGE)
        return RexAT_Image;
    if (asset_type == ASSETTYPENAME_SOUNDVORBIS)
        return RexAT_SoundVorbis;
    if (asset_type == ASSETTYPENAME_SOUNDWAV)
        return RexAT_SoundWav;
    if (asset_type == ASSETTYPENAME_MESH)
        return RexAT_Mesh;
    if (asset_type == ASSETTYPENAME_SKELETON)
        return RexAT_Skeleton;
    if (asset_type == ASSETTYPENAME_PARTICLE_SCRIPT)
        return RexAT_ParticleScript;
    if (asset_type == ASSETTYPENAME_MATERIAL_SCRIPT)
        return RexAT_MaterialScript;
    if (asset_type == ASSETTYPENAME_FLASH_ANIMATION)
        return RexAT_FlashAnimation;
    if (asset_type == ASSETTYPENAME_GENERIC_AVATAR_XML)
        return RexAT_GenericAvatarXml;

    return RexAT_None;
}

const std::string& GetTypeNameFromAssetType(asset_type_t asset_type)
{
    if (asset_type == RexAT_Texture || asset_type == RexAT_TextureJPEG)
        return ASSETTYPENAME_TEXTURE;
    if (asset_type == RexAT_SoundVorbis)
        return ASSETTYPENAME_SOUNDVORBIS;
    if (asset_type == RexAT_SoundWav)
        return ASSETTYPENAME_SOUNDWAV;
    if (asset_type == RexAT_Mesh)
        return ASSETTYPENAME_MESH;
    if (asset_type == RexAT_Skeleton)
        return ASSETTYPENAME_SKELETON;
    if (asset_type == RexAT_ParticleScript)
        return ASSETTYPENAME_PARTICLE_SCRIPT;
    if (asset_type == RexAT_MaterialScript)
        return ASSETTYPENAME_MATERIAL_SCRIPT;
    if (asset_type == RexAT_GenericAvatarXml)
        return ASSETTYPENAME_GENERIC_AVATAR_XML;
    if (asset_type == RexAT_Image)
        return ASSETTYPENAME_IMAGE;

    return ASSETTYPENAME_UNKNOWN;
}

const std::string& GetInventoryTypeString(asset_type_t asset_type)
{
    if (asset_type == RexAT_Texture || asset_type == RexAT_TextureJPEG)
        return IT_TEXTURE;
    if (asset_type == RexAT_SoundVorbis ||
        asset_type == RexAT_SoundWav)
        return IT_SOUND;
    if (asset_type == RexAT_Mesh)
        return IT_MESH;
    if (asset_type == RexAT_Skeleton)
        return IT_SKELETON;
    if (asset_type == RexAT_MaterialScript)
        return IT_MATERIAL_SCRIPT;
    if (asset_type == RexAT_ParticleScript)
        return IT_PARTICLE_SCRIPT;
    if (asset_type == RexAT_FlashAnimation)
        return IT_FLASH_ANIMATION;
    if (asset_type == RexAT_GenericAvatarXml)
        return IT_WEARABLE;

    return IT_UNKNOWN;
}

const inventory_type_t &GetInventoryTypeFromAssetType(asset_type_t asset_type)
{
    if (asset_type == RexAT_Texture)
        return RexIT_Texture;
    if (asset_type == RexAT_SoundVorbis ||
        asset_type == RexAT_SoundWav)
        return RexIT_Sound;
    if (asset_type == RexAT_Mesh)
        return RexIT_Object;
    if (asset_type == RexAT_Skeleton)
        return RexIT_Animation;
    if (asset_type == RexAT_MaterialScript ||
        asset_type == RexAT_ParticleScript)
        return RexIT_OgreScript;
    if (asset_type == RexAT_FlashAnimation)
        return RexIT_FlashAnimation;
    if (asset_type == RexAT_GenericAvatarXml)
        return RexIT_Wearable;

    return RexIT_None;
}

const std::string& GetAssetTypeString(asset_type_t asset_type)
{
    if (asset_type == RexAT_Texture)
        return AT_TEXTURE;
    if (asset_type == RexAT_SoundVorbis)
        return AT_SOUND_VORBIS;
    if (asset_type == RexAT_SoundWav)
        return AT_SOUND_WAV;
    if (asset_type == RexAT_Mesh)
        return AT_MESH;
    if (asset_type == RexAT_Skeleton)
        return AT_SKELETON;
    if (asset_type == RexAT_MaterialScript)
        return AT_MATERIAL_SCRIPT;
    if (asset_type == RexAT_ParticleScript)
        return AT_PARTICLE_SCRIPT;
    if (asset_type == RexAT_FlashAnimation)
        return AT_FLASH_ANIMATION;
    if (asset_type == RexAT_GenericAvatarXml)
        return AT_GENERIC_AVATAR_XML;

    return AT_UNKNOWN;
}

const std::string& GetCategoryNameForAssetType(asset_type_t asset_type)
{
    if (asset_type == RexAT_Texture || asset_type == RexAT_TextureJPEG)
        return CATEGORY_TEXTURE;
    if (asset_type == RexAT_SoundVorbis ||
        asset_type == RexAT_SoundWav)
        return CATEGORY_SOUND;
    if (asset_type == RexAT_Mesh)
        return CATEGORY_MESH;
    if (asset_type == RexAT_Skeleton)
        return CATEGORY_SKELETON;
    if (asset_type == RexAT_MaterialScript)
        return CATEGORY_MATERIAL_SCRIPT;
    if (asset_type == RexAT_ParticleScript)
        return CATEGORY_PARTICLE_SCRIPT;
    if (asset_type == RexAT_FlashAnimation)
        return CATEGORY_FLASH_ANIMATION;
    if (asset_type == RexAT_GenericAvatarXml)
        return CATEGORY_AVATAR;

    return CATEGORY_UNKNOWN;
}

const std::string& GetOpenFileNameFilter(asset_type_t asset_type)
{
    if (asset_type == RexAT_Texture || asset_type == RexAT_TextureJPEG)
        return IMAGE_FILTER;
    if (asset_type == RexAT_SoundVorbis ||
        asset_type == RexAT_SoundWav)
        return SOUND_FILTER;
    if (asset_type == RexAT_Mesh)
        return MESH_FILTER;
    if (asset_type == RexAT_Skeleton)
        return MESHANIMATION_FILTER;
    if (asset_type == RexAT_MaterialScript)
        return MATERIALSCRIPT_FILTER;
    if (asset_type == RexAT_ParticleScript)
        return PARTICLE_FILTER;
    if (asset_type == RexAT_FlashAnimation)
        return FLASHANIMATION_FILTER;

    return ALLFILES_FILTER;
}

asset_type_t GetAssetTypeFromFilename(const std::string &filename)
{
    size_t pos = filename.find_last_of('.');
    std::string file_ext = filename.substr(pos + 1);
    boost::to_lower(file_ext);

    if (file_ext == "tga" ||
        file_ext == "bmp" ||
        file_ext == "jpg" ||
        file_ext == "jpeg" ||
        file_ext == "png")
        return RexAT_Texture;
    if (file_ext == "ogg")
        return RexAT_SoundVorbis;
    if (file_ext == "wav")
        return RexAT_SoundWav;
    if (file_ext == "mesh")
        return RexAT_Mesh;
    if (file_ext == "skeleton")
        return RexAT_Skeleton;
    if (file_ext == "particle")
        return RexAT_ParticleScript;
    if (file_ext == "material")
        return RexAT_MaterialScript;
    if (file_ext == "swf")
        return RexAT_FlashAnimation;
    if (file_ext == "xml")
        return RexAT_GenericAvatarXml;

    return RexAT_None;
}

std::string GetFileExtensionFromAssetType(const asset_type_t &asset_type)
{
    switch(asset_type)
    {
    case RexAT_Texture:
        return ".png";
    case RexAT_TextureJPEG:
        return ".jpeg";
    case RexAT_SoundVorbis:
        return ".ogg";
    case RexAT_SoundWav:
        return ".wav";
    case RexAT_Mesh:
        return ".mesh";
    case RexAT_Skeleton:
        return ".skeleton";
    case RexAT_ParticleScript:
        return ".particle";
    case RexAT_MaterialScript:
        return ".material";
    case RexAT_FlashAnimation:
        return ".swf";
    case RexAT_GenericAvatarXml:
        return ".xml";
    case RexAT_None:
    default:
        return ".unknown";
    }
}

bool IsNull(const RexAssetID& id)
{
    if (id.size() == 0)
        return true;

    if (RexUUID::IsValid(id) && RexUUID(id).IsNull())
        return true;

    return false;
}

bool IsUrlBased(const RexAssetID& id)
{
    if (id.size() == 0)
        return false;
    if (RexUUID::IsValid(id))
        return false;
    return true;
}

}
