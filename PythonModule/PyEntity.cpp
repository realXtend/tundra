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
#include "EntityComponent/EC_NetworkPosition.h"
#include <PythonQt.h>

#include "MemoryLeakCheck.h"

static PyObject* entity_getattro(PyObject *self, PyObject *name);
static int entity_setattro(PyObject *self, PyObject *name, PyObject *value);

namespace
{
    PyTypeObject rexviewer_EntityType = {
        PyObject_HEAD_INIT(NULL)
        0,                         /*ob_size*/
        "rexviewer.Entity",             /*tp_name*/
        sizeof(PythonScript::rexviewer_EntityObject), /*tp_basicsize*/
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
        entity_setattro,          /*tp_setattro*/
        0,                         /*tp_as_buffer*/
        Py_TPFLAGS_DEFAULT,        /*tp_flags*/
        "Entity object",           /* tp_doc */
    };
}

namespace PythonScript
{
    static PyMethodDef entity_methods[] = {
        {NULL}  /* Sentinel */
    };

    void entity_init(PyObject* pyNamespace)
    {
        rexviewer_EntityType.tp_new = PyType_GenericNew;
        if (PyType_Ready(&rexviewer_EntityType) < 0)
        {
            //std::cout << "PythonScriptModule: EntityType not ready?" << std::endl;
            PythonScript::self()->LogDebug("PythonScriptModule: EntityType not ready?");
            return;
        }

        Py_INCREF(&rexviewer_EntityType);
        PyModule_AddObject(pyNamespace, "Entity", (PyObject *)&rexviewer_EntityType);
    }

    PyTypeObject *GetRexPyTypeObject()
    {
        return &rexviewer_EntityType;
    }

    PyObject* PythonScriptModule::entity_create(entity_id_t ent_id) //, Scene::EntityPtr entity)
    {
//        rexviewer_EntityObject* eob;
        //std::cout << "Entity: creating a wrapper pyobject ..";
        rexviewer_EntityObject *eob = PyObject_New(rexviewer_EntityObject, &rexviewer_EntityType); //sets refcount to 1

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

/* this belongs to Entity.cpp but when added to the api from there, the staticframework is always null */
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
    Scene::ScenePtr scene = owner->GetScene();
    if (!scene)
    {
        PyErr_SetString(PyExc_RuntimeError, "default scene not there when trying to use an entity.");
        return NULL;
    }

    rexviewer_EntityObject *eob = (rexviewer_EntityObject *)self;
    Scene::EntityPtr entity = scene->GetEntity(eob->ent_id);

    const Foundation::ComponentInterfacePtr &prim_component = entity->GetComponent("EC_OpenSimPrim");
    RexLogic::EC_OpenSimPrim *prim = 0;
    if (prim_component)
		prim = checked_static_cast<RexLogic::EC_OpenSimPrim *>(prim_component.get());  
	
    const Foundation::ComponentInterfacePtr &ogre_component = entity->GetComponent("EC_OgrePlaceable");
    OgreRenderer::EC_OgrePlaceable *placeable = 0;
    if (ogre_component)
	placeable = checked_static_cast<OgreRenderer::EC_OgrePlaceable *>(ogre_component.get());       
    
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

        //RexLogic::EC_OpenSimPrim* prim = checked_static_cast<RexLogic::EC_OpenSimPrim*>(primentity->GetComponent(RexLogic::EC_OpenSimPrim::NameStatic()).get());

        return PythonQt::self()->wrapQObject(prim);
    }
    else if (s_name.compare("name") == 0)
    {
        //std::cout << ".. getting prim" << std::endl;
        if (!prim)
        {
            PyErr_SetString(PyExc_AttributeError, "prim not found.");
            return NULL;   
        }
        return PyString_FromString(prim->ObjectName.c_str());
    }

    else if (s_name.compare("meshid") == 0)
	{
        //std::cout << ".. getting prim in mesh getting" << std::endl;
        if (!prim)
        {
            PyErr_SetString(PyExc_AttributeError, "prim not found.");
            return NULL;   
        }  
        return PyString_FromString(prim->MeshID.c_str());	
        
        /* was a test thing, just changes what ogre shows locally
        Foundation::ComponentPtr placeable = entity->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic());
		Foundation::ComponentPtr component_meshptr = entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
		if (placeable)
		{
			OgreRenderer::EC_OgreMesh &ogremesh = *checked_static_cast<OgreRenderer::EC_OgreMesh*>(component_meshptr.get());
			
			std::string text = ogremesh.GetMeshName();
			return PyString_FromString(text.c_str());
		}*/
    }
    else if (s_name.compare("mesh") == 0)
	{
        Foundation::ComponentPtr component_meshptr = entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
        OgreRenderer::EC_OgreMesh* ogremesh = checked_static_cast<OgreRenderer::EC_OgreMesh*>(component_meshptr.get());
        //placeable = checked_static_cast<OgreRenderer::EC_OgrePlaceable *>(ogre_component.get());       
        return PythonQt::self()->wrapQObject(ogremesh);
    }
    else if (s_name.compare("placeable") == 0)
	{    
        return PythonQt::self()->wrapQObject(placeable);
    }

	else if(s_name.compare("uuid") == 0)
	{
		//std::cout << ".. getting prim" << std::endl;
        if (!prim)
        {
            PyErr_SetString(PyExc_AttributeError, "prim not found.");
            return NULL;
        }
		return PyString_FromString(prim->FullId.ToString().c_str());
	}
	else if(s_name.compare("updateflags") == 0)
	{
		//std::cout << ".. getting prim" << std::endl;
        if (!prim)
        {
            PyErr_SetString(PyExc_AttributeError, "prim not found.");
            return NULL;   
        }
		return Py_BuildValue("I", prim->UpdateFlags);
	}
	else if(s_name.compare("editable") == 0)
	{
		// refactor to take into account permissions etc aswell later?
		if(!prim_component || !placeable)
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
        const Foundation::ComponentInterfacePtr &overlay = entity->GetComponent(OgreRenderer::EC_OgreMovableTextOverlay::NameStatic());

        if (!overlay)
        {
            PyErr_SetString(PyExc_AttributeError, "overlay not found.");
            return NULL;   
        }  
        OgreRenderer::EC_OgreMovableTextOverlay *name_overlay = checked_static_cast<OgreRenderer::EC_OgreMovableTextOverlay *>(overlay.get());
        std::string text = name_overlay->GetText();
        return PyString_FromString(text.c_str());
    }
	else if (s_name.compare("boundingbox") == 0)
	{
		if (!placeable)
		{
			PyErr_SetString(PyExc_AttributeError, "placeable not found.");
            return NULL;  
		}
		Foundation::ComponentPtr meshptr = entity->GetComponent(OgreRenderer::EC_OgreCustomObject::NameStatic());
		if (meshptr)
		{
			OgreRenderer::EC_OgreCustomObject& cobj = *checked_static_cast<OgreRenderer::EC_OgreCustomObject*>(meshptr.get());
			Vector3df min, max;

			cobj.GetBoundingBox(min, max);

			return Py_BuildValue("ffffffi", min.x, min.y, min.z, max.x, max.y, max.z, 0);
		}
		else
		{
			meshptr = entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
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


    std::cout << "unknown component type."  << std::endl;
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
    rexviewer_EntityObject *eob = (rexviewer_EntityObject *)self;

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
    Scene::ScenePtr scene = owner->GetScene();
    if (!scene)
    {
        PyErr_SetString(PyExc_RuntimeError, "default scene not there when trying to use an entity.");
        return -1;
    }

    Scene::EntityPtr entity = scene->GetEntity(eob->ent_id);

    const Foundation::ComponentInterfacePtr &prim_component = entity->GetComponent("EC_OpenSimPrim");
	RexLogic::EC_OpenSimPrim *prim = 0;
    if (prim_component)
	   prim = checked_static_cast<RexLogic::EC_OpenSimPrim *>(prim_component.get());  
	
    const Foundation::ComponentInterfacePtr &ogre_component = entity->GetComponent("EC_OgrePlaceable");
	OgreRenderer::EC_OgrePlaceable *placeable = 0;
    if (ogre_component)
		placeable = checked_static_cast<OgreRenderer::EC_OgrePlaceable *>(ogre_component.get());       
    
    RexLogic::EC_NetworkPosition* networkpos = dynamic_cast<RexLogic::EC_NetworkPosition*>(entity->GetComponent(RexLogic::EC_NetworkPosition::NameStatic()).get());
    
    /*if (s_name.compare("prim") == 0)
    {
        std::cout << ".. getting prim" << std::endl;
        const Foundation::ComponentInterfacePtr &prim_component = entity->GetComponent("EC_OpenSimPrim");
        if (!prim_component)
            return NULL; //XXX report AttributeError
        RexLogic::EC_OpenSimPrim *prim = checked_static_cast<RexLogic::EC_OpenSimPrim *>(prim_component.get());
            
        //m->AddU32(prim->LocalId);
        std::string retstr = "local id:" + prim->FullId.ToString() + "- prim name: " + prim->ObjectName;
        return PyString_FromString(retstr.c_str());
    }*/
	//PyObject_Print(value, stdout, 0);
	//std::cout << "\n" << std::endl;
    //else 
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
            const Foundation::ComponentPtr &overlay = entity->GetComponent(OgreRenderer::EC_OgreMovableTextOverlay::NameStatic());
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
        else
        {
            PyErr_SetString(PyExc_ValueError, "text is a string"); //XXX change the exception
            return -1;
        }
        
        //if(!PyArg_ParseTuple(value, "s", c_text))
        //    return NULL; //XXX report ArgumentException error
                


        return 0;
    }
	else if (s_name.compare("meshid") == 0)
	{
	    //std::cout << "Setting mesh" << std::endl;
		if (PyString_Check(value) || PyUnicode_Check(value))
        {
			//NOTE: This is stricly done locally only for now, nothing is sent to the server.
			const char* c_text = PyString_AsString(value);
			std::string text = std::string(c_text);

            //std::cout << ".. getting prim in mesh setting" << std::endl;
			if (!prim)
			{
				PyErr_SetString(PyExc_AttributeError, "prim not found.");
				return -1;   
			}  

            prim->MeshID = text;
            prim->DrawType = RexTypes::DRAWTYPE_MESH;

            return 0;
        }
        
        /* was a test thing, just changes what ogre shows locally

			
            /*Foundation::ComponentPtr placeable = entity->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic());
			Foundation::ComponentPtr component_meshptr = entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
			if (placeable)
			{
				OgreRenderer::EC_OgreMesh &ogremesh = *checked_static_cast<OgreRenderer::EC_OgreMesh*>(component_meshptr.get());
				
				ogremesh.SetMesh(text);

				PythonScript::self()->LogInfo("Entity's mesh changed locally.");
				return NULL;
			}*/

        else
        {
            PyErr_SetString(PyExc_ValueError, "Mesh asset id is expected as a string"); //XXX change the exception
            return -1;
        }
	}
	
    //XXX why does this even exist when uuid is not settable?
	else if(s_name.compare("uuid") == 0)
	{
        PythonScript::self()->LogInfo("UUID cannot be set manually.");
        return 0;   
	}

    //std::cout << "unknown component type."  << std::endl;
	PythonScript::self()->LogDebug("Unknown component type.");
    return -1; //the way for setattr to report a failure
}
