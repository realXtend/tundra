// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtModule_EC_UICanvas_h
#define incl_QtModule_EC_UICanvas_h

#include "ComponentInterface.h"
#include "Foundation.h"
#include "QtModuleApi.h"

#include <OgreMaterial.h>

namespace QtUI
{
    class UICanvas;
    
    /*! The presence of this object on an entity means that it is showing an UI canvas as a texture for a 3D object on
        one or more submeshes, ie. a EC_OgreMesh or EC_OgreCustomObject. The canvas can be interacted with through
        3D raycasting.
     */
    class QT_MODULE_API EC_UICanvas : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_UICanvas);
        
    public:
        virtual ~EC_UICanvas();
     
        //! Sets parent entity. Needed, because needs access to other components to change the materials etc.
        void SetEntity(Scene::Entity* entity);
        
        //! Sets UICanvas to use. Multiple EC_UICanvases can use the same UICanvas.
        /*! Needs internal canvas to work properly. 
         */
        void SetCanvas(boost::shared_ptr<UICanvas> canvas);
   
        //! Sets in which submeshes to show the UICanvas.
        void SetSubmeshes(const std::vector<uint>& submeshes);

        //! Sets in which submeshes to show the UICanvas (just one submesh)         
        void SetSubmeshes(uint submesh);
        
        //! Disables UICanvas from all submeshes. Also resets canvas referred to.
        void ClearSubmeshes();
     
        //! Refresh.
        /*! Ensures that submeshes which should have the canvas, have it, and restores previous material (if known)
            on submeshes which should not have it
         */
        void Refresh();
     
        //! Returns UICanvas.
        boost::shared_ptr<UICanvas> GetCanvas() const { return canvas_; }
        
        //! Returns entity.
        Scene::Entity* GetEntity() const { return entity_; }
        
        //! Returns submeshes to use
        const std::vector<uint>& GetSubmeshes() const { return submeshes_; }
        
    private:
        EC_UICanvas(Foundation::ModuleInterface* module);   
        
        boost::shared_ptr<UICanvas> canvas_;
        
        Scene::Entity* entity_;
    
        std::vector<uint> submeshes_;
        
        Ogre::MaterialPtr material_;
        
        StringVector original_materials_;
        
    };
}

#endif