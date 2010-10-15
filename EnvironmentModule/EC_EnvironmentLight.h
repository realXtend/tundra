// For conditions of distribution and use, see copyright notice in license.txt

#ifndef EC_ENVIRONMENTLIGHT_H_
#define EC_ENVIRONMENTLIGHT_H_

#include "IComponent.h"
#include "Declare_EC.h"
#include "CoreTypes.h"
#include "RexTypes.h"
#include <Color.h>
#include <QString>

#ifdef CAELUM
#include <Caelum.h>
#endif 

namespace OgreRenderer
{
    class Renderer;
    typedef boost::shared_ptr<Renderer> RendererPtr;
    typedef boost::weak_ptr<Renderer> RendererWeakPtr;
}

namespace Ogre
{
    class Light;
}

#ifdef CAELUM

namespace Caelum
{
    class CaelumSystem;
}

#endif 


/// Makes the entity a enviroment light.

/**

<table class="header">
<tr>
<td>
<h2>Water plane</h2>

Registered by Enviroment::EnvironmentModule. 

<b>Attributes</b>:
<ul>
<li> Color : sunColorAttr.
<div> </div>
<li> Color : ambientColorAttr.
<div> </div>
<li> Color : sunDiffuseColorAttr.
<div> </div>
<li> Vector3df : sunDirectionAttr.
<div></div>
<li> bool : fixedTimeAttr.
<div></div>
<li> float : currentTimeAttr.
<div>  </div>
<li> bool : sunCastShadowsAttr.
<div>  </div>

</table>

*/

namespace Environment
{

class EC_EnvironmentLight : public IComponent
{
    Q_OBJECT
    DECLARE_EC(EC_EnvironmentLight);
    
    public:
        virtual ~EC_EnvironmentLight();

        virtual bool IsSerializable() const { return true; }
       
        /// Defines sun color.
        DEFINE_QPROPERTY_ATTRIBUTE(Color,  sunColorAttr);
        Q_PROPERTY(Color sunColorAttr READ getsunColorAttr WRITE setsunColorAttr); 

        /// Defines ambient light color.
        DEFINE_QPROPERTY_ATTRIBUTE(Color, ambientColorAttr);
        Q_PROPERTY(Color ambientColorAttr READ getambientColorAttr WRITE setambientColorAttr); 

        /// Defines sun diffuse light color.
        DEFINE_QPROPERTY_ATTRIBUTE(Color, sunDiffuseColorAttr);
        Q_PROPERTY(Color sunDiffuseColorAttr READ getsunDiffuseColorAttr WRITE setsunDiffuseColorAttr); 
   
        /// Defines sun light direction.
        DEFINE_QPROPERTY_ATTRIBUTE(Vector3df, sunDirectionAttr);
        Q_PROPERTY(Vector3df sunDirectionAttr READ getsunDirectionAttr WRITE setsunDirectionAttr); 
   
        /// Defines that is fixed time used.
        DEFINE_QPROPERTY_ATTRIBUTE(bool, fixedTimeAttr);
        Q_PROPERTY(bool fixedTimeAttr READ getfixedTimeAttr WRITE setfixedTimeAttr); 

        /// Defines current time
        DEFINE_QPROPERTY_ATTRIBUTE(float, currentTimeAttr);
        Q_PROPERTY(float currentTimeAttr READ getcurrentTimeAttr WRITE setcurrentTimeAttr); 

        /// Defines time which is used currently
        DEFINE_QPROPERTY_ATTRIBUTE(bool, sunCastShadowsAttr);
        Q_PROPERTY(bool sunCastShadowsAttr READ getsunCastShadowsAttr WRITE setsunCastShadowsAttr); 

         /// Defines that is caelum to used to define sun color & direction vector and ambient light.
        DEFINE_QPROPERTY_ATTRIBUTE(bool, useCaelumAttr);
        Q_PROPERTY(bool useCaelumAttr READ getuseCaelumAttr WRITE setuseCaelumAttr); 

       
    public slots: 
       
        /// Called If some of the attributes has been changed.
        void AttributeUpdated(IAttribute* attribute, AttributeChange::Type change);

        /// Update sun state
        void UpdateSun();

        /// Remove sun.
        void RemoveSun();

        /// Update ambient light state.
        void UpdateAmbientLight();

        /// This is update loop, which will update caelum systems 
        void Update(float frameTime);

        void UpdateTime();

    private:
        /** 
         * Constuctor.
         * @param module Module where component belongs.
         **/
        explicit EC_EnvironmentLight(IModule *module);
        
        void CreateOgreLight();

        /**
         * Helper function which is used to update environment light. 
         */
        void ChangeEnvironmentLight(IAttribute* attribute);
        OgreRenderer::RendererWeakPtr renderer_;

         /// Sunlight which is used if there does not exist caelum
        Ogre::Light* sunLight_;

#ifdef CAELUM
        /// Caelum system.
        Caelum::CaelumSystem *caelumSystem_;
#endif
       
    
};

}

#endif // EC_ENVIRONMENTLIGHT_H