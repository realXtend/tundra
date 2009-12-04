// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   OgreMaterialProperties.cpp
 *  @brief  Dynamically created QProperties for OGRE material scripts.
 */

#include "StableHeaders.h"
#include "OgreMaterialProperties.h"
#include "OgreMaterialResource.h"

#include <Ogre.h>

#include <QVariant>
#include <QVector>
#include <QTextStream>

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

    material_ = material;

    // Material
    Ogre::Material::TechniqueIterator tIter = matPtr->getTechniqueIterator();
    while(tIter.hasMoreElements())
    {
        // Technique
        Ogre::Technique *tech = tIter.getNext();
        Ogre::Technique::PassIterator pIter = tech->getPassIterator();
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
                        while(mapIter.hasMoreElements())
                        {
                            std::string paramName = mapIter.peekNextKey();
                            const Ogre::GpuConstantDefinition &paramDef  = mapIter.getNext();
                            // Filter names that end with '[0]'
                            std::string::size_type found = paramName.find_last_of("[0]");
                            if (found != paramName.npos)
                                continue;

                            // Ignore auto parameters
                            bool is_auto_param = false;
                            Ogre::GpuProgramParameters::AutoConstantIterator autoConstIter = verPtr->getAutoConstantIterator();
                            while(autoConstIter.hasMoreElements())
                            {
                                Ogre::GpuProgramParameters::AutoConstantEntry autoConstEnt = autoConstIter.getNext();
                                if (autoConstEnt.physicalIndex == paramDef.physicalIndex)
                                {
                                    is_auto_param = true;
                                    break;
                                }
                            }

                            if (is_auto_param)
                                continue;

                            if (!paramDef.isFloat())
                                continue;

                            size_t count = paramDef.elementSize * paramDef.arraySize;
                            QVector<float> paramValue;
                            QVector<float>::iterator iter;
                            paramValue.resize(count);
                            verPtr->_readRawConstants(paramDef.physicalIndex, count, &*paramValue.begin());

                            QTextStream vector_string;
                            //std::stringstream vector_string;
                            for(iter = paramValue.begin(); iter != paramValue.end(); ++iter)
                                vector_string << *iter << " ";

                            // Insert vertex program values into the map
                            // add to "VP" to the end of the parameter name in order to identify VP parameters
                            paramName.append(" VP");

                            // Add QPROPERTY
                            setProperty(paramName.c_str(), *vector_string.string());
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

                                            // Add QPROPERTY
                                            setProperty(paramName.c_str(), QString(vector_string.str().c_str()));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Ogre::Pass::TextureUnitStateIterator texIter = pass->getTextureUnitStateIterator();
            while(texIter.hasMoreElements())
            {
                // Texture units
                const Ogre::TextureUnitState *tu = texIter.getNext();

                // Don't insert tu's with empty texture names (i.e. shadowMap)
                // add to "TU" to the end of the parameter name in order to identify FP parameters
                if(tu->getTextureName().size() > 0)
                {
                    QString tu_name(tu->getName().c_str());
                    tu_name.append(" TU");
                    // Add QPROPERTY
                    setProperty(tu_name.toStdString().c_str(), QString(tu->getTextureName().c_str()));
                }
            }
        }
    }

    return true;
}

/*LLUUID asset_uuid, std::map<LLString, LLString> material_map*/
Ogre::MaterialPtr OgreMaterialProperties::ToOgreMaterial()
{
//    std::map<LLString, LLString>::const_iterator iter;

/*
    LLOgreAssetLoader* assetLoader = LLOgreRenderer::getPointer()->getAssetLoader();
    if (!assetLoader) 
    {
        Ogre::MaterialPtr nullPtr;
        nullPtr.setNull();
        return nullPtr;
    }

    if(!assetLoader->isMaterialLoaded(asset_uuid))
    {
        // Material not loaded, load now
        assetLoader->loadAsset(asset_uuid, LLAssetType::AT_MATERIAL, 0);
    }

    RexOgreMaterial* material = assetLoader->getMaterial(asset_uuid);
    if (!material) 
    {
        Ogre::MaterialPtr nullPtr;
        nullPtr.setNull();
        return nullPtr;
    }
*/

    // Make clone from the original and uset that for the new material
    Ogre::MaterialPtr MatPtr = material_->GetMaterial();
    Ogre::MaterialPtr MatPtrClone = MatPtr->clone("MatPtrClone");

    // Material
    if(!MatPtrClone.isNull())
    {
        // Technique
        Ogre::Material::TechniqueIterator tIter = MatPtrClone->getTechniqueIterator();
        while(tIter.hasMoreElements())
        {
            Ogre::Technique *tech = tIter.getNext();
            Ogre::Technique::PassIterator pIter = tech->getPassIterator();
            while(pIter.hasMoreElements())
            {
                // Pass
                Ogre::Pass *pass = pIter.getNext();
                if (!pass)
                    continue;

                if (pass->hasVertexProgram())
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
                            int constNum = 0;
                            while(mapIter.hasMoreElements())
                            {
                                QString paramName(mapIter.peekNextKey().c_str());
                                const Ogre::GpuConstantDefinition &paramDef  = mapIter.getNext();

                                // Filter names that end with '[0]'
                                if (paramName.lastIndexOf("[0]") != -1)
                                    continue;

                                if (!paramDef.isFloat())
                                    continue;

                                QVector<float> newParamValue;
                                QVector<float>::iterator it;
                                newParamValue.resize(paramDef.elementSize * paramDef.arraySize);

                                // Find the corresponding property value.
                                QVariant val = property(paramName.append(" VP").toLatin1());
                                if (!val.isValid() || val.isNull())
                                    continue;

                                QString newValueString(val.toByteArray());
                                newValueString.trimmed();

                                // fill the float vector with new values
                                it = newParamValue.begin();
                                int i = 0, j = 0;
                                bool ok = true;
                                while(j != -1 && ok)
                                {
                                    j = newValueString.indexOf(' ', i);
                                    QString newValue = newValueString.mid(i, j == -1 ? j : j - i);
                                    if (!newValue.isEmpty())
                                    {
                                        *it = newValue.toFloat(&ok);
                                        ++it;
                                    }
                                    i = j + 1;
                                }

                                // Set the new value.
                                Ogre::Vector4 vector(newParamValue[0], newParamValue[1], newParamValue[2], newParamValue[3]);
                                verPtr->_writeRawConstant(paramDef.physicalIndex, vector);
                            }
                        }
                    }
                }

                if (pass->hasFragmentProgram())
                {
                    // Fragment program
                    const Ogre::GpuProgramPtr &fragProg = pass->getFragmentProgram();
                    if(!fragProg.isNull())
                    {
                        Ogre::GpuProgramParametersSharedPtr fragPtr = pass->getFragmentProgramParameters();
                        if (!fragPtr.isNull())
                        {
                            if(fragPtr->hasNamedParameters())
                            {
                                // Named parameters (constants)
                                Ogre::GpuConstantDefinitionIterator mapIter = fragPtr->getConstantDefinitionIterator();
                                while(mapIter.hasMoreElements())
                                {
                                    QString paramName(mapIter.peekNextKey().c_str());
                                    const Ogre::GpuConstantDefinition &paramDef  = mapIter.getNext();

                                    // Filter names that end with '[0]'
                                    if (paramName.lastIndexOf("[0]") != -1)
                                        continue;

                                    if (!paramDef.isFloat())
                                        continue;

                                    QVector<float> newParamValue;
                                    QVector<float>::iterator it;
                                    newParamValue.resize(paramDef.elementSize * paramDef.arraySize);

                                    // Find the corresponding property value.
                                    QVariant val = property(paramName.append(" FP").toLatin1());
                                    if (!val.isValid() || val.isNull())
                                        continue;

                                    QString newValueString(val.toByteArray());
                                    newValueString.trimmed();

                                    // Fill the float vector with new values.
                                    it = newParamValue.begin();
                                    int i = 0, j = 0;
                                    bool ok = true;
                                    while(j != -1 && ok)
                                    {
                                        j = newValueString.indexOf(' ', i);
                                        QString newValue = newValueString.mid(i, j == -1 ? j : j - i);
                                        if (!newValue.isEmpty())
                                        {
                                            *it = *it = newValue.toFloat(&ok);
                                            ++it;
                                        }
                                        i = j + 1;
                                    }

                                    // Set the new value 
                                    Ogre::Vector4 vector(newParamValue[0], newParamValue[1], newParamValue[2], newParamValue[3]);
                                    fragPtr->_writeRawConstant(paramDef.physicalIndex, vector);
                                }
                            }
                        }
                    }
                }

                Ogre::Pass::TextureUnitStateIterator texIter = pass->getTextureUnitStateIterator();
                while(texIter.hasMoreElements())
                {
                    // Texture units
                    Ogre::TextureUnitState *tu = texIter.getNext();

                    // Replace the texture name (uuid) with the new one
                    QString tu_name(tu->getName().c_str());

                    QVariant val = property(tu_name.append(" TU").toLatin1());
                    if (!val.isValid() || val.isNull())
                        continue;

                    tu->setTextureName(QString(val.toByteArray()).toStdString().c_str());
                    /*
                    //QString new_texture_name = iter->second;
                    RexUUID new_name(iter->second);
                    // If new texture is UUID-based one, make sure the corresponding RexOgreTexture gets created,
                    // because we may not be able to load it later if load fails now
                    if (LLUUID::validate(new_texture_name))
                    {
                        LLUUID imageID(new_texture_name);
                        if (imageID != LLUUID::null)
                        {
                            LLViewerImage* image = gImageList.getImage(imageID);
                            if (image)
                            {
                                image->getOgreTexture();
                            }
                        }
                    }

                    //tu->setTextureName(iter->second);
                    */
                }
            }
        }

        return MatPtrClone;
    }

    MatPtrClone.setNull();
    return MatPtrClone;
}


}

