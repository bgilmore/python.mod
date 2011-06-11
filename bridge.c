#include "bridge.h"
#include "pymod.h"

static unsigned int command_id = 0;
static PyObject *command_q  = NULL,
				*result_map = NULL;

static PyThreadState *main_thread;

static PyObject * run_command(PyObject *py_cmdv)
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
	return Py_None;
}

static PyObject * run_queued(PyObject *py_cmdv)
{
	PyObject *command, *result;

	command = Py_BuildValue("(IO)", command_id, py_cmdv);
	command_id++;

	fprintf(stderr, "appending...\n");
	if (command == NULL || PyList_Append(command_q, command) != 0) {
		PyErr_SetString(PyExc_RuntimeError, "Unable to queue command!");
		return NULL;
	}

	fprintf(stderr, "waiting....\n");
	while (PySequence_Contains(command_q, command) == 1 || 
			PyDict_Contains(result_map, PyTuple_GET_ITEM(command, 0)) == 0) {
		/* TODO: add deadlock prevention, timeout */
		Py_BEGIN_ALLOW_THREADS
		fprintf(stderr, "doot!\n");
		usleep(333333);
		Py_END_ALLOW_THREADS
	}

	fprintf(stderr, "fetching.....\n");
	result = PyDict_GetItem(result_map, PyTuple_GET_ITEM(command, 0));
	PyDict_DelItem(result_map, PyTuple_GET_ITEM(command, 0));
	Py_DECREF(command);

	return result;
}

static void drain_command_q(void)
{
	PyGILState_STATE gst;
	PyObject *command, *result;
	Py_ssize_t i, j;

	gst = PyGILState_Ensure();
	j = PyList_GET_SIZE(command_q);

	fprintf(stderr, "** TRACE: drain queue (depth=%zu)\n", j);

	for (i = 0; i < j; i++) {
		command = PyList_GET_ITEM(command_q, i);
		result = run_command(PyTuple_GET_ITEM(command, 1));
		
		PyDict_SetItem(result_map, PyTuple_GET_ITEM(command, 0), result);
	}
	
	if (j > 0) {
		fprintf(stderr, "Drained %zu items\n", j);
		Py_DECREF(command_q);      /* GC the old queue */
		command_q = PyList_New(0); /* create a new, empy queue */
	}
	
	PyGILState_Release(gst);
}


static PyObject * TclBridge_call(PyObject *self, PyObject *args)
{	
	if (PyThreadState_Get() == main_thread) {
		fprintf(stderr, "*** RUNNING COMMAND (%u) ***\n", pthread_self());
		return run_command(args);
	} else {
		fprintf(stderr, "*** PUTTING CMD IN Q (%u) ***\n", pthread_self());
		return run_queued(args);
	}
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

void bridge_init(void)
{
	PyEval_InitThreads();
	main_thread = PyThreadState_Get();

	TclBridgeType.ob_type = &PyType_Type;
	TclBridgeType.tp_new = PyType_GenericNew;
	PyType_Ready(&TclBridgeType);

	command_q  = PyList_New(0);
	result_map = PyDict_New();

	add_hook(HOOK_SECONDLY, (Function) drain_command_q);
}

void bridge_cleanup(void)
{
	del_hook(HOOK_SECONDLY, (Function) drain_command_q);
}

