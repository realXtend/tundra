// For conditions of distribution and use, see copyright notice in license.txt

#ifndef EC_FOG_H_
#define EC_FOG_H_

#include "IComponent.h"
#include "Declare_EC.h"
#include "CoreTypes.h"
#include "RexTypes.h"
#include <Color.h>
#include <QString>

namespace Ogre
{
    class ColourValue;
}

/// Environment fog.

/**

<table class="header">
<tr>
<td>
<h2>Environment fog</h2>

Registered by Enviroment::EnvironmentModule. EC_Fog is a component which defines scene overall fog. Its values are used when camera is above of any water cube. If user wants to syncronize 
old world enviroment changes with other clients this component can be used. In those cases user needs to create to scene primitive which has EC_Name-component for that component attribute name 
must be se as "FogEnvironment" after that Naali's enviroment module will add EC_Fog for that primitive and it is then syncronizable with other clients. 

<b>Attributes</b>:
<ul>
<li> float : startDistanceAttr.
<div> The start distance from the camera of fog, note this has only effect if useAttr is on (or there does not exist Caelum).  </div>
<li> float : endDistanceAttr.
<div> The end distance from the camera of fog, note this has only effect if useAttr is on (or there does not exist Caelum). </div>
<li> Color : colorAttr.
<div> Defines what is fog color, note this has only effect if useAttr is on (or there does not exist Caelum). </div>
<li> int : modeAttr.
<div> Fog type ( none, linear, exp, exp2) this defines how fast fog density increases. </div>
<li> bool : useAttr.
<div> Defines that are this fog values used over Caelums calculated fog values. </div>

</ul>

<b> Exposes the following scriptable functions: </b>
<ul>
<li>...
</ul>

<b> Reacts on the following actions: </b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

</table>

*/


namespace Environment
{

class EC_Fog : public IComponent
{
    Q_OBJECT
    DECLARE_EC(EC_Fog);
   
    public:
        virtual ~EC_Fog() {}

        virtual bool IsSerializable() const { return true; }
         
        /// Fog start distance 
        DEFINE_QPROPERTY_ATTRIBUTE(float, startDistanceAttr);
        Q_PROPERTY(float startDistanceAttr READ getstartDistanceAttr WRITE setstartDistanceAttr); 
        
        /// Fog end distance
        DEFINE_QPROPERTY_ATTRIBUTE(float, endDistanceAttr);
        Q_PROPERTY(float endDistanceAttr READ getendDistanceAttr WRITE setendDistanceAttr); 
    
        /// Fog color
        DEFINE_QPROPERTY_ATTRIBUTE(Color, colorAttr);
        Q_PROPERTY(Color colorAttr READ getcolorAttr WRITE setcolorAttr); 

        /// Fog mode, defines how Fog density increases.
        DEFINE_QPROPERTY_ATTRIBUTE(int, modeAttr);
        Q_PROPERTY(int modeAttr READ getmodeAttr WRITE setmodeAttr); 
     
        /// Use this fog over Caelum (even that caelum is in use)
        DEFINE_QPROPERTY_ATTRIBUTE(bool, useAttr);
        Q_PROPERTY(bool useAttr READ getuseAttr WRITE setuseAttr); 
     
        /// Returns fog color as Ogre colour value. 
        Ogre::ColourValue GetColorAsOgreValue() const;
       
     private:
        /** 
         * Constuctor.
         * @param module Module where component belongs.
         **/
        explicit EC_Fog(IModule *module);

};

}

#endif // EC_FOG