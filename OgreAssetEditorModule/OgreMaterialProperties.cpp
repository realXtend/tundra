#include "StableHeaders.h"
#include "OgreMaterialProperties.h"
#include "OgreMaterialResource.h"

#include <Ogre.h>

//#include <QString>
#include <QVariant>

namespace OgreAssetEditor
{

OgreMaterialProperties::OgreMaterialProperties(OgreRenderer::OgreMaterialResource *material)
{
    CreateProperties(material);
}

OgreMaterialProperties::~OgreMaterialProperties()
{
//    properties_.clear();
}

bool OgreMaterialProperties::CreateProperties(OgreRenderer::OgreMaterialResource *material)
{
    Ogre::MaterialPtr matPtr = material->GetMaterial();
    if (matPtr.isNull())
        return false;

    // Material
    Ogre::Material::TechniqueIterator tIter = matPtr->getTechniqueIterator();
    //int techNum = 0;
    while(tIter.hasMoreElements())
    {
        // Technique
        Ogre::Technique *tech = tIter.getNext();
        Ogre::Technique::PassIterator pIter = tech->getPassIterator();
        //int passNum = 0;
        while(pIter.hasMoreElements())
        {
            // Pass
            Ogre::Pass *pass = pIter.getNext();
            if (!pass)
                continue;

            if(pass->hasVertexProgram())
            {
                // Vertex program
                const Ogre::GpuProgramPtr &verProg = pass->getVertexProgram();
                if (!verProg.isNull())
                {
                    Ogre::GpuProgramParametersSharedPtr verPtr = pass->getVertexProgramParameters();
                    if (verPtr->hasNamedParameters())
                    {
                        // Named parameters (constants)
                        Ogre::GpuConstantDefinitionIterator mapIter = verPtr->getConstantDefinitionIterator();
                        //int constNum = 0;
                        while(mapIter.hasMoreElements())
                        {
                            std::string paramName = mapIter.peekNextKey();
                            const Ogre::GpuConstantDefinition &paramDef  = mapIter.getNext();
                            // Filter names that end with '[0]'
                            std::string::size_type found = paramName.find_last_of("[0]");
                            if (found == paramName.npos)
                            {
                                // Ignore auto parameters
                                bool is_auto_param = false;
                                Ogre::GpuProgramParameters::AutoConstantIterator autoConstIter = verPtr->getAutoConstantIterator();
                                while(autoConstIter.hasMoreElements())
                                {
                                    Ogre::GpuProgramParameters::AutoConstantEntry autoConstEnt = autoConstIter.getNext();
                                    if(autoConstEnt.physicalIndex == paramDef.physicalIndex)
                                    {
                                        is_auto_param = true;
                                        break;
                                    }
                                }
                                
                                if (!is_auto_param)
                                {
                                    if (paramDef.isFloat())
                                    {
                                        size_t count = paramDef.elementSize * paramDef.arraySize;
                                        std::vector<float> paramValue;
                                        std::vector<float>::iterator iter;
                                        paramValue.resize(count, 0);
                                        verPtr->_readRawConstants(paramDef.physicalIndex, count, &*paramValue.begin());

                                        std::stringstream vector_string;
                                        for(iter = paramValue.begin(); iter != paramValue.end(); ++iter)
                                            vector_string << *iter << " ";

                                        // Insert vertex program values into the map
                                        // add to "@" to the end of the parameter name in order to identify VP parameters
                                        paramName.append("@");
                                        // ADD PROPERTY
                                        //parameter_map.insert(std::pair<LLString, LLString>(paramName, vector_string.str()));
                                        setProperty(paramName.c_str(), QString(vector_string.str().c_str()));
                                    }
                                }
                            }
                            //++constNum;
                        }
                    }
                }
            }

            if(pass->hasFragmentProgram())
            {
                // Fragment program
                const Ogre::GpuProgramPtr fragProg = pass->getFragmentProgram();
                if (!fragProg.isNull())
                {
                    Ogre::GpuProgramParametersSharedPtr fragPtr = pass->getFragmentProgramParameters();
                    if (!fragPtr.isNull())
                    {
                        if (fragPtr->hasNamedParameters())
                        {
                            // Named parameters (constants)
                            Ogre::GpuConstantDefinitionIterator mapIter = fragPtr->getConstantDefinitionIterator();
                            //int constNum = 0;
                            while(mapIter.hasMoreElements())
                            {
                                std::string paramName = mapIter.peekNextKey();
                                const Ogre::GpuConstantDefinition &paramDef  = mapIter.getNext();
                                // Filter names that end with '[0]'
                                std::string::size_type found = paramName.find_last_of("[0]");
                                if (found == paramName.npos)
                                {
                                    // Ignore auto parameters
                                    bool is_auto_param = false;
                                    Ogre::GpuProgramParameters::AutoConstantIterator autoConstIter = fragPtr->getAutoConstantIterator();
                                    while(autoConstIter.hasMoreElements())
                                    {
                                        Ogre::GpuProgramParameters::AutoConstantEntry autoConstEnt = autoConstIter.getNext();
                                        if (autoConstEnt.physicalIndex == paramDef.physicalIndex)
                                        {
                                            is_auto_param = true;
                                            break;
                                        }
                                    }

                                    if(!is_auto_param)
                                    {
                                        if (paramDef.isFloat())
                                        {
                                            size_t count = paramDef.elementSize * paramDef.arraySize;
                                            std::vector<float> paramValue;
                                            std::vector<float>::iterator iter;
                                            paramValue.resize(count, 0);
                                            fragPtr->_readRawConstants(paramDef.physicalIndex, count, &*paramValue.begin());
                                            
                                            std::stringstream vector_string;
                                            for(iter = paramValue.begin(); iter != paramValue.end(); ++iter)
                                                vector_string << *iter << " ";

                                            // Insert fragment program values into the map
                                            // add to " FP" to the end of the parameter name in order to identify FP parameters
                                            paramName.append(" FP");
                                            // ADD PROPERTY
                                            //parameter_map.insert(std::pair<LLString, LLString>(paramName, vector_string.str()));
                                            setProperty(paramName.c_str(), QString(vector_string.str().c_str()));
                                        }
                                    }
                                }
                                //++constNum;
                            }
                        }
                    }
                }
            }

            Ogre::Pass::TextureUnitStateIterator texIter = pass->getTextureUnitStateIterator();
            //int tuNum = 0;
            while(texIter.hasMoreElements())
            {
                // Texture units
                const Ogre::TextureUnitState *tu = texIter.getNext();

                // Insert texture unit values into the map
                // Don't insert tu's with empty texture names (i.e. shadowMap)
                // add to "#" to the end of the parameter name in order to identify FP parameters
                if(tu->getTextureName().size() > 0)
                {
                    //QString tu_name(tu_name = tu->getName().c_str());
                    //tu_name.append("#");
                    // ADD PROPERTY: TEXTURE UNIT
                    //parameter_map.insert(std::pair<LLString, LLString>(tu_name, tu->getTextureName().c_str()));
                    setProperty(tu->getName().c_str(), QString(tu->getTextureName().c_str()));
                }
                //++tuNum;
            }
            //++passNum;
        }
        //++techNum;
    }

    return true;
}

}

