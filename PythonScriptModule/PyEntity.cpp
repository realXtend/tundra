// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "PythonScriptModule.h" //for the staticframework hack, for entityptr re-get workaround
#include "PyEntity.h"

#include "PythonScriptModule.h"
//#include "RexLogicModule.h" //for getting prim component data
#include "SceneManager.h"
#include "EC_OgreMovableTextOverlay.h"
#include "EC_Placeable.h"
#include "EC_OgreCustomObject.h"
#include "EC_Mesh.h"
#include "EC_OgreCamera.h"
#include "EC_AnimationController.h"
#include "EC_NetworkPosition.h"
#include "EntityComponent/EC_AttachedSound.h"
#include "EC_SoundRuler.h"
#include "EC_Highlight.h"
#include "EC_Touchable.h"
#include "EC_OpenSimPrim.h"
#include "EC_DynamicComponent.h"
#include "EC_OpenSimPresence.h"

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
        const ComponentPtr &newcomponent = owner->GetFramework()->GetComponentManager()->CreateComponent(componentname);
        if (newcomponent)
        {
            entity->AddComponent(newcomponent);
        }
        else
        {
             owner->LogInfo("Create component: unknown component type string:");
             owner->LogInfo(componentname);
        }

        //const ComponentPtr &prim_component = entity->GetComponent("EC_OpenSimPrim");        

        Py_RETURN_NONE;
    }
    
    static PyObject* Entity_RemoveSoundComponent(PyEntity* self, PyObject* args)
    {
        PyObject *pyobComponent = NULL;

        if(!PyArg_ParseTuple(args, "O", &pyobComponent))
        {
            PyErr_SetString(PyExc_ValueError, "removeComponent expects a python object");
            return NULL;
        }
        if (!PyObject_TypeCheck(pyobComponent, &PythonQtInstanceWrapper_Type))
        {
            PyErr_SetString(PyExc_ValueError, "argument should be wrapped by PythonQt");
            return NULL;
        }
        PythonQtInstanceWrapper* wrappedAttachedSound= (PythonQtInstanceWrapper*)pyobComponent;
        QObject* qobject_ptr = wrappedAttachedSound->_obj;
        RexLogic::EC_AttachedSound* component_soundptr = (RexLogic::EC_AttachedSound*)qobject_ptr;
        
        PythonScriptModule *owner = PythonScriptModule::GetInstance();
        Scene::ScenePtr scene = owner->GetScenePtr();
        if (!scene)
        {
            PyErr_SetString(PyExc_RuntimeError, "default scene not there when trying to use an entity.");
            return NULL;
        }

        Scene::EntityPtr entity = scene->GetEntity(self->ent_id);
        ComponentPtr component_ptr = entity->GetComponent(component_soundptr->TypeName(), component_soundptr->Name());

        entity->RemoveComponent(component_ptr);

        Py_RETURN_NONE;
    }

    static PyObject* Entity_GetDynamicComponent(PyEntity* self, PyObject* args)
    {
        const char* dcname;

        if(!PyArg_ParseTuple(args, "s", &dcname))
        {
            PyErr_SetString(PyExc_ValueError, "getDynamicComponent expects the component identifier as a string");
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
        //XXX \todo could check if the entity is still there, in case it's deleted and py is still having a wrapper for it

        const ComponentPtr &dynamic_component_ptr = entity->GetComponent("EC_DynamicComponent", dcname);
        EC_DynamicComponent* dynamic_component = 0;
        if (dynamic_component_ptr)
        {
            dynamic_component = checked_static_cast<EC_DynamicComponent *>(dynamic_component_ptr.get());
            return PythonScriptModule::GetInstance()->WrapQObject(dynamic_component);
        }
        else
        {
            PyErr_SetString(PyExc_AttributeError, "Entity does not have a dynamic component with the given name:"); // " + dcname);
            return NULL;
        }
    }

    static PyMethodDef entity_methods[] = {
        {"createComponent", (PyCFunction)Entity_CreateComponent, METH_VARARGS,
         "Create a new component for this entity. The type constant is given as a string"},
        {"getDynamicComponent", (PyCFunction)Entity_GetDynamicComponent, METH_VARARGS,
         "Gets a DynamicComponent with a certain name from this entity. The name is given as a string"},
        {"removeSound", (PyCFunction)Entity_RemoveSoundComponent, METH_VARARGS,
         "Remove a component for this entity."},
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
        PyEntity *eob = PyObject_New(PyEntity, &PyEntityType); //sets refcount to 1
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

    //id is stored in the wrapper itself, and is return even if the entity is not in the scene anymore, for selection removal code to work in obedit.
    if (s_name.compare("id") == 0)
    {
        return Py_BuildValue("I", eob->ent_id); //unsigned int - is verified to be correct, same as c++ shows (at least in GetEntity debug print)
    }

    Scene::EntityPtr entity = scene->GetEntity(eob->ent_id);

    if (!entity)
    {
        PyErr_SetString(PyExc_ValueError, "This entity does not exist in the scene anymore. Was deleted by someone / something else?");
        return NULL;
    }


    else if (s_name.compare("prim") == 0)
    {
        EC_OpenSimPrim *prim = entity->GetComponent<EC_OpenSimPrim>().get();

        if (!prim)
        {
            PyErr_SetString(PyExc_AttributeError, "This entity does not have a prim component.");
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
        ComponentPtr component_meshptr = entity->GetComponent(EC_Mesh::TypeNameStatic());
        if (!component_meshptr)
        {
             PyErr_SetString(PyExc_AttributeError, "Entity doesn't have a mesh component.");
             return NULL;
        }
        EC_Mesh* ogremesh = checked_static_cast<EC_Mesh*>(component_meshptr.get());
        return PythonScriptModule::GetInstance()->WrapQObject(ogremesh);
    }
    
    else if (s_name.compare("placeable") == 0)
    {    
        EC_Placeable *placeable = entity->GetComponent<EC_Placeable>().get();
        if (!placeable)
        {
            PyErr_SetString(PyExc_AttributeError, "This entity does not have a placeable component.");
            return NULL;   
        }  

        return PythonScriptModule::GetInstance()->WrapQObject(placeable);
    }
    
    else if (s_name.compare("network") == 0)
    {
        EC_NetworkPosition* networkpos = dynamic_cast<EC_NetworkPosition*>(entity->GetComponent(EC_NetworkPosition::TypeNameStatic()).get());
        if (!networkpos)
        {
            PyErr_SetString(PyExc_AttributeError, "This entity does not have a networkpos component.");
            return NULL;   
        }  
        return PythonScriptModule::GetInstance()->WrapQObject(networkpos);
    }
    
    else if (s_name.compare("sound") == 0)
    {
        ComponentPtr component_soundptr = entity->GetComponent(RexLogic::EC_AttachedSound::TypeNameStatic());
        if (!component_soundptr)
        {
             PyErr_SetString(PyExc_AttributeError, "Entity doesn't have a sound component.");
             return NULL;
        }
        RexLogic::EC_AttachedSound* sound= checked_static_cast<RexLogic::EC_AttachedSound*>(component_soundptr.get());
        return PythonScriptModule::GetInstance()->WrapQObject(sound);
    }
    
    else if (s_name.compare("soundruler") == 0)
    {
        ComponentPtr soundrulerComponent= entity->GetComponent(EC_SoundRuler::TypeNameStatic());
        if (!soundrulerComponent)
        {
             PyErr_SetString(PyExc_AttributeError, "Entity doesn't have a sound ruler component.");
             return NULL;
        }
        EC_SoundRuler* soundruler = checked_static_cast<EC_SoundRuler*>(soundrulerComponent.get());
        return PythonScriptModule::GetInstance()->WrapQObject(soundruler);
    }

    else if (s_name.compare("text") == 0)
    {
        const ComponentPtr &overlay = entity->GetComponent(EC_OgreMovableTextOverlay::TypeNameStatic());

        if (!overlay)
        {
            PyErr_SetString(PyExc_AttributeError, "overlay not found.");
            return NULL;   
        }  
        EC_OgreMovableTextOverlay *name_overlay = checked_static_cast<EC_OgreMovableTextOverlay *>(overlay.get());
        std::string text = name_overlay->GetText();
        return PyString_FromString(text.c_str());
    }

    //XXX make the getter in EC_Mesh a qt slot and switch to using that
    else if (s_name.compare("boundingbox") == 0)
    {
        EC_Placeable *placeable = entity->GetComponent<EC_Placeable>().get();
        if (!placeable)
        {
            PyErr_SetString(PyExc_AttributeError, "This entity does not have a placeable component.");
            return NULL;   
        }  

        ComponentPtr meshptr = entity->GetComponent(EC_OgreCustomObject::TypeNameStatic());
        if (meshptr)
        {
            EC_OgreCustomObject& cobj = *checked_static_cast<EC_OgreCustomObject*>(meshptr.get());
            Vector3df min, max;
            
            cobj.GetBoundingBox(min, max);

            return Py_BuildValue("ffffffi", min.x, min.y, min.z, max.x, max.y, max.z, 0);
        }
        else
        {
            meshptr = entity->GetComponent(EC_Mesh::TypeNameStatic());
            if (meshptr) 
            {
                EC_Mesh& mesh = *checked_static_cast<EC_Mesh*>(meshptr.get());
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
        const ComponentPtr &highlight_componentptr = entity->GetComponent("EC_Highlight");
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
        const ComponentPtr &touchable_componentptr = entity->GetComponent("EC_Touchable");
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
        const ComponentPtr &dynamic_component_ptr = entity->GetComponent("EC_DynamicComponent");
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
        const ComponentPtr animationcontrol_ptr = entity->GetComponent("EC_AnimationController");
        EC_AnimationController* animationcontrol = 0;
        if (animationcontrol_ptr)
        {
            animationcontrol = checked_static_cast<EC_AnimationController *>(animationcontrol_ptr.get());
            return PythonScriptModule::GetInstance()->WrapQObject(animationcontrol);
        }
        else
        {
            PyErr_SetString(PyExc_AttributeError, "Entity does not have an AnimationController component.");
            return NULL;
        }
    }

    else if (s_name.compare("camera") == 0)
    {
        const ComponentPtr camera_ptr = entity->GetComponent("EC_OgreCamera");
        EC_OgreCamera* camera = 0;
        if (camera_ptr)
        {
            camera = checked_static_cast<EC_OgreCamera*>(camera_ptr.get());
            return PythonScriptModule::GetInstance()->WrapQObject(camera);
        }
        else
        {
            PyErr_SetString(PyExc_AttributeError, "Entity does not have a camera component.");
            return NULL;
        }
    }

    else if (s_name.compare("opensimpresence") == 0)
    {  
        EC_OpenSimPresence* presence_ptr = entity->GetComponent<EC_OpenSimPresence>().get();
        if (!presence_ptr)
        {
             PyErr_SetString(PyExc_AttributeError, "Entity doesn't have a opensimpresence component.");
             return NULL;
        }
        return PythonScriptModule::GetInstance()->WrapQObject(presence_ptr);
    }

    PyErr_SetString(PyExc_AttributeError, "Unknown component type.");
    return NULL;
}

static int entity_setattro(PyObject *self, PyObject *name, PyObject *value)
{
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
    EC_Placeable *placeable = entity->GetComponent<EC_Placeable>().get();
    EC_NetworkPosition* networkpos = entity->GetComponent<EC_NetworkPosition>().get();

    if (s_name.compare("text") == 0)
    {
        if (PyString_Check(value) || PyUnicode_Check(value)) 
        {
            const ComponentPtr &overlay = entity->GetComponent(EC_OgreMovableTextOverlay::TypeNameStatic());
            const char* c_text = PyString_AsString(value);
            std::string text = std::string(c_text);
            if (overlay)
            {
                EC_OgreMovableTextOverlay &name_overlay = *checked_static_cast<EC_OgreMovableTextOverlay*>(overlay.get());
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
