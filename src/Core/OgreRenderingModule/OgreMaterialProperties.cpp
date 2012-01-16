/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   OgreMaterialProperties.cpp
 *  @brief  Dynamically created QProperties for OGRE material scripts.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "CoreDefines.h"
#include "OgreMaterialProperties.h"
#include "Renderer.h"

#include <OgreMaterial.h>
#include <OgreTechnique.h>
#include <OgreMaterialSerializer.h>

#include <QVariant>
#include <QVector>
#include <QTextStream>

PropertyMap GatherShaderParameters(const Ogre::MaterialPtr &material, bool includeTextureUnits)
{
    PropertyMap ret;
    if (material.isNull())
        return ret;

    // Material
    Ogre::Material::TechniqueIterator tIter = material->getTechniqueIterator();
    while(tIter.hasMoreElements())
    {
        // Technique
        Ogre::Technique *tech = tIter.getNext();
        Ogre::Technique::PassIterator pIter = tech->getPassIterator();
        while(pIter.hasMoreElements())
        {
            // Pass
            Ogre::Pass *pass = pIter.getNext();
            // Vertex program
            if (pass->hasVertexProgram())
            {
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
                            QString paramName = mapIter.peekNextKey().c_str();
                            const Ogre::GpuConstantDefinition &paramDef  = mapIter.getNext();

                            // Filter names that end with '[0]'
                            int found = paramName.indexOf("[0]");
                            if (found != -1)
                                continue;

                            // Ignore auto parameters
                            bool isAutoParam = false;
                            Ogre::GpuProgramParameters::AutoConstantIterator autoConstIter = verPtr->getAutoConstantIterator();
                            while(autoConstIter.hasMoreElements())
                            {
                                Ogre::GpuProgramParameters::AutoConstantEntry autoConstEnt = autoConstIter.getNext();
                                if (autoConstEnt.physicalIndex == paramDef.physicalIndex)
                                {
                                    isAutoParam = true;
                                    break;
                                }
                            }

                            if (isAutoParam)
                                continue;
//                            if (!paramDef.isFloat())
//                                continue;

                            size_t count = paramDef.elementSize * paramDef.arraySize;
                            QVector<float> paramValue;
                            QVector<float>::iterator iter;
                            paramValue.resize(count);
                            verPtr->_readRawConstants(paramDef.physicalIndex, count, &*paramValue.begin());

                            QTextStream vector_string;
                            QString string;
                            vector_string.setString(&string, QIODevice::WriteOnly);

                            for(iter = paramValue.begin(); iter != paramValue.end(); ++iter)
                                vector_string << *iter << " ";

                            QMap<QString, QVariant> typeValuePair;
                            //typeValuePair[OgreMaterialProperties::GpuConstantTypeToString(paramDef.constType)] = *vector_string.string();
                            typeValuePair[QString::number(paramDef.constType)] = *vector_string.string();
                            // Add to "VP" to the end of the parameter name in order to identify VP parameters.
                            ret[paramName.append(" VP").toLatin1()] = QVariant(typeValuePair);
                        }
                    }
                }
            }
            // Fragment program
            if (pass->hasFragmentProgram())
            {
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
                                QString paramName = mapIter.peekNextKey().c_str();
                                const Ogre::GpuConstantDefinition &paramDef  = mapIter.getNext();

                                // Filter names that end with '[0]'
                                int found = paramName.indexOf("[0]");
                                if (found != -1)
                                    continue;

                                // Ignore auto parameters
                                bool isAutoParam = false;
                                Ogre::GpuProgramParameters::AutoConstantIterator autoConstIter = fragPtr->getAutoConstantIterator();
                                while(autoConstIter.hasMoreElements())
                                {
                                    Ogre::GpuProgramParameters::AutoConstantEntry autoConstEnt = autoConstIter.getNext();
                                    if (autoConstEnt.physicalIndex == paramDef.physicalIndex)
                                    {
                                        isAutoParam = true;
                                        break;
                                    }
                                }

                                if (isAutoParam)
                                    continue;
//                                if (!paramDef.isFloat())
//                                    continue;

                                size_t count = paramDef.elementSize * paramDef.arraySize;
                                QVector<float> paramValue;
                                QVector<float>::iterator iter;
                                paramValue.resize(count);

                                fragPtr->_readRawConstants(paramDef.physicalIndex, count, &*paramValue.begin());

                                QTextStream vector_string;
                                QString string;
                                vector_string.setString(&string, QIODevice::WriteOnly);

                                for(iter = paramValue.begin(); iter != paramValue.end(); ++iter)
                                    vector_string << *iter << " ";

                                TypeValuePair typeValuePair;
                                //typeValuePair[OgreMaterialProperties::GpuConstantTypeToString(paramDef.constType)] = *vector_string.string();
                                typeValuePair[QString::number(paramDef.constType)] = *vector_string.string();
                                // Add to " FP" to the end of the parameter name in order to identify FP parameters
                                ret[paramName.append(" FP").toLatin1()] = QVariant(typeValuePair);
                            }
                        }
                    }
                }
            }

            if (includeTextureUnits)
            {
                Ogre::Pass::TextureUnitStateIterator texIter = pass->getTextureUnitStateIterator();
                while(texIter.hasMoreElements())
                {
                    // Texture units
                    const Ogre::TextureUnitState *tu = texIter.getNext();
                    // Don't insert tu's with empty texture names (i.e. shadowMap)
                    if(tu->getTextureName().size() > 0)
                    {
                        QString tuName(tu->getName().c_str());
                        TypeValuePair typeValuePair;
                        typeValuePair[OgreMaterialProperties::TextureTypeToString(tu->getTextureType())] = tu->getTextureName().c_str();
                        // add to " TU" to the end of the parameter name in order to identify texture units.
                        ret[tuName.append(" TU").toLatin1()] = typeValuePair;
                    }
                }
            }
        }
    }

    return ret;
}

//OgreMaterialProperties::OgreMaterialProperties(const QString &name, AssetInterfacePtr asset)
OgreMaterialProperties::OgreMaterialProperties(const QString &name, void *asset)
    ///\todo Regression. Reimplement using the new Asset API. -jj.
//:material_(0)
{
    ///\todo Regression. Reimplement using the new Asset API. -jj.
    /*
    // Hack: tell the materialresource that shadows are medium quality, so that it won't create extra shadow texture units
    material_ = new OgreRenderer::OgreMaterialResource(asset->Id(), OgreRenderer::Shadows_Low, asset);
    if (material_)
        if (material_->IsValid())
            CreateProperties();

    setObjectName(name);
*/
}

OgreMaterialProperties::~OgreMaterialProperties()
{
    ///\todo Regression. Reimplement using the new Asset API. -jj.
//    SAFE_DELETE(material_);
}

bool OgreMaterialProperties::HasProperties() const
{
    return GetPropertyMap().size() > 0;
}

PropertyMap OgreMaterialProperties::GetPropertyMap() const
{
    PropertyMap map;

    QListIterator<QByteArray> it(dynamicPropertyNames());
    while(it.hasNext())
    {
        const QString &propertyName = it.next();
        if (propertyName.isNull() || propertyName.isEmpty())
            continue;
        map[propertyName] = property(propertyName.toStdString().c_str()).toMap();
    }

    return map;
}

bool OgreMaterialProperties::CreateProperties()
{
    return false;
    ///\todo Regression. Reimplement using the new Asset API. -jj.
/*
    Ogre::MaterialPtr matPtr = material_->GetMaterial();
    if (matPtr.isNull())
        return false;

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
                            QString paramName = mapIter.peekNextKey().c_str();
                            const Ogre::GpuConstantDefinition &paramDef  = mapIter.getNext();

                            // Filter names that end with '[0]'
                            int found = paramName.indexOf("[0]");
                            if (found != -1)
                                continue;

                            // Ignore auto parameters
                            bool isAutoParam = false;
                            Ogre::GpuProgramParameters::AutoConstantIterator autoConstIter = verPtr->getAutoConstantIterator();
                            while(autoConstIter.hasMoreElements())
                            {
                                Ogre::GpuProgramParameters::AutoConstantEntry autoConstEnt = autoConstIter.getNext();
                                if (autoConstEnt.physicalIndex == paramDef.physicalIndex)
                                {
                                    isAutoParam = true;
                                    break;
                                }
                            }

                            if (isAutoParam)
                                continue;

                            if (!paramDef.isFloat())
                                continue;

                            size_t count = paramDef.elementSize * paramDef.arraySize;
                            QVector<float> paramValue;
                            QVector<float>::iterator iter;
                            paramValue.resize(count);
                            verPtr->_readRawConstants(paramDef.physicalIndex, count, &*paramValue.begin());

                            QTextStream vector_string;
                            QString string;
                            vector_string.setString(&string, QIODevice::WriteOnly);

                            for(iter = paramValue.begin(); iter != paramValue.end(); ++iter)
                                vector_string << *iter << " ";

                            // Add QPROPERTY. Add to "VP" to the end of the parameter name in order to identify VP parameters.
                            QMap<QString, QVariant> typeValuePair;
                            typeValuePair[GpuConstantTypeToString(paramDef.constType)] = *vector_string.string();
                            setProperty(paramName.append(" VP").toLatin1(), QVariant(typeValuePair));
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
                                QString paramName = mapIter.peekNextKey().c_str();
                                const Ogre::GpuConstantDefinition &paramDef  = mapIter.getNext();

                                // Filter names that end with '[0]'
                                int found = paramName.indexOf("[0]");
                                if (found != -1)
                                    continue;

                                // Ignore auto parameters
                                bool isAutoParam = false;
                                Ogre::GpuProgramParameters::AutoConstantIterator autoConstIter = fragPtr->getAutoConstantIterator();
                                while(autoConstIter.hasMoreElements())
                                {
                                    Ogre::GpuProgramParameters::AutoConstantEntry autoConstEnt = autoConstIter.getNext();
                                    if (autoConstEnt.physicalIndex == paramDef.physicalIndex)
                                    {
                                        isAutoParam = true;
                                        break;
                                    }
                                }

                                if (isAutoParam)
                                    continue;

                                if (!paramDef.isFloat())
                                    continue;

                                size_t count = paramDef.elementSize * paramDef.arraySize;
                                QVector<float> paramValue;
                                QVector<float>::iterator iter;
                                paramValue.resize(count);

                                fragPtr->_readRawConstants(paramDef.physicalIndex, count, &*paramValue.begin());

                                QTextStream vector_string;
                                QString string;
                                vector_string.setString(&string, QIODevice::WriteOnly);

                                for(iter = paramValue.begin(); iter != paramValue.end(); ++iter)
                                    vector_string << *iter << " ";

                                // Add QPROPERTY. Add to " FP" to the end of the parameter name in order to identify FP parameters
                                TypeValuePair typeValuePair;
                                typeValuePair[GpuConstantTypeToString(paramDef.constType)] = *vector_string.string();
                                setProperty(paramName.append(" FP").toLatin1(), QVariant(typeValuePair));
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
                // add to " TU" to the end of the parameter name in order to identify texture units.
                if(tu->getTextureName().size() > 0)
                {
                    QString tuName(tu->getName().c_str());

                    // Add QPROPERTY
                    TypeValuePair typeValuePair;
                    typeValuePair[TextureTypeToString(tu->getTextureType())] = tu->getTextureName().c_str();
                    setProperty(tuName.append(" TU").toLatin1(), typeValuePair);
                }
            }
        }
    }

    return true;
*/
}

Ogre::MaterialPtr OgreMaterialProperties::ToOgreMaterial()
{
    ///\todo Regression. Reimplement using the new Asset API. -jj.
    return Ogre::MaterialPtr();
#if 0
    // Make clone from the original and uset that for creating the new material.
    Ogre::MaterialPtr matPtr = material_->GetMaterial();
    Ogre::MaterialPtr matPtrClone = matPtr->clone(objectName().toStdString() + "Clone");

    // Material
    if (!matPtrClone.isNull())
    {
        // Technique
        Ogre::Material::TechniqueIterator tIter = matPtrClone->getTechniqueIterator();
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
                            //int constNum = 0;
                            while(mapIter.hasMoreElements())
                            {
                                QString paramName(mapIter.peekNextKey().c_str());
                                const Ogre::GpuConstantDefinition &paramDef  = mapIter.getNext();

                                // Filter names that end with '[0]'
                                if (paramName.lastIndexOf("[0]") != -1)
                                    continue;

                                if (!paramDef.isFloat())
                                    continue;

                                size_t size = paramDef.elementSize * paramDef.arraySize;
                                QVector<float> newParamValue;
                                QVector<float>::iterator it;
                                newParamValue.resize(size);

                                // Find the corresponding property value.
                                QVariant val = property(paramName.append(" VP").toLatin1());
                                if (!val.isValid() || val.isNull())
                                    continue;

                                TypeValuePair typeValuePair = val.toMap();
                                QString newValueString(typeValuePair.begin().value().toByteArray());
                                newValueString = newValueString.trimmed();

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
                                ///\todo use the exact count rather than just 4 values if needed.
                                if (size == 16)
                                {
                                    Ogre::Matrix4 matrix(newParamValue[0], newParamValue[1], newParamValue[2], newParamValue[3],
                                        newParamValue[4], newParamValue[5], newParamValue[6], newParamValue[7],
                                        newParamValue[8], newParamValue[9], newParamValue[10], newParamValue[11],
                                        newParamValue[12], newParamValue[13], newParamValue[14], newParamValue[15]);

#if OGRE_VERSION_MINOR <= 6 && OGRE_VERSION_MAJOR <= 1
                                    verPtr->_writeRawConstant(paramDef.physicalIndex, matrix);
#else
                                    verPtr->_writeRawConstant(paramDef.physicalIndex, matrix, size);
#endif
                                }
                                else
                                {
                                    Ogre::Vector4 vector(newParamValue[0], newParamValue[1], newParamValue[2], newParamValue[3]);
                                    verPtr->_writeRawConstant(paramDef.physicalIndex, vector);
                                }
                            }
                        }
                    }
                }

                if (pass->hasFragmentProgram())
                {
                    // Fragment program
                    const Ogre::GpuProgramPtr &fragProg = pass->getFragmentProgram();
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
                                    QString paramName(mapIter.peekNextKey().c_str());
                                    const Ogre::GpuConstantDefinition &paramDef  = mapIter.getNext();

                                    // Filter names that end with '[0]'
                                    if (paramName.lastIndexOf("[0]") != -1)
                                        continue;

                                    if (!paramDef.isFloat())
                                        continue;

                                    size_t size = paramDef.elementSize * paramDef.arraySize;
                                    QVector<float> newParamValue;
                                    QVector<float>::iterator it;
                                    newParamValue.resize(size);

                                    // Find the corresponding property value.
                                    QVariant val = property(paramName.append(" FP").toLatin1());
                                    if (!val.isValid() || val.isNull())
                                        continue;

                                    TypeValuePair typeValuePair = val.toMap();
                                    QString newValueString(typeValuePair.begin().value().toByteArray());
                                    newValueString = newValueString.trimmed();

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

                                    // Set the new value.
                                    ///\todo use the exact count rather than just 4 values if needed.
                                    if (size == 16)
                                    {
                                        Ogre::Matrix4 matrix(newParamValue[0], newParamValue[1], newParamValue[2], newParamValue[3],
                                            newParamValue[4], newParamValue[5], newParamValue[6], newParamValue[7],
                                            newParamValue[8], newParamValue[9], newParamValue[10], newParamValue[11],
                                            newParamValue[12], newParamValue[13], newParamValue[14], newParamValue[15]);

#if OGRE_VERSION_MINOR <= 6 && OGRE_VERSION_MAJOR <= 1
                                    fragPtr->_writeRawConstant(paramDef.physicalIndex, matrix);
#else
                                    fragPtr->_writeRawConstant(paramDef.physicalIndex, matrix, size);
#endif
                                    }
                                    else
                                    {
                                        Ogre::Vector4 vector(newParamValue[0], newParamValue[1], newParamValue[2], newParamValue[3]);
                                        fragPtr->_writeRawConstant(paramDef.physicalIndex, vector);
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
                    Ogre::TextureUnitState *tu = texIter.getNext();

                    // Replace the texture name (uuid) with the new one
                    QString tu_name(tu->getName().c_str());

                    QVariant val = property(tu_name.append(" TU").toLatin1());
                    if (!val.isValid() || val.isNull())
                        continue;

                    TypeValuePair typeValuePair = val.toMap();
                    QString newValueString(typeValuePair.begin().value().toByteArray());
                    newValueString = newValueString.trimmed();

                    tu->setTextureName(AssetAPI::SanitateAssetRef(newValueString));
                    /*
                    //QString new_texture_name = iter->second;
                    RexUUID new_name(iter->second);
                    // If new texture is UUID-based one, make sure the corresponding RexOgreTexture gets created,
                    // because we may not be able to load it later if load fails now
                    if (RexUUID::IsValid(new_texture_name))
                    {
                        RexUUID imageID(new_texture_name);
                        if (!imageID.IsNull())
                        {
                            image* image = imageList.getImage(imageID);
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

        return matPtrClone;
    }

    matPtrClone.setNull();
    return matPtrClone;
#endif
}

QString OgreMaterialProperties::ToString()
{
    Ogre::MaterialPtr matPtr = ToOgreMaterial();
    if (matPtr.isNull())
        return "";

    Ogre::MaterialSerializer serializer;
    serializer.queueForExport(matPtr, true, false);
    return QString(serializer.getQueuedAsString().c_str());
}

QString OgreMaterialProperties::GpuConstantTypeToString(Ogre::GpuConstantType type)
{
    using namespace Ogre;
    ///@note We use GCT_UNKNOWN for texture units' texture names.
    switch(type)
    {
    case GCT_FLOAT1:
        return "float";
    case GCT_FLOAT2:
        return "float2";
    case GCT_FLOAT3:
        return "float3";
    case GCT_FLOAT4:
        return "float4";
    case GCT_SAMPLER1D:
        return "Sampler1D";
    case GCT_SAMPLER2D:
        return "Sampler2D";
    case GCT_SAMPLER3D:
        return "Sampler3D";
    case GCT_SAMPLERCUBE:
        return "SamplerCube";
    case GCT_SAMPLER1DSHADOW:
        return "Sampler1DShadow";
    case GCT_SAMPLER2DSHADOW:
        return "Sampler2DShadow";
    case GCT_MATRIX_2X2:
        return "float2x2";
    case GCT_MATRIX_2X3:
        return "float2x3";
    case GCT_MATRIX_2X4:
        return "float2x4";
    case GCT_MATRIX_3X2:
        return "float3x2";
    case GCT_MATRIX_3X3:
        return "float3x3";
    case GCT_MATRIX_3X4:
        return "float3x4";
    case GCT_MATRIX_4X2:
        return "float4x2";
    case GCT_MATRIX_4X3:
        return "float4x3";
    case GCT_MATRIX_4X4:
        return "float4x4";
    case GCT_INT1:
        return "int";
    case GCT_INT2:
        return "int2";
    case GCT_INT3:
        return "int3";
    case GCT_INT4:
        return "int4";
    case GCT_UNKNOWN:
    default:
        return "Unknown";
    };
}

QString OgreMaterialProperties::TextureTypeToString(Ogre::TextureType type)
{
    using namespace Ogre;
    switch(type)
    {
    case TEX_TYPE_1D:
        return "Tex1D";
    case TEX_TYPE_2D:
        return "Tex2D";
    case TEX_TYPE_3D:
        return "Tex3D";
    case TEX_TYPE_CUBE_MAP:
        return "TexCubeMap";
    default:
        return "Unknown";
    };
}

