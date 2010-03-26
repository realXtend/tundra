// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   OgreMaterialProperties.cpp
 *  @brief  Dynamically created QProperties for OGRE material scripts.
 */

#include "StableHeaders.h"
#include "CoreDefines.h"
#include "OgreMaterialProperties.h"
#include "OgreMaterialResource.h"

#include <OgreMaterial.h>
#include <OgreTechnique.h>
#include <OgreMaterialSerializer.h>

#include <QVariant>
#include <QVector>
#include <QTextStream>

namespace Naali
{

OgreMaterialProperties::OgreMaterialProperties(const QString &name, Foundation::AssetPtr asset) :
    material_(0)
{
    material_ = new OgreRenderer::OgreMaterialResource(asset->GetId(), asset);
    if (material_)
        if (material_->IsValid())
            CreateProperties();

    setObjectName(name);
}

OgreMaterialProperties::~OgreMaterialProperties()
{
    SAFE_DELETE(material_);
}

bool OgreMaterialProperties::HasProperties()
{
    return GetPropertyMap().size() > 0;
}

PropertyMap OgreMaterialProperties::GetPropertyMap()
{
    PropertyMap map;

    QListIterator<QByteArray> it(dynamicPropertyNames());
    while(it.hasNext())
    {
        QString propertyName = it.next();
        if (propertyName.isNull() || propertyName.isEmpty())
            continue;
        map[propertyName] = property(propertyName.toStdString().c_str()).toMap();
    }

    return map;
}

bool OgreMaterialProperties::CreateProperties()
{
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

                                if (is_auto_param)
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
}

Ogre::MaterialPtr OgreMaterialProperties::ToOgreMaterial()
{
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
                    newValueString.trimmed();

                    tu->setTextureName(newValueString.toStdString());
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

// static
QString OgreMaterialProperties::GpuConstantTypeToString(const Ogre::GpuConstantType &type)
{
    using namespace Ogre;
    ///\note We use GCT_UNKNOWN for texture units' texture names.

    QString str("");
    switch(type)
    {
    case GCT_FLOAT1:
        str = "FLOAT1";
        break;
    case GCT_FLOAT2:
        str = "FLOAT2";
        break;
    case GCT_FLOAT3:
        str = "FLOAT3";
        break;
    case GCT_FLOAT4:
        str = "FLOAT4";
        break;
    case GCT_SAMPLER1D:
        str = "SAMPLER1D";
        break;
    case GCT_SAMPLER2D:
        str = "SAMPLER2D";
        break;
    case GCT_SAMPLER3D:
        str = "SAMPLER3D";
        break;
    case GCT_SAMPLERCUBE:
        str = "SAMPLERCUBE";
        break;
    case GCT_SAMPLER1DSHADOW:
        str = "SAMPLER1DSHADOW";
        break;
    case GCT_SAMPLER2DSHADOW:
        str = "SAMPLER2DSHADOW";
        break;
    case GCT_MATRIX_2X2:
        str = "MATRIX_2X2";
        break;
    case GCT_MATRIX_2X3:
        str = "MATRIX_2X3";
        break;
    case GCT_MATRIX_2X4:
        str = "MATRIX_2X4";
        break;
    case GCT_MATRIX_3X2:
        str = "MATRIX_3X2";
        break;
    case GCT_MATRIX_3X3:
        str = "MATRIX_3X3";
        break;
    case GCT_MATRIX_3X4:
        str = "MATRIX_3X4";
        break;
    case GCT_MATRIX_4X2:
        str = "MATRIX_4X2";
        break;
    case GCT_MATRIX_4X3:
        str = "MATRIX_4X3";
        break;
    case GCT_MATRIX_4X4:
        str = "MATRIX_4X4";
        break;
    case GCT_INT1:
        str = "INT1";
        break;
    case GCT_INT2:
        str = "INT2";
        break;
    case GCT_INT3:
        str = "INT3";
        break;
    case GCT_INT4:
        str = "INT4";
        break;
    case GCT_UNKNOWN:
    default:
        str = "UNKNOWN";
        break;
    };

    return str;
}

// static
QString OgreMaterialProperties::TextureTypeToString(const Ogre::TextureType &type)
{
    using namespace Ogre;

    QString str("");
    switch(type)
    {
    case TEX_TYPE_1D:
        str = "TEX_1D";
        break;
    case TEX_TYPE_2D:
        str = "TEX_2D";
        break;
    case TEX_TYPE_3D:
        str = "TEX_3D";
        break;
    case TEX_TYPE_CUBE_MAP:
        str = "TEX_CUBEMAP";
        break;
    default:
        str = "UNKNOWN";
        break;
    };

    return str;
}

}
