#include "bridge.h"
#include "pymod.h"

static int run_command(PyObject *py_cmdv)
{
	Tcl_Obj *tcl_cmdv, *tcl_arg, **objv;
	PyObject *py_arg;
	Py_ssize_t len, i;
	char *strbuf;
	int objc;

	/* allocate empty list to hold translated cmdv */
	tcl_cmdv = Tcl_NewListObj(0, NULL);


	/* marshal proc name */
	py_arg = PyTuple_GET_ITEM(py_cmdv, 0);

	if (PyString_Check(py_arg) == 0) {
		PyErr_SetString(PyExc_TypeError, "proc name must be passed as a string");
		goto out;
	}

	PyString_AsStringAndSize(py_arg, &strbuf, &len);
	tcl_arg = Tcl_NewStringObj(strbuf, len);

	Tcl_ListObjAppendElement(NULL, tcl_cmdv, tcl_arg);


	/* marshal arguments */
	for (i = 1; i < PyTuple_GET_SIZE(py_cmdv); i++) {
		/* make a copy of the argument, enforcing stringification */
		py_arg = PyObject_Str(PyTuple_GET_ITEM(py_cmdv, i));
		if (py_arg == NULL) {
			PyErr_SetString(PyExc_TypeError,
				"all arguments must be callable or coercible to str");
			goto out;
		}

		PyString_AsStringAndSize(py_arg, &strbuf, &len);
		tcl_arg = Tcl_NewStringObj(strbuf, len);

		Tcl_ListObjAppendElement(NULL, tcl_cmdv, tcl_arg);

		/* delete stringified copy of argument */
		Py_DECREF(py_arg);
	}

	// execute translated cmdv (split into objv)
	Tcl_ListObjGetElements(interp, tcl_cmdv, &objc, &objv);
	Tcl_EvalObjv(interp, objc, objv, TCL_EVAL_GLOBAL);

out:
	Tcl_DecrRefCount(tcl_cmdv);
	return 0;
}

static int queue_command(Tcl_Obj *cmdv)
{
	/* runs a command asynchronously; usable from Python subthreads */
}

void process_queued_commands(void)
{
	PyGILState_STATE gst;
	gst = PyGILState_Ensure();
	
	PyGILState_Release(gst);
}


static PyObject * TclBridge_call(PyObject *self, PyObject *args)
{	
	run_command(args);
	return Py_None;
}


PyDoc_STRVAR(TclBridgeDoc,
	"TclBridge()\n\nGives Python access to the Eggdrop TCL interpreter");

PyTypeObject TclBridgeType = {
	PyObject_HEAD_INIT(NULL)
	0,                                        /* ob_size */
	"eggdrop.TclBridge",                      /* tp_name */
	sizeof(TclBridge),                        /* tp_basicsize */
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
	(hashfunc) PyObject_HashNotImplemented,   /* tp_hash */
	(ternaryfunc) TclBridge_call,             /* tp_call */
	0,                                        /* tp_str */
	PyObject_GenericGetAttr,                  /* tp_getattro */
	0,                                        /* tp_setattro */
	0,                                        /* tp_as_buffer */
	Py_TPFLAGS_HAVE_CLASS,                    /* tp_flags */
	TclBridgeDoc,                             /* tp_doc */
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

