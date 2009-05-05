#include "StableHeaders.h"

#include "StableHeaders.h"

#include "Entity.h"

/* this is not a module but defines a type for the module created in PythonScriptModule */
//initentity(void) 

namespace PythonScript
{
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
		eob = PyObject_New(rexviewer_EntityObject, &rexviewer_EntityType); //sets refcount to 1
		eob->entity = entity; //doesn't have a constructor, just this factory

		return (PyObject*) eob;
		//Py_RETURN_NONE;
	}
}