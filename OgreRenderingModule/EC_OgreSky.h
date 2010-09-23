// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_OgreSky_h
#define incl_OgreRenderer_EC_OgreSky_h

#include "IComponent.h"
#include "RexUUID.h"
#include "OgreModuleApi.h"
#include "Declare_EC.h"

namespace OgreRenderer
{

    /// Sky type
    //! \ingroup EnvironmentModuleClient.
    enum SkyType
    {
        SKYTYPE_NONE = 0,
        SKYTYPE_BOX,
        SKYTYPE_DOME,
        SKYTYPE_PLANE,
    };

    /// Enumeration of the skybox texture indexes.
    enum SkyBoxIndex
    {
        SKYBOX_IND_FRONT = 0,
        SKYBOX_IND_BACK,
        SKYBOX_IND_LEFT,
        SKYBOX_IND_RIGHT,
        SKYBOX_IND_TOP,
        SKYBOX_IND_BOTTOM,
        SKYBOX_IND_COUNT
    };

    /// Generic ogre sky parameters, see Ogre documentation for more info.
    /*struct SkyParameters
    {
        std::string material;
        float distance;
        bool drawFirst;
        float angle;
        RexTypes::Vector3 angleAxis;
        
        SkyParameters() : 
        material("Rex/skybox"),
        distance(50),
        drawFirst(true),
        angle(90),
        angleAxis(1, 0, 0) {}

        void Reset()
        {
            material= "";
            distance = 0;
            drawFirst = true;
            angle = 90;
            angleAxis = RexTypes::Vector3(1,0,0);
        }
    };*/

    //! Sky parameters for skybox, see ogre documentation for more information.
    //! \ingroup EnvironmentModuleClient.
    struct SkyBoxParameters
    {
        std::string material;
        float distance;
        float angle;
        bool drawFirst;
        RexTypes::Vector3 angleAxis;

        SkyBoxParameters() : 
        material("Rex/skybox"),
        distance(50),
        drawFirst(true),
        angle(90),
        angleAxis(1, 0, 0){}

        void Reset()
        {
            material= "";
            distance = 0;
            drawFirst = true;
            angle = 90;
            angleAxis = RexTypes::Vector3(1,0,0);
        }
    };

    /// Sky parameters for skydome, see Ogre documentation for more information.
    //! \ingroup EnvironmentModuleClient.
    struct SkyDomeParameters
    {
        std::string material;
        float curvature;
        float tiling;
        float distance;
        int xSegments;
        int ySegments;
        int ySegmentsKeep;
        bool drawFirst;
        float angle;
        RexTypes::Vector3 angleAxis;

        SkyDomeParameters() :
        //Sky dome and plane are sharing same material cause they both only need single sky texture.
        material("Rex/SkyPlane"),
        curvature(10.f),
        tiling(8.f),
        distance(50.f),
        xSegments(16),
        ySegments(16),
        ySegmentsKeep(-1),
        drawFirst(true),
        angle(90),
        angleAxis(1,0,0){}

        void Reset()
        {
            material= "";
            curvature = 0;
            tiling = 0;
            distance = 0;
            xSegments = 0;
            ySegments = 0;
            ySegmentsKeep = 0;
            drawFirst = true;
            angle = 0;
            angleAxis = RexTypes::Vector3(1,0,0);
        }
    };

    /// Sky parameters for skyplane, see Ogre documentation for more infomation.
    //! \ingroup EnvironmentModuleClient.
    struct SkyPlaneParameters
    {
        std::string material;
        float scale;
        float tiling;
        float bow;
        float distance;
        int xSegments; 
        int ySegments;
        bool drawFirst;
        
        SkyPlaneParameters() :
        material("Rex/SkyPlane"),
        scale(300.f),
        tiling(150.f),
        bow(1.5f),
        distance(50.f),
        xSegments(150),
        ySegments(150),
        drawFirst(true){}

        void Reset()
        {
            material= "";
            scale = 0;
            tiling = 0;
            distance = 0;
            bow = 0;
            xSegments = 0;
            ySegments = 0;
            drawFirst = true;
        }
    };

    class Renderer;
    typedef boost::shared_ptr<Renderer> RendererPtr;
    typedef boost::weak_ptr<Renderer> RendererWeakPtr;

	/**
<table class="header">
<tr>
<td>
<h2>OgreSky</h2>
		

Registered by OgreRenderer::OgreRenderingModule.

<b>No Attributes</b>.

<b>Exposes the following scriptable functions:</b>
<ul>
<li>...
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>Doesn't depend on any components</b>.
</table>
*/
    /** Ogre sky component
     *
     *  \ingroup OgreRenderingModuleClient EnvironmentModuleClient.
     *
     */
    class OGRE_MODULE_API EC_OgreSky : public IComponent
    {
        Q_OBJECT
        
        DECLARE_EC(EC_OgreSky);
    public:
        virtual ~EC_OgreSky();

        /// Creates the default sky. The sky type (box, dome, plane) is read from a config file.
        /// @param show Is the sky shown.
        void CreateSky(bool show = true);

        //! Disables sky methods
        void DisableSky();
           
        /** Enables a skybox.
         *  Other sky methods will be disabled
         *  @param material_name material name
         *  @param distance distance of skybox from camera
         *  @return true if successful
         *  @todo use material/texture asset reference when asset system exists
         */
        //bool SetSkyBox(const std::string& material_name, float distance);

        /// @return material texture string array.
        StringVector GetMaterialTextureNames();

        /// Sets the sky material.
        /// @param material_name Material name.
        //void SetSkyMaterial(const std::string& material_name) { genericSkyParameters.material = material_name; }
        
        /// Sets new textures for SkyBox.
        /// @param index Index of the texture.
        /// @param texture name Texture name.
        void SetSkyBoxMaterialTexture(int index, const char *texture_name, size_t image_count);
        
        /// Sets the texture for SkyDome.
        /// @param texture name Texture name.
        void SetSkyDomeMaterialTexture(const char *texture_name);
        
        /// Sets the texture for SkyPlane.
        /// @param texture name Texture name.
        void SetSkyPlaneMaterialTexture(const char *texture_name);

        /// Set new sky dome parameters.
        void SetSkyDomeParameters(const SkyDomeParameters &params, bool update_sky = true);

        /// Set new sky plane parameters.
        void SetSkyPlaneParameters(const SkyPlaneParameters &params, bool update_sky = true);

        /// Set new sky generic parameters.
        void SetSkyBoxParameters(const SkyBoxParameters &params, bool update_sky = true);

        /// Get sky dome material texture name as string switch is same as assetID.
        /// @return Texture name.
        RexTypes::RexAssetID GetSkyDomeTextureID() const;

        /// Get sky dome material texture name as string switch is same as assetID.
        /// @return Texture name.
        RexTypes::RexAssetID GetSkyPlaneTextureID() const;

        /// Get sky dome material texture name as string switch is same as assetID.
        /// @param sky_box texture index, where(0 = front, 1 = back, 2 = left, 3 = right, 4 = up, 5 = down).
        /// @return Texture name.
        RexTypes::RexAssetID GetSkyBoxTextureID(uint texuture_index) const;

        /// Return sky generic sky parameters mostly used by sky box.
        SkyBoxParameters GetBoxSkyParameters() const;

        /// Returns sky dome parameters like tiling, curvature and so on.
        SkyDomeParameters GetSkyDomeParameters() const;

        /// Returns sky plane parameters like tiling and bow.
        SkyPlaneParameters GetSkyPlaneParameters() const;

        /// Reads the sky parameters from the configuration file.
        void GetSkyConfig();
        
        /// Has sky enabled.
        bool IsSkyEnabled() const;

        /// @Return sky type that is in use.
        SkyType GetSkyType() const;

        /// Change sky type into new one and create new sky if wanted.
        /// @param sky type (Skybox, skydome and skyplane).
        void SetSkyType(SkyType type, bool update_sky = true);

    private:
        /// Constructor
        /// @param module module pointer.
        EC_OgreSky(IModule* module);
        
        /// Renderer
        RendererWeakPtr renderer_;
        
        /// whether sky enabled by this component
        bool skyEnabled_;
        
        /// Type of the sky.
        SkyType type_;

        /// UUID's of the texture assets the skybox uses for rendering. Should be stored per-scene.
    //    RexUUID skyboxTexturesIds_[SKYBOX_IND_COUNT];
        
        /// Generic sky parameters common to all sky types.
        //SkyParameters genericSkyParameters;

        /// Parameters for skybox.
        SkyBoxParameters skyBoxParameters;

        /// Parameters for skydome
        SkyDomeParameters skyDomeParameters;
       
        /// Parameters for skyplane.
        SkyPlaneParameters skyPlaneParameters;
        
        /// List of skybox image names.
        std::vector<std::string> skyBoxImages_;
        
        IModule* owner_; 
        
        size_t currentSkyBoxImageCount_;
    };
}

#endif
