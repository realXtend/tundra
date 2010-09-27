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