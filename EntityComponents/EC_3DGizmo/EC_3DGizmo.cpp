/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_3DGizmo.h
 *  @brief  EC_3DGizmo enables manipulators for values
 *  @author Nathan Letwory | http://www.letworyinteractive.com
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EC_3DGizmo.h"
#include "IModule.h"
#include "Entity.h"
#include "Renderer.h"
#include "OgreMaterialUtils.h"
#include "EC_Placeable.h"
#include "EC_Mesh.h"
#include "EC_OgreCustomObject.h"
#include "EC_OpenSimPrim.h"
#include "LoggingFunctions.h"
#include "RexUUID.h"
#include <Ogre.h>

DEFINE_POCO_LOGGING_FUNCTIONS("EC_3DGizmo")

#include "MemoryLeakCheck.h"

EC_3DGizmo::EC_3DGizmo(IModule *module) :
    IComponent(module->GetFramework())
{
    renderer_ = module->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>();
    
    attributes_ = QList<IAttribute *>();
    subproperties_ = QStringList();
    attribute_ = QString();
}

EC_3DGizmo::~EC_3DGizmo()
{
}

void EC_3DGizmo::AddEditableAttribute(IComponent* component, QString attribute_name, QString subprop)
{
    if(attribute_.isEmpty())
        attribute_ = attribute_name;
    
    if(attribute_ == attribute_name) {
        IAttribute* attribute = component->GetAttribute(attribute_name);
        attributes_ << attribute;
        subproperties_ << subprop;

        for( int i = 0; i < attributes_.size(); i++)
        {
            IAttribute *attr = attributes_.at(i);
            std::cout << attr->GetNameString() << std::endl;
        }
    }
}

void EC_3DGizmo::RemoveEditableAttribute(IComponent* component, QString &attribute_name)
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

void EC_3DGizmo::ClearEditableAttributes()
{
    attributes_.clear();
    subproperties_.clear();
    attribute_.clear();
}

void EC_3DGizmo::Manipulate(QVariant datum)
{
    std::cout << "3dgizmo update" << std::endl;
    for(int i = 0; i < attributes_.size(); i++) {
        IAttribute * attr = attributes_.at(i);
        if(attr->GetNameString() == "Transform") {
            Attribute<Transform>* attribute = dynamic_cast<Attribute<Transform> *>(attr);
            if(attribute && datum.type()==QVariant::Vector3D) {
                Transform trans = attribute->Get();
                QVector3D vec = datum.value<QVector3D>();
                QString subproperty = subproperties_.at(i);
                if(subproperty=="position") {
                    trans.position.x += vec.x();
                    trans.position.y += vec.y();
                    trans.position.z += vec.z();
                } else if (subproperty=="scale") {
                    trans.scale.x += vec.x();
                    trans.scale.y += vec.y();
                    trans.scale.z += vec.z();
                } else if (subproperty=="rotation") {
                    trans.rotation.x += vec.x();
                    trans.rotation.y += vec.y();
                    trans.rotation.z += vec.z();
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
                        Vector3df value(vec.x(), vec.y(), vec.z());
                        Attribute<Vector3df> *attribute = dynamic_cast<Attribute<Vector3df> *>(attr);
                        if(attribute)
                            attribute->Set(attribute->Get() + value, AttributeChange::Default);
                    }
                    break;
            }
        }
    }
    std::cout << " ... ok" << std::endl;
}
