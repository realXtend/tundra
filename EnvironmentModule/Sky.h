// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogicModule_Sky_h
#define incl_RexLogicModule_Sky_h

#include "EC_OgreSky.h"
#include "EnvironmentModuleApi.h"
#include "EnvironmentModule.h"
#include "EC_SkyPlane.h"
#include "EC_SkyBox.h"
#include "EC_SkyDome.h"
#include "SceneManager.h"
#include "Entity.h"

#include <QObject>

namespace Resource
{
    namespace Events
    {
        class ResourceReady;
    }
}

namespace ProtocolUtilities
{
    class NetworkEventInboundData;
}

namespace Environment
{
    class EnvironmentModule;

  
    /// Sky component
    /// \ingroup EnvironmentModuleClient.
    class ENVIRONMENT_MODULE_API Sky : public QObject
    {
        Q_OBJECT

    public:
        Sky(EnvironmentModule *owner);
        virtual ~Sky();

        void Update();

        /// Handler for the "RexSky" generic message.
        /// @param data Event data pointer.
        bool HandleRexGM_RexSky(ProtocolUtilities::NetworkEventInboundData* data);

        /** 
        *  Update sky info.
         * @param type Type of the sky: box, dome, plane or none. 
         * @param images List of image uuid's for the sky.
         * @param curvature If sky type is dome, curvature of the dome.
         * @param tiling If sky type is dome, tiling of the dome.
         */
        void UpdateSky(const SkyType &type, std::vector<std::string> images,
            const float &curvature, const float &tiling);

        /// Creates the default sky.
        /// @param show Visibility of the sky.
        void CreateDefaultSky(const bool &show = true);

     
        /// Called whenever a texture is loaded so it can be attached to the sky.
        /// @param Resource pointer to the texture.
        void OnTextureReadyEvent(Resource::Events::ResourceReady *tex);

        /// @return The sky type that is in use.
        SkyType GetSkyType() const;

        /// Disable currently active sky.
        void DisableSky();

        /// Return that if sky is enabled.
        bool IsSkyEnabled();

        /// Enable/Disable sky.
        void EnableSky(bool enabled);

        /// Request sky type chaged. If requested sky type is same as currently used sky do nothing.
        void ChangeSkyType(SkyType type, bool update_sky = true);

        /// Returns current used sky. Returns zero pointer if given parameter sky does not exist.
        template <typename T> T* GetEnviromentSky();
     

    signals:
        /// Signal is emited when sky is enabled/disabled.
        void SkyEnabled(bool enabled);

        /// Signal is emited when sky type has changed.
        void SkyTypeChanged();

    private:
        
        Sky(const Sky &);
        void operator =(const Sky &);

        EnvironmentModule *owner_;

        /// Can be used to check that does given sky exist.
        template <typename T> bool ExistSky(); 
        /// Creates given sky.
        template <typename T> void CreateSky();
        /// Removes given sky type. 
        template <typename T> void RemoveSky();
       

        /// The type of the sky (none, box, dome or plane).
        SkyType type_;

        /// Whether sky is enabled by this component.
        bool skyEnabled_;

        QList<request_tag_t > lstRequestTags_;
        QMap<int, int >  requestMap_;
    };
}

#include "Sky-templates.h"

#endif
