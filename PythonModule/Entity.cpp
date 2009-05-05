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

	using namespace PythonScript;

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

	PyObject* PythonScript::entity_create(Foundation::EntityPtr entity)
	{
		//PyObject* pye;
		rexviewer_EntityObject* eob;
		std::cout << "Entity: creating a wrapper pyobject ..";
		eob = PyObject_New(rexviewer_EntityObject, &rexviewer_EntityType); //sets refcount to 1

		std::cout << "setting the pointer to the entity in the wrapper: " << entity << std::endl;
		eob->entity = entity; //doesn't have a constructor, just this factory
		return (PyObject*) eob;
	}

	PyObject* PythonScript::entity_getattro(rexviewer_EntityObject* self, PyObject* name)
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
		//Py_RETURN_NONE;

		if (s_name == "prim")
		{
			std::cout << ".. getting prim" << std::endl;
			const Foundation::ComponentInterfacePtr &prim_component = self->entity->GetComponent("EC_OpenSimPrim");
			RexLogic::EC_OpenSimPrim *prim = checked_static_cast<RexLogic::EC_OpenSimPrim *>(prim_component.get());
		        
			//m->AddU32(prim->LocalId);
			std::string retstr = "local id:" + prim->FullId.ToString() + "- prim name: " + prim->ObjectName;
			return PyString_FromString(retstr.c_str());
		}

		std::cout << "unknown component type."  << std::endl;
		return NULL;
	}
}