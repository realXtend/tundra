#include "StableHeaders.h"

#include "Entity.h"

#include "RexLogicModule.h" //for getting prim component data
#include "EC_OpenSimPrim.h"

/* this is not a module but defines a type for the module created in PythonScriptModule */
//initentity(void) 

namespace PythonScript
{
	static PyMethodDef entity_methods[] = {
		{NULL}  /* Sentinel */
	};

	void PythonScript::entity_init(PyObject* m)
	{
 		rexviewer_EntityType.tp_new = PyType_GenericNew;
		if (PyType_Ready(&rexviewer_EntityType) < 0)
		{
			std::cout << "PythonScriptModule: EntityType not ready?" << std::endl;
			return;
		}

		Py_INCREF(&rexviewer_EntityType);
		PyModule_AddObject(m, "Entity", (PyObject *)&rexviewer_EntityType);
	}

	PyObject* PythonScript::entity_create(Core::entity_id_t ent_id, Foundation::EntityPtr entity)
	{
		//PyObject* pye;
		rexviewer_EntityObject* eob;
		std::cout << "Entity: creating a wrapper pyobject ..";
		eob = PyObject_New(rexviewer_EntityObject, &rexviewer_EntityType); //sets refcount to 1

		//std::cout << "setting the pointer to the entity in the wrapper: " << entity << std::endl;
		//eob->entity = entity; //doesn't have a constructor, just this factory
		//XXX creating the map failed for some reason
		//entity_ptrs[ent_id] = entity;
		return (PyObject*) eob;
	}

	PyObject* PythonScript::entity_getattro(PythonScript::rexviewer_EntityObject *self, PyObject *name)
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
		
		//entity_ptrs map usage, enable later
		//std::map<Core::entity_id_t, Foundation::EntityPtr>::iterator ep_iter = entity_ptrs.find(self->ent_id);
		//Foundation::EntityPtr entity = 0; //ep_iter->second;

		/* re-getting the EntityPtr as it wasn't stored anywhere yet,
		   is copy-paste from PythonScriptModule GetEntity 
		   but to be removed when that map is used above
		   .. can't get this to work as static, so can't get the framework, so commented out now
		   *
		Foundation::Framework *framework_ = PythonScript::staticframework;
		Foundation::SceneManagerServiceInterface *sceneManagerService = framework_->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager);
		const Foundation::ScenePtr &scene = sceneManagerService->GetScene("World"); //XXX hardcoded scene name, like in debugstats now
		if (scene == 0)
			return NULL; //XXX return some sensible exception info
		const Foundation::EntityPtr entity = scene->GetEntity(self->ent_id);
		/* end copy-paste / ptr re-get */

		/*
		if (s_name == "prim")
		{
			std::cout << ".. getting prim" << std::endl;
			const Foundation::ComponentInterfacePtr &prim_component = entity->GetComponent("EC_OpenSimPrim");
			RexLogic::EC_OpenSimPrim *prim = checked_static_cast<RexLogic::EC_OpenSimPrim *>(prim_component.get());
		        
			//m->AddU32(prim->LocalId);
			std::string retstr = "local id:" + prim->FullId.ToString() + "- prim name: " + prim->ObjectName;
			return PyString_FromString(retstr.c_str());
		}*/

		std::cout << "unknown component type."  << std::endl;
		return NULL;
	}

	void foo()
	{
		int a = 1 + 1;
	}
}