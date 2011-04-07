/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Gizmo.cpp
 *  @brief  EC_Gizmo enables manipulators for values
 *  @author Nathan Letwory | http://www.letworyinteractive.com
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EC_Gizmo.h"
#include "IModule.h"
#include "Entity.h"
#include "Renderer.h"
#include "OgreMaterialUtils.h"
#include "EC_Placeable.h"
#include "EC_Mesh.h"
#include "EC_OgreCustomObject.h"
#include "LoggingFunctions.h"
#ifdef ENABLE_TAIGA_SUPPORT
#include "RexUUID.h"
#endif
#include <Ogre.h>

#include "MemoryLeakCheck.h"

EC_Gizmo::EC_Gizmo(IModule *module) :
    IComponent(module->GetFramework())
{
    renderer_ = module->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>();
    
    attributes_ = QList<IAttribute *>();
    subproperties_ = QStringList();
    attribute_ = QString();
}

EC_Gizmo::~EC_Gizmo()
{
    attributes_.clear();
    subproperties_.clear();
}

void EC_Gizmo::AddEditableAttribute(IComponent* component, const QString &attribute_name, const QString &subprop)
{
    if(attribute_.isEmpty())
        attribute_ = attribute_name;
    
    if(attribute_ == attribute_name) {
        IAttribute* attribute = component->GetAttribute(attribute_name);
        if(attribute) {
            attributes_ << attribute;
            subproperties_ << subprop;
        }
    }
}

void EC_Gizmo::RemoveEditableAttribute(IComponent* component, const QString &attribute_name)
{
    int remove_idx = -1;
    IAttribute* attribute = component->GetAttribute(attribute_name);
    
    for(int i = 0; i < attributes_.size(); i++)
    {
        IAttribute *attr = attributes_.at(i);
        if(attr == attribute) {
            remove_idx = i;
            break;
        }
    }
    
    if(remove_idx!=-1) {
        attributes_.removeAt(remove_idx);
        subproperties_.removeAt(remove_idx);
    }
}

void EC_Gizmo::ClearEditableAttributes()
{
    attributes_.clear();
    subproperties_.clear();
    attribute_.clear();
}

void EC_Gizmo::Manipulate(const QVariant &datum)
{
    for(int i = 0; i < attributes_.size(); i++) {
        IAttribute * attr = attributes_.at(i);
        if(attr->GetNameString() == "Transform") {
            Attribute<Transform>* attribute = dynamic_cast<Attribute<Transform> *>(attr);
            if(attribute && datum.type()==QVariant::Vector3D) {
                Transform trans = attribute->Get();
                QVector3D vec = datum.value<QVector3D>();
                QString subproperty = subproperties_.at(i);
                if (subproperty=="scale") {
                    trans.scale.x += vec.x();
                    trans.scale.y += vec.y();
                    trans.scale.z += vec.z();
                }
                else if (subproperty=="rotation") {
                    trans.rotation.x += vec.x();
                    trans.rotation.y += vec.y();
                    trans.rotation.z += vec.z();
                }
                else { /* by default do position */
                    trans.position.x += vec.x();
                    trans.position.y += vec.y();
                    trans.position.z += vec.z();
                }
                attribute->Set(trans, AttributeChange::Default);
            }
        } else {
            switch(datum.type()) {
                case QVariant::Double:
                    {
                        float value = datum.toFloat();
                        Attribute<float> *attribute = dynamic_cast<Attribute<float> *>(attr);
                        if(attribute)
                            attribute->Set(attribute->Get() + value, AttributeChange::Default);
                    }
                    break;
                case QVariant::Int:
                    {
                        int value = datum.toInt();
                        Attribute<int> *attribute = dynamic_cast<Attribute<int> *>(attr);
                        if(attribute)
                            attribute->Set(attribute->Get() + value, AttributeChange::Default);
                    }
                    break;
                case QVariant::Bool:
                    {
                        bool value = datum.toBool();
                        Attribute<bool> *attribute = dynamic_cast<Attribute<bool> *>(attr);
                        if(attribute)
                            attribute->Set(value, AttributeChange::Default);
                    }
                    break;
                case QVariant::Vector3D:
                    {
                        QVector3D vec = datum.value<QVector3D>();
                        Attribute<QVector3D> *attribute = dynamic_cast<Attribute<QVector3D> *>(attr);
                        if(attribute) {
                            attribute->Set(attribute->Get() + vec, AttributeChange::Default);
                        } else {
                            Attribute<Vector3df> *attribute = dynamic_cast<Attribute<Vector3df> *>(attr);
                            Vector3df newvec(vec.x(), vec.y(), vec.z());
                            if(attribute) {
                                attribute->Set(attribute->Get() + newvec, AttributeChange::Default);
                            }
                        }
                    }
                    break;
            }
        }
    }
}
