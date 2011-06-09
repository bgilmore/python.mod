#include "bridge.h"
#include "../module.h"

#undef global
static Function *global = NULL;

PyObject * TclBridge_call(PyObject *self, PyObject *args)
{	
	Tcl_Obj *cmdv  = NULL,
			**argv = NULL,
	        *t_arg = NULL;
	PyObject *p_arg  = NULL;
	Py_ssize_t i, len;
	char *strbuf;
	int argc;

	/* allocate Tcl list to hold command + args */
	cmdv = Tcl_NewListObj(0, NULL);

	/* marshal command/proc name into cmdv */
	p_arg = PyTuple_GET_ITEM(args, 0);

	if (PyString_Check(p_arg) == 0) {
		PyErr_SetString(PyExc_TypeError, "command must be passed as a string");
		goto out;
	}

	PyString_AsStringAndSize(p_arg, &strbuf, &len);
	t_arg = Tcl_NewStringObj(strbuf, len);
	Tcl_ListObjAppendElement(interp, cmdv, t_arg);

	/* marshal arguments into cmdv */
	for (i = 1; i < PyTuple_GET_SIZE(args); i++) {
		/* stringify argument */
		p_arg = PyObject_Str(PyTuple_GET_ITEM(args, i));
		if (p_arg == NULL) {
			PyErr_SetString(PyExc_TypeError,
				"all arguments must be callable or coercible to str");
			goto out;
		}

		/* copy from Python scope into Tcl scope */
		PyString_AsStringAndSize(p_arg, &strbuf, &len);
		t_arg = Tcl_NewStringObj(strbuf, len);
		Tcl_ListObjAppendElement(interp, cmdv, t_arg);

		/* delete stringified copy of argument */
		Py_DECREF(p_arg);
	}

	Tcl_ListObjGetElements(NULL, cmdv, &argc, &argv);
	Tcl_EvalObjv(interp, i, argv, TCL_EVAL_GLOBAL);

out:
	Tcl_DecrRefCount(cmdv);
	return Py_None;
}

PyTypeObject TclBridgeType = {
	PyObject_HEAD_INIT(0)
	0,                                        /* ob_size */
	"eggdrop.TclBridge",                      /* tp_name */
	0,                                        /* tp_basicsize */
	0,                                        /* tp_itemsize */


	/*** METHODS ***/

	0,                                        /* tp_dealloc */
	0,                                        /* tp_print */
	0,                                        /* tp_getattr */
	0,                                        /* tp_setattr */
	0,                                        /* tp_compare */
	0,                                        /* tp_repr */
	0,                                        /* tp_as_number*/
	0,                                        /* tp_as_sequence */
	0,                                        /* tp_as_mapping */
	0,                                        /* tp_hash */
	(ternaryfunc) TclBridge_call,             /* tp_call */
	0,                                        /* tp_str */
	PyObject_GenericGetAttr,                  /* tp_getattro */
	0,                                        /* tp_setattro */
	0,                                        /* tp_as_buffer */
	0,                                        /* tp_flags */
	0,                                        /* tp_doc */
	0,                                        /* tp_traverse */
	0,                                        /* tp_clear */
	0,                                        /* tp_richcompare */
	0,                                        /* tp_weaklistoffset */
	0,                                        /* tp_iter */
	0,                                        /* tp_iternext */
	0,                                        /* tp_methods */
	0,                                        /* tp_members */
	0,                                        /* tp_getset */
	0,                                        /* tp_base */
	0,                                        /* tp_dict */
	0,                                        /* tp_descr_get */
	0,                                        /* tp_descr_set */
	0,                                        /* tp_dictoffset */
	0,                                        /* tp_init */
	0,                                        /* tp_alloc */
	0,                                        /* tp_new */
	0,                                        /* tp_free */
};

