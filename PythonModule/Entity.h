#ifndef incl_PythonEntityType_h
#define incl_PythonEntityType_h

#include "Foundation.h"
#ifdef PYTHON_FORCE_RELEASE_VERSION
  #ifdef _DEBUG
    #undef _DEBUG
    #include "Python.h"
    #define _DEBUG
  #else
    #include "Python.h"
  #endif 
#else
    #include "Python.h"
#endif

#include "EC_OpenSimPrim.h"
#include "EC_OgrePlaceable.h"

namespace PythonScript
{
	//the wrapper can't directly keep these 
	static std::map<Core::entity_id_t, Foundation::EntityPtr> entity_ptrs; //XXX should definitely be weakrefs (right?)
	//am getting 'already defined' when linking, what's wrong?

	typedef struct {
		PyObject_HEAD
		/* Type-specific fields go here. */
		//Foundation::EntityPtr entity;
		//smart_ptrs can't be just like this in pyobjects, see e.g. http://wiki.python.org/moin/boost.python/PointersAndSmartPointers 
		Core::entity_id_t ent_id;
	} rexviewer_EntityObject;

	//why can't these be static? doesn't find the definitions in that case..
	void entity_init(PyObject* m);
	PyObject* entity_create(Core::entity_id_t ent_id); //, Foundation::EntityPtr entity);

	static void foo();

	//static PyObject* entity_getattro(rexviewer_EntityObject *self, PyObject *name);
	//put the implementation here so it can be static, uh :/
	static PyObject* entity_getattro(rexviewer_EntityObject *self, PyObject *name)
	{
		PyObject* tmp;

		if (!(tmp = PyObject_GenericGetAttr((PyObject*)self, name))) {
			if (!PyErr_ExceptionMatches(PyExc_AttributeError))
				return NULL;
			PyErr_Clear();
		}
		else
			return tmp;

		char* s_name = PyString_AsString(name);

		std::cout << "Entity: getting unknown attribute: " << s_name;
		
		//entity_ptrs map usage
		/* this crashes now in boost, 
		   void add_ref_copy() { BOOST_INTERLOCKED_INCREMENT( &use_count_ );
		std::map<Core::entity_id_t, Foundation::EntityPtr>::iterator ep_iter = entity_ptrs.find(self->ent_id);
		Foundation::EntityPtr entity = ep_iter->second;
		fix.. */

		/* re-getting the EntityPtr as it wasn't stored anywhere yet,
		   is copy-paste from PythonScriptModule GetEntity 
		   but to be removed when that map is used above.*/
		Foundation::ScenePtr scene = PythonScript::GetScene();
		Foundation::EntityPtr entity = scene->GetEntity(self->ent_id);
		
		if (s_name == "prim")
		{
			std::cout << ".. getting prim" << std::endl;
			const Foundation::ComponentInterfacePtr &prim_component = entity->GetComponent("EC_OpenSimPrim");
			if (!prim_component)
				return NULL; //XXX report AttributeError
			RexLogic::EC_OpenSimPrim *prim = checked_static_cast<RexLogic::EC_OpenSimPrim *>(prim_component.get());
		        
			//m->AddU32(prim->LocalId);
			std::string retstr = "local id:" + prim->FullId.ToString() + "- prim name: " + prim->ObjectName;
			return PyString_FromString(retstr.c_str());
		}

		else if (s_name == "place")
		{
	        const Foundation::ComponentInterfacePtr &ogre_component = entity->GetComponent("EC_OgrePlaceable");
			if (!ogre_component)
	            return NULL; //XXX report AttributeError        
			OgreRenderer::EC_OgrePlaceable *placeable = checked_static_cast<OgreRenderer::EC_OgrePlaceable *>(ogre_component.get());
			
			/* this must probably return a new object, a 'Place' instance, that has these.
			   or do we wanna hide the E-C system in the api and have these directly on entity? 
			   probably not a good idea to hide the actual system that much. or? */
			RexTypes::Vector3 pos = placeable->GetPosition();
			//RexTypes::Vector3 scale = ogre_pos->GetScale();
			//RexTypes::Vector3 rot = Core::PackQuaternionToFloat3(ogre_pos->GetOrientation());
			/* .. i guess best to wrap the Rex Vector and other types soon,
			   the pyrr irrlicht binding project does it for these using swig,
			   https://opensvn.csie.org/traccgi/pyrr/browser/pyrr/irrlicht.i */
			return Py_BuildValue("fff", pos.x, pos.y, pos.z);
		}

		std::cout << "unknown component type."  << std::endl;
		return NULL;
	}

	static PyTypeObject rexviewer_EntityType = {
		PyObject_HEAD_INIT(NULL)
		0,                         /*ob_size*/
		"rexviewer.Entity",             /*tp_name*/
		sizeof(rexviewer_EntityObject), /*tp_basicsize*/
		0,                         /*tp_itemsize*/
		0,                         /*tp_dealloc*/
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
		(PyCFunction)entity_getattro, /*tp_getattro*/
		0,                         /*tp_setattro*/
		0,                         /*tp_as_buffer*/
		Py_TPFLAGS_DEFAULT,        /*tp_flags*/
		"Entity object",           /* tp_doc */
	};
}

#endif