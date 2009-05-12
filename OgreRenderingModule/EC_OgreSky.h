// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_OgreSky_h
#define incl_OgreRenderer_EC_OgreSky_h

#include "ComponentInterface.h"
#include "Foundation.h"
#include "RexUUID.h"
#include "OgreModuleApi.h"

namespace OgreRenderer
{

class Renderer;

//! Sky type
enum SkyType
{
    SKYTYPE_PLANE,
    SKYTYPE_BOX,
    SKYTYPE_DOME,
    SKYTYPE_NONE
};

//! Enumeration of the skybox texture indexes.
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

    /// Skybox texture index.
    SkyBoxIndex index;

    float curvature;

    float tiling;
};

//! Generic ogre sky parameters, see Ogre documentation for more info.
struct SkyParameters
{
    std::string material;
    float distance;
    bool drawFirst;
    float angle;
    RexTypes::Vector3 angleAxis;
    
    // Set the default values.
    SkyParameters() : 
    material("Rex/skybox"),
    distance(50),
    drawFirst(true),
    angle(0.f),
    angleAxis(0, 0, 1) {}
};

//! Sky parameters for skydome, see Ogre documentation for more info.
struct SkydomeParameters
{
    float curvature;
    float tiling;
    int xSegments;
    int ySegments;
    int ySegmentsKeep;
    
    // Set the default values.
    SkydomeParameters() :
    curvature(10.f),
    tiling(8.f),
    xSegments(16),
    ySegments(16),
    ySegmentsKeep(-1) {}
};

typedef boost::shared_ptr<Renderer> RendererPtr;

//! Ogre sky component
/*! \todo other sky methods, like plane or dome
    \ingroup OgreRenderingModuleClient
 */
class OGRE_MODULE_API EC_OgreSky : public Foundation::ComponentInterface
{
    DECLARE_EC(EC_OgreSky);
public:
    virtual ~EC_OgreSky();

   /**
    * @param index Index of texture to be set.
    * @param id UUID of the new skybox texture.
    */
//   void SetSkyBoxTextureID(const SkyBoxIndex &index, const RexUUID &id) { skyboxTexturesIds_[index] = id; }

   /** @param index Index of texture to be get.
     * @return UUID of the skybox texture.
     */
//   const RexUUID &GetSkyBoxTextureID(const SkyBoxIndex &index) const { return skyboxTexturesIds_[index]; }
    
    /// Creates the default skybox. Note: this will be removed soon!
    void CreateDefaultSkybox();
   
    /// Creates the default sky. The sky type (box, dome, plane) is read from a config file.
    /// @param show Is the sky shown.
    void CreateDefaultSky(bool show = true);
   
    //! enables a skybox
    /*! other sky methods will be disabled
        \param material_name material name
        \param distance distance of skybox from camera
        \return true if successful
        \todo use material/texture asset reference when asset system exists
     */
   bool SetSkyBox(const std::string& material_name, Core::Real distance);

    //! disables sky methods
    void DisableSky();
       
    /** Update sky info.
     *
     * @param type Type of the sky: box, dome or none. 
     * @param images List of image uuids for the sky.
     * @param curvature If sky type is dome, curvature of the dome.
     * @param tiling If sky type is dome, tiling of the dome.
     */
    void UpdateSky(SkyType type, const std::vector<std::string> &images, float curvature, float tiling);
        
private:
    //! constructor
    /*! \param module renderer module
     */
    EC_OgreSky(Foundation::ModuleInterface* module);
    
    ///\todo remove?
    Foundation::ModuleInterface* owner_;
    
    //! Renderer
    RendererPtr renderer_;
    
    //! whether sky enabled by this component
    bool skyEnabled_;
    
    //! Type of the sky.
    SkyType type_;

    //! UUID's of the texture assets the skybox uses for rendering. Should be stored per-scene.
//    RexTypes::RexUUID skyboxTexturesIds_[SKYBOX_IND_COUNT];
    
    //! Generic sky parameters common to all sky types
    SkyParameters genericSkyParameters;

    //! Parameters for skydome
    SkydomeParameters skydomeParameters;
   
   //! List of skybox image names.
   std::vector<std::string> skyboxImages_;
    
};

} // namespace OgreRenderer

#endif