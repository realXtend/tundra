// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_OgreSky_h
#define incl_OgreRenderer_EC_OgreSky_h

#include "Foundation.h"
#include "ComponentInterface.h"
#include "RexUUID.h"
#include "OgreModuleApi.h"

namespace OgreRenderer
{
    /// Sky type
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

    struct SkyImageData
    {
        /// Sky type for which the image is for
        SkyType type;

        /// SkyBox texture index.
        SkyBoxIndex index;
        
        /// Curvature of SkyDome.
        float curvature;
        
        /// Tiling of SkyDome.
        float tiling;
    };

    /// Generic ogre sky parameters, see Ogre documentation for more info.
    struct SkyParameters
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
    };

    /// Sky parameters for skydome, see Ogre documentation for more info.
    struct SkyDomeParameters
    {
        float curvature;
        float tiling;
        int xSegments;
        int ySegments;
        int ySegmentsKeep;
        
        SkyDomeParameters() :
        curvature(10.f),
        tiling(8.f),
        xSegments(16),
        ySegments(16),
        ySegmentsKeep(-1) {}
    };

    /// Sky parameters for skyplane, see Ogre documentation for more info.
    struct SkyPlaneParameters
    {
        float scale;
        float tiling; 
        float bow; 
        int xSegments; 
        int ySegments; 
        
        ///\todo ctor with default values.
    };

    class Renderer;
    typedef boost::shared_ptr<Renderer> RendererPtr;
    typedef boost::weak_ptr<Renderer> RendererWeakPtr;

    /** Ogre sky component
     *
     *  \ingroup OgreRenderingModuleClient
     *
     */
    class OGRE_MODULE_API EC_OgreSky : public Foundation::ComponentInterface
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
        bool SetSkyBox(const std::string& material_name, Real distance);
        
        /// Sets the sky material.
        /// @param material_name Material name.
        void SetSkyMaterial(const std::string& material_name) { genericSkyParameters.material = material_name; }
        
        /// Sets new textures for SkyBox.
        /// @param index Index of the texture.
        /// @param texture name Texture name.
        void SetSkyBoxMaterialTexture(int index, const char *texture_name, size_t image_count);
        
        /// Sets the texture for SkyDome.
        /// @param texture name Texture name.
        void SetSkyDomeMaterialTexture(const char *texture_name, const SkyImageData *parameters);
        
        /// Sets the texture for SkyPlane.
        /// @param texture name Texture name.
        void SetSkyPlaneMaterialTexture(const char *texture_name);
        
        /// Reads the sky parameters from the configuration file.
        void GetSkyConfig();
            
    private:
        /// Constructor
        /// @param module ModuleInterface pointer.
        EC_OgreSky(Foundation::ModuleInterface* module);
        
        /// Renderer
        RendererWeakPtr renderer_;
        
        /// whether sky enabled by this component
        bool skyEnabled_;
        
        /// Type of the sky.
        SkyType type_;

        /// UUID's of the texture assets the skybox uses for rendering. Should be stored per-scene.
    //    RexUUID skyboxTexturesIds_[SKYBOX_IND_COUNT];
        
        /// Generic sky parameters common to all sky types.
        SkyParameters genericSkyParameters;

        /// Parameters for skydome
        SkyDomeParameters skyDomeParameters;
       
        /// Parameters for skyplane.
        SkyPlaneParameters skyPlaneParameters;
           
        /// List of skybox image names.
        std::vector<std::string> skyBoxImages_;
       
        Foundation::ModuleInterface* owner_; 
        
        size_t currentSkyBoxImageCount_;
    };
}

#endif
