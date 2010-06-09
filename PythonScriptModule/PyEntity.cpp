#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "PythonScriptModule.h" //for the staticframework hack, for entityptr re-get workaround
#include "PyEntity.h"

#include "PythonScriptModule.h"
#include "RexLogicModule.h" //for getting prim component data
#include "SceneManager.h"

#include "EC_OgreMovableTextOverlay.h"
#include "EC_OgreCustomObject.h"
#include "EC_OgreMesh.h"
#include "EC_OgreAnimationController.h"
#include "EntityComponent/EC_NetworkPosition.h"
#include "EC_Highlight.h"
#include "EC_Touchable.h"
#include "EC_OpenSimPrim.h"
#include "EC_DynamicComponent.h"

#include <PythonQt.h>

#include "MemoryLeakCheck.h"

static PyObject* entity_getattro(PyObject *self, PyObject *name);
static int entity_setattro(PyObject *self, PyObject *name, PyObject *value);

namespace PythonScript
{
    static PyObject* Entity_CreateComponent(PyEntity* self, PyObject* args)
    {
        const char* componentname;

        if(!PyArg_ParseTuple(args, "s", &componentname))
        {
            PyErr_SetString(PyExc_ValueError, "createComponent expects the component type given as a string");
            return NULL;
        }
        
        PythonScriptModule *owner = PythonScriptModule::GetInstance();
        Scene::ScenePtr scene = owner->GetScenePtr();
        if (!scene)
        {
            PyErr_SetString(PyExc_RuntimeError, "default scene not there when trying to use an entity.");
            return NULL;
        }

        Scene::EntityPtr entity = scene->GetEntity(self->ent_id);

        //XXX \todo check whether a component of the given type exists already for this entity. raise exception is yes?
        //entity->GetComponent("EC_Highlight")
        const Foundation::ComponentInterfacePtr &newcomponent = owner->GetFramework()->GetComponentManager()->CreateComponent(componentname);
        if (newcomponent)
        {
            entity->AddComponent(newcomponent);
        }
        else
        {
             owner->LogInfo("Create component: unknown component type string:");
             owner->LogInfo(componentname);
        }

        //const Foundation::ComponentInterfacePtr &prim_component = entity->GetComponent("EC_OpenSimPrim");        

        Py_RETURN_NONE;
    }

    static PyMethodDef entity_methods[] = {
        {"createComponent", (PyCFunction)Entity_CreateComponent, METH_VARARGS,
         "Create a new component for this entity. The type constant is given as string"},
        {NULL}  /* Sentinel */
    };    

    PyTypeObject PyEntityType = {
        PyObject_HEAD_INIT(NULL)
        0,                         /*ob_size*/
        "rexviewer.Entity",             /*tp_name*/
        sizeof(PythonScript::PyEntity), /*tp_basicsize*/
        0,                         /*tp_itemsize*/
        0,/*PyObject_Del,*/                         /*tp_dealloc*/
        0,                         /*tp_print*/
        0,                         /*tp_getattr*/
        0,                         /*tp_setattr*/
        0,                         /*tp_compare*/
        0,                         /*tp_repr*/
        0,                         /*tp_as_number*/
        0,                         /*tp_as_sequence*/
        0,                         /*tp_as_mapping*/
        0,                         /*tp_hash */
        0,                         /*tp_call*/
        0,                         /*tp_str*/
        entity_getattro,           /*tp_getattro*/
        entity_setattro,           /*tp_setattro*/
        0,                         /*tp_as_buffer*/
        Py_TPFLAGS_DEFAULT,        /*tp_flags*/
        "Entity object",           /* tp_doc */
        0,                   /* tp_traverse */
        0,                   /* tp_clear */
        0,                   /* tp_richcompare */
        0,                   /* tp_weaklistoffset */
        0,                   /* tp_iter */
        0,                   /* tp_iternext */
        entity_methods,            /* tp_methods */
        0,                         /* tp_members */
        0,                         /* tp_getset */
        0,                         /* tp_base */
        0,                         /* tp_dict */
        0,                         /* tp_descr_get */
        0,                         /* tp_descr_set */
        0,                         /* tp_dictoffset */
        0,                         /* tp_init */
        0,                         /* tp_alloc */
        0,                          /* tp_new */
    };

    void entity_init(PyObject* pyNamespace)
    {
        PyEntityType.tp_new = PyType_GenericNew;
        if (PyType_Ready(&PyEntityType) < 0)
        {
            //std::cout << "PythonScriptModule: EntityType not ready?" << std::endl;
            PythonScript::self()->LogDebug("PythonScriptModule: EntityType not ready?");
            return;
        }

        Py_INCREF(&PyEntityType);
        PyModule_AddObject(pyNamespace, "Entity", (PyObject *)&PyEntityType);
    }

    PyTypeObject *GetRexPyTypeObject()
    {
        return &PyEntityType;
    }

    PyObject* PythonScriptModule::entity_create(entity_id_t ent_id) //, Scene::EntityPtr entity)
    {
//        rexviewer_EntityObject* eob;
        //std::cout << "Entity: creating a wrapper pyobject ..";
        PyEntity *eob = PyObject_New(PyEntity, &PyEntityType); //sets refcount to 1

        //std::cout << "setting the pointer to the entity in the wrapper: " << entity << std::endl;
        //eob->entity = entity; //doesn't have a constructor, just this factory
        
        //std::cout << "storing the pointer to the entity in the entity_ptrs map:" << entity << std::endl;
        //entity_ptrs[ent_id] = entity;

        //std::cout << "storing the entity id in the wrapper object:" << ent_id << std::endl;
        PythonScript::self()->LogDebug("Storing the entity id in the wrapper object:" + QString::number(ent_id).toStdString());
        eob->ent_id = ent_id;
        return (PyObject*) eob;
    }
}

using namespace PythonScript;

static PyObject* entity_getattro(PyObject *self, PyObject *name)
{
    PyObject* tmp;

    if (!(tmp = PyObject_GenericGetAttr((PyObject*)self, name))) {
        if (!PyErr_ExceptionMatches(PyExc_AttributeError))
        {
            //std::cout << "..attribute error" << std::endl;
            return NULL;
        }

        PyErr_Clear();
    }
    else
        return tmp;

    const char* c_name = PyString_AsString(name);
    std::string s_name = std::string(c_name);

    //std::cout << "Entity: getting unknown attribute: " << s_name;
    
    //entity_ptrs map usage
    /* this crashes now in boost, 
       void add_ref_copy() { BOOST_INTERLOCKED_INCREMENT( &use_count_ );
    std::map<entity_id_t, Scene::EntityPtr>::iterator ep_iter = entity_ptrs.find(self->ent_id);
    Scene::EntityPtr entity = ep_iter->second;
    fix.. */

    /* re-getting the EntityPtr as it wasn't stored anywhere yet,
       is copy-paste from PythonScriptModule GetEntity 
       but to be removed when that map is used above.*/
    PythonScriptModule *owner = PythonScriptModule::GetInstance();
    Scene::ScenePtr scene = owner->GetScenePtr();
    if (!scene)
    {
        PyErr_SetString(PyExc_RuntimeError, "default scene not there when trying to use an entity.");
        return NULL;
    }

    PyEntity *eob = (PyEntity*) self;
    Scene::EntityPtr entity = scene->GetEntity(eob->ent_id);

    EC_OpenSimPrim *prim = entity->GetComponent<EC_OpenSimPrim>().get();
    OgreRenderer::EC_OgrePlaceable *placeable = entity->GetComponent<OgreRenderer::EC_OgrePlaceable>().get();

    RexLogic::EC_NetworkPosition* networkpos = dynamic_cast<RexLogic::EC_NetworkPosition*>(entity->GetComponent(RexLogic::EC_NetworkPosition::TypeNameStatic()).get());

    if (s_name.compare("id") == 0)
    {
        return Py_BuildValue("I", eob->ent_id); //unsigned int - is verified to be correct, same as c++ shows (at least in GetEntity debug print)
    }
    else if (s_name.compare("prim") == 0)
    {
        if (!prim)
        {
            PyErr_SetString(PyExc_AttributeError, "prim not found.");
            return NULL;   
        }  
        /*
        //m->AddU32(prim->LocalId);
        std::string retstr = "local id:" + prim->FullId.ToString() + "- prim name: " + prim->ObjectName;
        return PyString_FromString(retstr.c_str());
        */

        //RexLogic::EC_OpenSimPrim* prim = checked_static_cast<RexLogic::EC_OpenSimPrim*>(primentity->GetComponent(RexLogic::EC_OpenSimPrim::TypeNameStatic()).get());

        return PythonScriptModule::GetInstance()->WrapQObject(prim);
    }

    else if (s_name.compare("mesh") == 0)
    {
        Foundation::ComponentPtr component_meshptr = entity->GetComponent(OgreRenderer::EC_OgreMesh::TypeNameStatic());
        if (!component_meshptr)
        {
             PyErr_SetString(PyExc_AttributeError, "Entity doesn't have a mesh component.");
             return NULL;
        }
        OgreRenderer::EC_OgreMesh* ogremesh = checked_static_cast<OgreRenderer::EC_OgreMesh*>(component_meshptr.get());
        //placeable = checked_static_cast<OgreRenderer::EC_OgrePlaceable *>(ogre_component.get());       
        return PythonScriptModule::GetInstance()->WrapQObject(ogremesh);
    }
    
    else if (s_name.compare("placeable") == 0)
    {    
        return PythonScriptModule::GetInstance()->WrapQObject(placeable);
    }
    
    else if (s_name.compare("network") == 0)
    {
        return PythonScriptModule::GetInstance()->WrapQObject(networkpos);
    }

    else if(s_name.compare("editable") == 0)
    {
        // refactor to take into account permissions etc aswell later?
        if(!prim || !placeable)
            Py_RETURN_FALSE;
        else
            Py_RETURN_TRUE;
    }

    else if (s_name.compare("pos") == 0)
    {
        if (!placeable)
        {
            PyErr_SetString(PyExc_AttributeError, "placeable not found.");
            return NULL;   
        }       
        //std::cout << placeable->GetSelectPriority() << std::endl;
        /* this must probably return a new object, a 'Place' instance, that has these.
           or do we wanna hide the E-C system in the api and have these directly on entity? 
           probably not a good idea to hide the actual system that much. or? */
        Vector3df pos = placeable->GetPosition();
        //RexTypes::Vector3 scale = ogre_pos->GetScale();
        //RexTypes::Vector3 rot = PackQuaternionToFloat3(ogre_pos->GetOrientation());
        /* .. i guess best to wrap the Rex Vector and other types soon,
           the pyrr irrlicht binding project does it for these using swig,
           https://opensvn.csie.org/traccgi/pyrr/browser/pyrr/irrlicht.i 
    Now am experimenting with the new QVector3D type, see GetQPlaceable*/
        return Py_BuildValue("fff", pos.x, pos.y, pos.z);
    }

    else if (s_name.compare("scale") == 0)
    {
        if (!placeable)
        {
            PyErr_SetString(PyExc_AttributeError, "placeable not found.");
            return NULL;   
        }     
        Vector3df scale = placeable->GetScale();

        return Py_BuildValue("fff", scale.x, scale.y, scale.z);
    }

    else if (s_name.compare("orientation") == 0)
    {
        if (!placeable)
        {
            PyErr_SetString(PyExc_AttributeError, "placeable not found.");
            return NULL;   
        }         
        
        Quaternion orient = placeable->GetOrientation();
        return Py_BuildValue("ffff", orient.x, orient.y, orient.z, orient.w);
    }

    else if (s_name.compare("text") == 0)
    {
        const Foundation::ComponentInterfacePtr &overlay = entity->GetComponent(OgreRenderer::EC_OgreMovableTextOverlay::TypeNameStatic());

        if (!overlay)
        {
            PyErr_SetString(PyExc_AttributeError, "overlay not found.");
            return NULL;   
        }  
        OgreRenderer::EC_OgreMovableTextOverlay *name_overlay = checked_static_cast<OgreRenderer::EC_OgreMovableTextOverlay *>(overlay.get());
        std::string text = name_overlay->GetText();
        return PyString_FromString(text.c_str());
    }

    //XXX make the getter in EC_Mesh a qt slot and switch to using that
    else if (s_name.compare("boundingbox") == 0)
    {
        if (!placeable)
        {
            PyErr_SetString(PyExc_AttributeError, "placeable not found.");
            return NULL;  
        }
        Foundation::ComponentPtr meshptr = entity->GetComponent(OgreRenderer::EC_OgreCustomObject::TypeNameStatic());
        if (meshptr)
        {
            OgreRenderer::EC_OgreCustomObject& cobj = *checked_static_cast<OgreRenderer::EC_OgreCustomObject*>(meshptr.get());
            Vector3df min, max;
            
            cobj.GetBoundingBox(min, max);

            return Py_BuildValue("ffffffi", min.x, min.y, min.z, max.x, max.y, max.z, 0);
        }
        else
        {
            meshptr = entity->GetComponent(OgreRenderer::EC_OgreMesh::TypeNameStatic());
            if (meshptr) 
            {
                OgreRenderer::EC_OgreMesh& mesh = *checked_static_cast<OgreRenderer::EC_OgreMesh*>(meshptr.get());
                Vector3df min, max;

                mesh.GetBoundingBox(min, max);

                return Py_BuildValue("ffffffi", min.x, min.y, min.z, max.x, max.y, max.z, 1);
            }
        }
        
        PyErr_SetString(PyExc_AttributeError, "getting the bb failed.");
        return NULL;  
    }

    else if (s_name.compare("highlight") == 0)
    {
        //boost::shared_ptr<EC_Highlight> highlight = entity.GetComponent<EC_Highlight>();
        const Foundation::ComponentInterfacePtr &highlight_componentptr = entity->GetComponent("EC_Highlight");
        EC_Highlight* highlight = 0;
        if (highlight_componentptr)
        {
            highlight = checked_static_cast<EC_Highlight *>(highlight_componentptr.get());
            return PythonScriptModule::GetInstance()->WrapQObject(highlight);
        }
        else
        {
            PyErr_SetString(PyExc_AttributeError, "Entity does not have a highlight component.");
            return NULL;
        }
    }

    else if (s_name.compare("touchable") == 0)
    {
        //boost::shared_ptr<EC_Highlight> highlight = entity.GetComponent<EC_Highlight>();
        const Foundation::ComponentInterfacePtr &touchable_componentptr = entity->GetComponent("EC_Touchable");
        EC_Touchable* touchable = 0;
        if (touchable_componentptr)
        {
            touchable = checked_static_cast<EC_Touchable*>(touchable_componentptr.get());
            return PythonScriptModule::GetInstance()->WrapQObject(touchable);
        }
        else
        {
            PyErr_SetString(PyExc_AttributeError, "Entity does not have a touchable component.");
            return NULL;
        }
    }


    else if (s_name.compare("dynamic") == 0)
    {
        //boost::shared_ptr<EC_Highlight> highlight = entity.GetComponent<EC_Highlight>();
        const Foundation::ComponentInterfacePtr &dynamic_component_ptr = entity->GetComponent("EC_DynamicComponent");
        EC_DynamicComponent* dynamic_component = 0;
        if (dynamic_component_ptr)
        {
            dynamic_component = checked_static_cast<EC_DynamicComponent *>(dynamic_component_ptr.get());
            return PythonScriptModule::GetInstance()->WrapQObject(dynamic_component);
        }
        else
        {
            PyErr_SetString(PyExc_AttributeError, "Entity does not have a dynamic component.");
            return NULL;
        }
    }

    else if (s_name.compare("animationcontroller") == 0)
    {
        //boost::shared_ptr<EC_Highlight> highlight = entity.GetComponent<EC_Highlight>();
        const Foundation::ComponentInterfacePtr &animationcontrol_ptr = entity->GetComponent("EC_OgreAnimationController");
        OgreRenderer::EC_OgreAnimationController* animationcontrol = 0;
        if (animationcontrol_ptr)
        {
          animationcontrol = checked_static_cast<OgreRenderer::EC_OgreAnimationController *>(animationcontrol_ptr.get());
            return PythonScriptModule::GetInstance()->WrapQObject(animationcontrol);
        }
        else
        {
            PyErr_SetString(PyExc_AttributeError, "Entity does not have an AnimationController component.");
            return NULL;
        }
    }

    PyErr_SetString(PyExc_AttributeError, "Unknown component type.");
    return NULL;
}

static int entity_setattro(PyObject *self, PyObject *name, PyObject *value)
{
    /*
    if (!(tmp = PyObject_GenericSetAttr((PyObject*)self, name, value))) {
        if (!PyErr_ExceptionMatches(PyExc_AttributeError))
            return NULL;
        PyErr_Clear();
    }*/

    const char* c_name = PyString_AsString(name);
    std::string s_name = std::string(c_name);

    //std::cout << "Entity: setting unknown attribute: " << s_name;
    PyEntity *eob = (PyEntity*) self;

    //entity_ptrs map usage
    /* this crashes now in boost, 
       void add_ref_copy() { BOOST_INTERLOCKED_INCREMENT( &use_count_ );
    std::map<entity_id_t, Scene::EntityPtr>::iterator ep_iter = entity_ptrs.find(self->ent_id);
    Scene::EntityPtr entity = ep_iter->second;
    fix.. */

    /* re-getting the EntityPtr as it wasn't stored anywhere yet,
       is copy-paste from PythonScriptModule GetEntity 
       but to be removed when that map is used above.*/
    PythonScriptModule *owner = PythonScriptModule::GetInstance();
    Scene::ScenePtr scene = owner->GetScenePtr();
    if (!scene)
    {
        PyErr_SetString(PyExc_RuntimeError, "default scene not there when trying to use an entity.");
        return -1;
    }

    Scene::EntityPtr entity = scene->GetEntity(eob->ent_id);

    EC_OpenSimPrim *prim = entity->GetComponent<EC_OpenSimPrim>().get();
    OgreRenderer::EC_OgrePlaceable *placeable = entity->GetComponent<OgreRenderer::EC_OgrePlaceable>().get();
    RexLogic::EC_NetworkPosition* networkpos = entity->GetComponent<RexLogic::EC_NetworkPosition>().get();

    if (s_name.compare("pos") == 0)
    {
        /* this must probably return a new object, a 'Place' instance, that has these.
           or do we wanna hide the E-C system in the api and have these directly on entity? 
           probably not a good idea to hide the actual system that much. or? */
        float x, y, z;
        x = 0;
        y = 0;
        z = 0;

        //int parsing;
        //parsing = PyArg_ParseTuple(value, "fff", &x, &y, &z);
        /*
        PyObject* pos;
        if(!parsing) {
            parsing = PyArg_ParseTuple(value, "O", &pos);
            if (parsing != 0)
            {
                PyErr_SetString(PyExc_ValueError, "it worked.");
                return -1;
            }
        }
        */
        if(!PyArg_ParseTuple(value, "fff", &x, &y, &z))
        {    
            //std::cout << "...parse error" << std::endl;
            //PyErr_SetString(PyExc_ValueError, "params should be: (float, float, float).");
            return -1;
        }
        
        if (!placeable)
        {
            PyErr_SetString(PyExc_AttributeError, "placeable not found.");
            return -1;
        }  

        // Set the new values.
        placeable->SetPosition(Vector3df(x, y, z));
        if (networkpos)
        {
            // Override the dead reckoning system
            networkpos->SetPosition(placeable->GetPosition());
        }
            
        //ogre_pos->SetScale(OpenSimToOgreCoordinateAxes(scale));
        //ogre_pos->SetOrientation(OpenSimToOgreQuaternion(quat));
        /* .. i guess best to wrap the Rex Vector and other types soon,
           the pyrr irrlicht binding project does it for these using swig,
           https://opensvn.csie.org/traccgi/pyrr/browser/pyrr/irrlicht.i */

        /* sending a scene updated event to trigger network synch,
           copy-paste from DebugStats, 
           perhaps there'll be some MoveEntity thing in logic that can reuse for this? */
        return 0; //success.
    }

    else if (s_name.compare("scale") == 0)
    {
        float x, y, z;
        x = 0;
        y = 0;
        z = 0;
        if(!PyArg_ParseTuple(value, "fff", &x, &y, &z))
        {
            //PyErr_SetString(PyExc_ValueError, "params should be: (float, float, float)");
            return -1;   
        }

        if (!placeable)
        {
            PyErr_SetString(PyExc_AttributeError, "placeable not found.");
            return -1;   
        }  
        // Set the new values.
        placeable->SetScale(Vector3df(x, y, z));
 
        return 0; //success.
    }
    
    else if (s_name.compare("orientation") == 0)
    {
        float x, y, z, w;
        if(!PyArg_ParseTuple(value, "ffff", &x, &y, &z, &w))
        {
            PyErr_SetString(PyExc_ValueError, "params should be (float, float, float, float)"); //XXX change the exception
            return NULL;   
        }
        if (!placeable)
        {
            PyErr_SetString(PyExc_AttributeError, "placeable not found.");
            return -1;   
        }          
        // Set the new values.
        placeable->SetOrientation(Quaternion(x, y, z, w));
        if (networkpos)
        {
            // Override the dead reckoning system
            networkpos->SetOrientation(placeable->GetOrientation());
        }
                    
        return 0; //success.
    }

    else if (s_name.compare("text") == 0)
    {
        if (PyString_Check(value) || PyUnicode_Check(value)) 
        {
            const Foundation::ComponentPtr &overlay = entity->GetComponent(OgreRenderer::EC_OgreMovableTextOverlay::TypeNameStatic());
            const char* c_text = PyString_AsString(value);
            std::string text = std::string(c_text);
            if (overlay)
            {
                OgreRenderer::EC_OgreMovableTextOverlay &name_overlay = *checked_static_cast<OgreRenderer::EC_OgreMovableTextOverlay*>(overlay.get());
                name_overlay.SetText(text);
                //name_overlay.SetPlaceable(placeable); //is this actually needed for something?
            }
            else //xxx
            {
                PyErr_SetString(PyExc_ValueError, "overlay not found."); //XXX change the exception
                return -1;   
            }
        
        }
    }

    //std::cout << "unknown component type."  << std::endl;
    PythonScript::self()->LogDebug("Unknown component type.");
    return -1; //the way for setattr to report a failure
}
