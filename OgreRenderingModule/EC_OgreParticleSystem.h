// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_OgreParticleSystem_h
#define incl_OgreRenderer_EC_OgreParticleSystem_h

#include "IComponent.h"
#include "OgreModuleApi.h"
#include "Vector3D.h"
#include "Quaternion.h"
#include "Declare_EC.h"

namespace Ogre
{
    class ParticleSystem;
    class SceneNode;
}

namespace OgreRenderer
{
    class Renderer;
    class EC_OgrePlaceable;
    
    typedef boost::shared_ptr<Renderer> RendererPtr;
    typedef boost::weak_ptr<Renderer> RendererWeakPtr;
    
    //! Ogre particle system entity component
    /*! May contain multiple particle systems created from templates.
        Needs to be attached to a placeable (aka scene node) to be visible.
        \ingroup OgreRenderingModuleClient
     */
    class OGRE_MODULE_API EC_OgreParticleSystem : public IComponent
    {
        Q_OBJECT
        
        DECLARE_EC(EC_OgreParticleSystem);
    public:
        virtual ~EC_OgreParticleSystem();

        //! gets placeable component
        ComponentPtr GetPlaceable() const { return placeable_; }
        
        //! sets placeable component
        /*! set a null placeable to detach the system(s), otherwise will attach
            \param placeable placeable component
         */
        void SetPlaceable(ComponentPtr placeable);
        
        //! sets draw distance
        /*! \param draw_distance New draw distance, 0.0 = draw always (default)
         */
        void SetDrawDistance(float draw_distance);
        
        //! Adds a particle system
        /*! \param system_name particle system template to use
            \return true if successful
         */
        bool AddParticleSystem(const std::string& system_name);
        
        //! Gets particle system count
        uint GetNumParticleSystems() const { return (uint)systems_.size(); }
        
        //! Gets particle system by index
        /*! \param index index
            \return particle system, or 0 if index out of bounds
         */
        Ogre::ParticleSystem* GetParticleSystem(uint index) const;
        
        //! Returns particle system name by index
        /*! \param index index
            \return name, or empty if index out of bounds
         */
        const std::string& GetParticleSystemName(uint index) const;
        
        //! Removes a particle system by índex
        /*! Note: indexes change after the removal, because null systems are never retained in the internal particle system list
            \param index index
            \return true if successful
         */
        bool RemoveParticleSystem(uint index);
        
        //! Removes all particle systems
        void RemoveParticleSystems();
        
        //! sets adjustment (offset) position
        /*! \param position new position
         */
        void SetAdjustPosition(const Vector3df& position);
        
        //! sets adjustment orientation
        /*! \param orientation new orientation
         */
        void SetAdjustOrientation(const Quaternion& orientation);
                
        //! sets adjustment scale
        /*! \param position new scale
         */
        void SetAdjustScale(const Vector3df& scale);
        
        //! returns adjustment position
        Vector3df GetAdjustPosition() const;
        //! returns adjustment orientation
        Quaternion GetAdjustOrientation() const;
        //! returns adjustment scale
        Vector3df GetAdjustScale() const;
        
        //! returns draw distance
        float GetDrawDistance() const { return draw_distance_; }
        
    private:
        //! constructor
        /*! \param module renderer module
         */
        EC_OgreParticleSystem(IModule* module);
        
        //! attaches systems to placeable
        void AttachSystems();
        
        //! detaches systems from placeable
        void DetachSystems();
        
        //! placeable component 
        ComponentPtr placeable_;
        
        //! renderer
        RendererWeakPtr renderer_;
        
        //! Ogre particle systems
        std::vector<Ogre::ParticleSystem*> systems_;
        
        //! Particle system names, need to keep track manually because Ogre does not store them
        StringVector system_names_;
        
        //! Adjustment scene node (scaling/offset/orientation modifications)
        Ogre::SceneNode* adjustment_node_;
        
        //! Attached to placeable-flag
        bool attached_;
        
        //! draw distance
        float draw_distance_;
    };
}

#endif
