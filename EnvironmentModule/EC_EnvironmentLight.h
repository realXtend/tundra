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
<h2>Environment Light</h2>

Enviroment light component is registered by Enviroment::EnvironmentModule. Component gives an access to various scene related environment settings, 
such as sunlight, ambient light. On default component uses Caelum and because of that changes to some of attributes (sun light color, direction ambient color) 
does not work until user disables Caelum (by setting useCaelum attribute to false). Use can also override server time by setting useFixedTime true, if that and useCaelum is set 
false user can totally control scene "global" lights.   

<h3> Using component to syncronize environment in Taiga </h3>

Currently (not in Tundra) EC_EnvironmentLight component can be used to syncronize lights in Taiga worlds. This can be done
so that user creates entity and sets entity EC_Name-component. If this component name is set as "LightEnvironment" our current implementation
will create automagically a EC_EnvironmentLight-component on it. This component is now usable for every users and all changes on it will be transfered 
to all users.  

<b>Attributes</b>:
<ul>
<li> Color : sunColorAttr.
<div> Defines sun color (in Caelum) </div>
<li> Color : ambientColorAttr.
<div> Defines scene ambient color. </div>
<li> Color : sunDiffuseColorAttr.
<div> Defines sun diffuse color.  </div>
<li> Vector3df : sunDirectionAttr.
<div> Defines sun direction vector (note caelum must be disabled). </div>
<li> bool : fixedTimeAttr.
<div> Defines that do we use time from server or do we use given fixed time. </div>
<li> float : currentTimeAttr.
<div> Current time which is used in scene </div>
<li> bool : sunCastShadowsAttr.
<div> Defines that does sun cast shadows (usable only if caelum is disabled) </div>
<li> bool : useCaelumAttr.
<div> Defines that do we use Caelum or not. </div>


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

        /// This is update loop, which will update caelum systems this is called in Environment-class for each frame.
        void Update(float frameTime);

        /// Update time.
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