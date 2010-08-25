/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   RexTypes.h
 *  @brief  RealXtend-spesific typedefs, bit flags, enums etc. and utility functions for them.
 */

#ifndef incl_RexCommon_RexTypes_h
#define incl_RexCommon_RexTypes_h

#include "CoreTypes.h"
#include "Vector3D.h"
#include <boost/cstdint.hpp>
#include <map>
#include <QString>

///\todo Move to CoreTypes.h. Currently (2010/06/01) MumbleVoipModule causes problems with its own stdint.h
using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;
using boost::uint64_t;
using boost::int8_t;
using boost::int16_t;
using boost::int32_t;
using boost::int64_t;

typedef int asset_type_t;
typedef int inventory_type_t;

namespace RexTypes
{
    typedef Vector3D<float> Vector3;
    typedef Vector3D<double> Vector3d;

    ///\todo Is this needed? Only one message uses Vector4 (CameraConstraint).
    struct Vector4
    {
        Vector4() { x = y = z = w = 0.f; }
        float x;
        float y;
        float z;
        float w;
    };

    // Avatar control flags
    const int CONTROL_AT_POS_INDEX = 0;
    const int CONTROL_AT_NEG_INDEX = 1;
    const int CONTROL_LEFT_POS_INDEX = 2;
    const int CONTROL_LEFT_NEG_INDEX = 3;
    const int CONTROL_UP_POS_INDEX = 4;
    const int CONTROL_UP_NEG_INDEX = 5;
    const int CONTROL_PITCH_POS_INDEX = 6;
    const int CONTROL_PITCH_NEG_INDEX = 7;
    const int CONTROL_YAW_POS_INDEX = 8;
    const int CONTROL_YAW_NEG_INDEX = 9;
    const int CONTROL_FAST_AT_INDEX = 10;
    const int CONTROL_FAST_LEFT_INDEX = 11;
    const int CONTROL_FAST_UP_INDEX = 12;
    const int CONTROL_FLY_INDEX = 13;
    const int CONTROL_STOP_INDEX = 14;
    const int CONTROL_FINISH_ANIM_INDEX = 15;
    const int CONTROL_STAND_UP_INDEX = 16;
    const int CONTROL_SIT_ON_GROUND_INDEX = 17;
    const int CONTROL_MOUSELOOK_INDEX = 18;
    const int CONTROL_NUDGE_AT_POS_INDEX = 19;
    const int CONTROL_NUDGE_AT_NEG_INDEX = 20;
    const int CONTROL_NUDGE_LEFT_POS_INDEX = 21;
    const int CONTROL_NUDGE_LEFT_NEG_INDEX = 22;
    const int CONTROL_NUDGE_UP_POS_INDEX = 23;
    const int CONTROL_NUDGE_UP_NEG_INDEX = 24;
    const int CONTROL_TURN_LEFT_INDEX = 25;
    const int CONTROL_TURN_RIGHT_INDEX = 26;
    const int CONTROL_AWAY_INDEX = 27;
    const int CONTROL_LBUTTON_DOWN_INDEX = 28;
    const int CONTROL_LBUTTON_UP_INDEX = 29;
    const int CONTROL_ML_LBUTTON_DOWN_INDEX = 30;
    const int CONTROL_ML_LBUTTON_UP_INDEX = 31;
    const int TOTAL_CONTROLS = 32;

    enum ControlFlags
    {
        /// Empty flag
        NONE = 0,
        /// Move Forward
        AGENT_CONTROL_AT_POS = 0x1 << CONTROL_AT_POS_INDEX,
        /// Move Backward
        AGENT_CONTROL_AT_NEG = 0x1 << CONTROL_AT_NEG_INDEX,
        /// Move Left
        AGENT_CONTROL_LEFT_POS = 0x1 << CONTROL_LEFT_POS_INDEX,
        /// Move Right
        AGENT_CONTROL_LEFT_NEG = 0x1 << CONTROL_LEFT_NEG_INDEX,
        /// Not Flying: Jump/Flying: Move Up
        AGENT_CONTROL_UP_POS = 0x1 << CONTROL_UP_POS_INDEX,
        /// Not Flying: Croutch/Flying: Move Down
        AGENT_CONTROL_UP_NEG = 0x1 << CONTROL_UP_NEG_INDEX,
        /// Unused
        AGENT_CONTROL_PITCH_POS = 0x1 << CONTROL_PITCH_POS_INDEX,
        /// Unused
        AGENT_CONTROL_PITCH_NEG = 0x1 << CONTROL_PITCH_NEG_INDEX,
        /// Unused
        AGENT_CONTROL_YAW_POS = 0x1 << CONTROL_YAW_POS_INDEX,
        /// Unused
        AGENT_CONTROL_YAW_NEG = 0x1 << CONTROL_YAW_NEG_INDEX,
        /// ORed with AGENT_CONTROL_AT_* if the keyboard is being used
        AGENT_CONTROL_FAST_AT = 0x1 << CONTROL_FAST_AT_INDEX,
        /// ORed with AGENT_CONTROL_LEFT_* if the keyboard is being used
        AGENT_CONTROL_FAST_LEFT = 0x1 << CONTROL_FAST_LEFT_INDEX,
        /// ORed with AGENT_CONTROL_UP_* if the keyboard is being used
        AGENT_CONTROL_FAST_UP = 0x1 << CONTROL_FAST_UP_INDEX,
        /// Fly
        AGENT_CONTROL_FLY = 0x1 << CONTROL_FLY_INDEX,
        /// 
        AGENT_CONTROL_STOP = 0x1 << CONTROL_STOP_INDEX,
        /// Finish our current animation
        AGENT_CONTROL_FINISH_ANIM = 0x1 << CONTROL_FINISH_ANIM_INDEX,
        /// Stand up from the ground or a prim seat
        AGENT_CONTROL_STAND_UP = 0x1 << CONTROL_STAND_UP_INDEX,
        /// Sit on the ground at our current location
        AGENT_CONTROL_SIT_ON_GROUND = 0x1 << CONTROL_SIT_ON_GROUND_INDEX,
        /// Whether mouselook is currently enabled
        AGENT_CONTROL_MOUSELOOK = 0x1 << CONTROL_MOUSELOOK_INDEX,
        /// Legacy, used if a key was pressed for less than a certain amount of time
        AGENT_CONTROL_NUDGE_AT_POS = 0x1 << CONTROL_NUDGE_AT_POS_INDEX,
        /// Legacy, used if a key was pressed for less than a certain amount of time
        AGENT_CONTROL_NUDGE_AT_NEG = 0x1 << CONTROL_NUDGE_AT_NEG_INDEX,
        /// Legacy, used if a key was pressed for less than a certain amount of time
        AGENT_CONTROL_NUDGE_LEFT_POS = 0x1 << CONTROL_NUDGE_LEFT_POS_INDEX,
        /// Legacy, used if a key was pressed for less than a certain amount of time
        AGENT_CONTROL_NUDGE_LEFT_NEG = 0x1 << CONTROL_NUDGE_LEFT_NEG_INDEX,
        /// Legacy, used if a key was pressed for less than a certain amount of time
        AGENT_CONTROL_NUDGE_UP_POS = 0x1 << CONTROL_NUDGE_UP_POS_INDEX,
        /// Legacy, used if a key was pressed for less than a certain amount of time
        AGENT_CONTROL_NUDGE_UP_NEG = 0x1 << CONTROL_NUDGE_UP_NEG_INDEX,
        /// 
        AGENT_CONTROL_TURN_LEFT = 0x1 << CONTROL_TURN_LEFT_INDEX,
        /// 
        AGENT_CONTROL_TURN_RIGHT = 0x1 << CONTROL_TURN_RIGHT_INDEX,
        /// Set when the avatar is idled or set to away. Note that the away animation is 
        /// activated separately from setting this flag
        AGENT_CONTROL_AWAY = 0x1 << CONTROL_AWAY_INDEX,
        /// 
        AGENT_CONTROL_LBUTTON_DOWN = 0x1 << CONTROL_LBUTTON_DOWN_INDEX,
        /// 
        AGENT_CONTROL_LBUTTON_UP = 0x1 << CONTROL_LBUTTON_UP_INDEX,
        /// 
        AGENT_CONTROL_ML_LBUTTON_DOWN = 0x1 << CONTROL_ML_LBUTTON_DOWN_INDEX,
        /// 
        AGENT_CONTROL_ML_LBUTTON_UP = 0x1 << CONTROL_ML_LBUTTON_UP_INDEX
    };

    /// File filters with reX file types
    const QString rexFileFilters(
        "Images (*.tga; *.bmp; *.jpg; *.jpeg; *.png);;"
        "Sounds (*.ogg; *.wav);;"
        "Ogre 3D Models (*.mesh);;"
        "Ogre Particle Scripts (*.particle);;"
        "Ogre Skeleton (*.skeleton);;"
        "Ogre Material (*.material);;"
        "Flash Animation (*.swf);;"
        "All Files (*.*)");

    ///\note When adding new asset/inventory types, remember also to add them to the functions.
    // UDP Asset types
    const asset_type_t RexAT_Texture = 0;
    const asset_type_t RexAT_TextureJPEG = 19;
    const asset_type_t RexAT_SoundVorbis = 1;
    const asset_type_t RexAT_SoundWav = 17;
    const asset_type_t RexAT_Mesh = 43;
    const asset_type_t RexAT_Skeleton = 44;
    const asset_type_t RexAT_MaterialScript = 45;
    const asset_type_t RexAT_GenericAvatarXml = 46;
    
    const asset_type_t RexAT_ParticleScript = 47;
    const asset_type_t RexAT_FlashAnimation = 49;
    const asset_type_t RexAT_Image = 50;
    const asset_type_t RexAT_None = -1;

    // UDP Inventory types.
    const inventory_type_t RexIT_Texture = 0;
    const inventory_type_t RexIT_Sound = 1;
    const inventory_type_t RexIT_Object = 6; // Used for meshes.
    const inventory_type_t RexIT_Wearable = 18;
    const inventory_type_t RexIT_Animation = 19; // Used for skeletal animations.
    const inventory_type_t RexIT_OgreScript = 41;
    const inventory_type_t RexIT_FlashAnimation = 42;
    const inventory_type_t RexIT_None = -1;

    // Text asset types
    const std::string ASSETTYPENAME_TEXTURE("Texture");
    const std::string ASSETTYPENAME_MESH("Mesh");
    const std::string ASSETTYPENAME_SKELETON("Skeleton");
    const std::string ASSETTYPENAME_MATERIAL_SCRIPT("MaterialScript");
    const std::string ASSETTYPENAME_PARTICLE_SCRIPT("ParticleScript");
    const std::string ASSETTYPENAME_FLASH_ANIMATION("FlashAnimation");
    const std::string ASSETTYPENAME_GENERIC_AVATAR_XML("GenericAvatarXml");
    const std::string ASSETTYPENAME_IMAGE("Image"); // Avatar asset. No UDP equivalent
    const std::string ASSETTYPENAME_SOUNDVORBIS("SoundVorbis");
    const std::string ASSETTYPENAME_SOUNDWAV("SoundWav");
    const std::string ASSETTYPENAME_UNKNOWN("Unknown");

    // Inventory category names.
    const std::string CATEGORY_AVATAR("Avatar");
    const std::string CATEGORY_TEXTURE("Textures");
    const std::string CATEGORY_SOUND("Sounds");
    const std::string CATEGORY_MESH("3D Models");
    const std::string CATEGORY_SKELETON("Skeletal Animations");
    const std::string CATEGORY_MATERIAL_SCRIPT("Material Scripts");
    const std::string CATEGORY_PARTICLE_SCRIPT("Particle Scripts");
    const std::string CATEGORY_FLASH_ANIMATION("Flash Animations");
    const std::string CATEGORY_UNKNOWN("Unknown");

    // Text asset types used for uploading assets.
    const std::string AT_TEXTURE("texture");
    const std::string AT_SOUND_VORBIS("sound");
    const std::string AT_SOUND_WAV("snd_wav");
    const std::string AT_MESH("ogremesh");
    const std::string AT_SKELETON("ogreskel");
    const std::string AT_MATERIAL_SCRIPT("ogremate");
    const std::string AT_PARTICLE_SCRIPT("ogrepart");
    const std::string AT_FLASH_ANIMATION("flashani");
    const std::string AT_UNKNOWN("unknown");
    const std::string AT_GENERIC_AVATAR_XML("g.avatar"); 

    // Text inventory types used for uploading assets.
    const std::string IT_TEXTURE("texture");
    const std::string IT_SOUND("sound");
    const std::string IT_MESH("object");
    const std::string IT_SKELETON("animation");
    const std::string IT_MATERIAL_SCRIPT("ogrescript");
    const std::string IT_PARTICLE_SCRIPT("ogrescript");
    const std::string IT_FLASH_ANIMATION("flashanim");
    const std::string IT_UNKNOWN("unknown");
    const std::string IT_WEARABLE("wearable");

    // Filters for Open File Name dialog.
    const std::string IMAGE_FILTER("Images (*.tga; *.bmp; *.jpg; *.jpeg; *.png)");
    const std::string SOUND_FILTER("Sounds (*.ogg; *.wav)");
    const std::string MESH_FILTER("Ogre 3D Models (*.mesh)");
    const std::string PARTICLE_FILTER("Ogre Particle Scripts (*.particle)");
    const std::string MESHANIMATION_FILTER("Ogre Skeleton (*.skeleton)");
    const std::string MATERIALSCRIPT_FILTER("Ogre Material (*.material)");
    const std::string FLASHANIMATION_FILTER("Flash Animation (*.swf)");
    const std::string ALLFILES_FILTER("All Files (*.*)");

    // UDP Asset channels
    const int RexAC_Unknown = 0;
    const int RexAC_Miscellaneous = 1;
    const int RexAC_Asset = 2;

    // Asset sources
    const int RexAS_Unknown = 0;
    const int RexAS_Asset = 2;
    const int RexAS_Inventory = 3;

    // Image types
    const int RexIT_Normal = 0;

    // Asset transfer status codes
    const int RexTS_Ok = 0;
    const int RexTS_Done = 1;
    const int RexTS_Skip = 2;
    const int RexTS_Abort = 3;
    const int RexTS_Error = -1;
    const int RexTS_UnknownSource = -2;
    const int RexTS_NoPermission = -3;
    const int RexTS_UnknownError = -4;

    //! Returns ReX/OpenSim asset type from text asset type.
    /*! \param asset_type Asset type name
        \return non-negative asset type, or -1 if unknown
     */
    const asset_type_t GetAssetTypeFromTypeName(const std::string& asset_type);

    //! Returns asset type name from ReX/OpenSim asset type
    /*! \param asset_type Asset type
        \return asset type name
     */
    const std::string &GetTypeNameFromAssetType(asset_type_t asset_type);

    /** Returns inventory type name used for uploading from ReX/OpenSim asset type
     *  \param asset_type Asset type
     *  \return inventory name for asset type (8 characters)
     */
    const std::string &GetInventoryTypeString(asset_type_t asset_type);

    /** Returns inventory type used for ReX/OpenSim inventory actions.
     *  \param asset_type Asset type
     *  \return inventory type for asset type.
     */
    const inventory_type_t &GetInventoryTypeFromAssetType(asset_type_t asset_type);

    /** Returns asset type name used for uploading from ReX/OpenSim asset type
     *  \param asset_type Asset type
     *  \return asset type name for asset type (8 characters)
     */
    const std::string &GetAssetTypeString(asset_type_t asset_type);

    /** Returns Inventory category name used for uploading from ReX/OpenSim asset type.
     *  \param asset_type Asset type.
     *  \return Inventory category name for asset type.
     */
    const std::string &GetCategoryNameForAssetType(asset_type_t asset_type);

    /** Returns asset type name from ReX/OpenSim asset type
     *  \param asset_type Asset type
     *  \return asset type inventory name (8 characters)
     */
    const std::string &GetOpenFileNameFilter(asset_type_t asset_type);

    /** Returns ReX/OpenSim asset type from filename.
     *  @param filename Filename.
     *  @return Non-negative asset type, or -1 if unknown.
     */
    asset_type_t GetAssetTypeFromFilename(const std::string &filename);

    /** Returns file extension for ReX/OpenSim asset type.
     *  @param asset_type Asset type.
     *  @return File extension.
     */
    std::string GetFileExtensionFromAssetType(const asset_type_t &asset_type);

    // Prim drawtypes
    const int DRAWTYPE_PRIM = 0;
    const int DRAWTYPE_MESH = 1;

    // Primitive shapes
    const int SHAPE_CIRCLE = 0;
    const int SHAPE_SQUARE = 1;
    const int SHAPE_ISOMETRIC_TRIANGLE = 2;
    const int SHAPE_EQUILATERAL_TRIANGLE = 3;
    const int SHAPE_RIGHT_TRIANGLE = 4;
    const int SHAPE_HALF_CIRCLE = 5;

    // Primitive hollow shapes
    const int HOLLOW_CIRCLE = 16;
    const int HOLLOW_SQUARE = 32;
    const int HOLLOW_TRIANGLE = 48;

    // Primitive extrusion type
    const int EXTRUSION_STRAIGHT = 16;

    // Primitive texture entry material type
    const uint8_t MATERIALTYPE_BUMP = 0x1f;
    const uint8_t MATERIALTYPE_FULLBRIGHT = 0x20;
    const uint8_t MATERIALTYPE_SHINY = 0xc0;

    // Attached sound flags
    const uint8_t ATTACHED_SOUND_LOOP = 0x01;

    typedef std::string RexAssetID;

    typedef int ControllableType;
    //! invalid controllable type
    const ControllableType CT_INVALID_TYPE = 0;
    //! avatar controllable type
    const ControllableType CT_AVATAR = 1;
    //! camera controllable type
    const ControllableType CT_CAMERA = 2;

    //! List of common actions for controllables
    //! Zero is reserved and odd numbers are implicitly reserved for 'stopped' actions.
    namespace Actions
    {
        namespace
        {
            const int Invalid           = 0;
            const int MoveForward       = 2;
            const int MoveBackward      = 4;
            const int MoveLeft          = 6;
            const int MoveRight         = 8;
            const int RotateLeft        = 10;
            const int RotateRight       = 12;
            const int MoveUp            = 14;
            const int MoveDown          = 16;
            const int RotateUp          = 18;
            const int RotateDown        = 20;
            const int FlyMode           = 22;
            const int Zoom              = 24;
        }
        typedef std::map<event_id_t, int> ActionInputMap;
    }

    //! List of click actions for primitives.
    namespace ClickAction
    {
        //! Touch/grab (default).
        const uint None = 0;
        //! Touch/grab (default).
        const uint Touch = 0;
        //! Sit on object.
        const uint Sit = 1;
        //! Buy object.
        const uint Buy = 2; 
        //! Pay object.
        const uint Pay = 3;
        //! Open.
        const uint Open = 4;
        //! Play media.
        const uint Play = 5;
        //! Open media.
        const uint OpenMedia = 6;
    }

    //! Enumeration of in-world action states
    namespace InWorldActionState
    {
        //! Normal click
        const uint Click = 0;
        //! Camera focus control
        const uint Focus = 1;
        //! Object move/drag
        const uint Move = 2;
        //! Object edit & selection
        const uint Edit = 3; 
        //! Object creation
        const uint Create = 4;
        //! Land modification
        const uint Land = 5;
    }

    //! \return true if asset id is null (empty).
    bool IsNull(const RexAssetID& id);

    //! \return true if asset id is non-null (not empty) and can not be construed as an UUID
    bool IsUrlBased(const RexAssetID& id);

} // end of namespace: RexTypes

#endif
