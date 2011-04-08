#include "api.h"

extern Function *global;
extern int python_isolated;

khash_t(callbacks) *callback_table;
static uint32_t callback_base_id = 0;

/* This flag is used to mark when the Eggdrop TCL API is usable by our
 * interpreter threads since the main runloop isn't inherently thread-safe */
uint8_t api_available = 0;

/*** callback plumbing ***/

static int run_callback STDVAR
{
	/* TODO: 
	 * - needs a return value for certain binds
	 * - null checking everywhere
	 */

	PyObject *tmp, *args, *ret = NULL;
	callback_t *callback;
	uint32_t callback_id;
	int32_t i, k;

	API_ACTIVE();

	callback_id = strtol(argv[0]+17, NULL, 10);
	if (errno == EINVAL) {
		Tcl_AppendResult(irp, "Invalid callback ID", NULL);
		goto err;
	}

	k = kh_get(callbacks, callback_table, callback_id);
	callback = &(kh_val(callback_table, k));
	
	if (callback->subint != NULL) {
		PyThreadState_Swap(callback->subint);
	}

	args = PyTuple_New(argc - 1);
	for (i = 1; i < argc; i++) {
		tmp = PyString_FromString(argv[i]);
		PyTuple_SET_ITEM(args, i - 1, tmp);
	}

	ret = PyObject_Call(callback->callable, args, NULL);
	Py_DECREF(args);

	if (PyErr_Occurred() != NULL) {
		Tcl_AppendResult(irp, "Uncaught Python exception", NULL);
		PyErr_PrintEx(0);
		goto err;
	} 

	if ((ret != Py_None) && (ret != NULL)) {
		tmp = PyObject_Str(ret);
		Py_DECREF(ret);

		if (tmp == NULL) {
			Tcl_AppendResult(irp, "Invalid return value", NULL);
			goto err;
		} else {
			Tcl_AppendResult(irp, PyString_AS_STRING(tmp), NULL);
			Py_DECREF(tmp);
		}
	}

	API_INACTIVE();
	return TCL_OK;

err:
	API_INACTIVE();
	return TCL_ERROR;
}

static callback_t * install_callback(PyObject *callable)
{
	callback_t *callback;
	tcl_cmds *cmdtab;
	uint32_t callback_id;
	int32_t k, r;

	callback_id = callback_base_id++;

	k = kh_put(callbacks, callback_table, callback_id, &r);
	callback = &(kh_val(callback_table, k));

	callback->id = callback_id;
	callback->name = (char *) nmalloc(sizeof(char) * 28);
	snprintf(callback->name, 28, "python::callback_%u", callback_id);

	callback->callable = callable;
	Py_INCREF(callable);

	if (python_isolated)
		callback->subint = PyThreadState_Get();
	else
		callback->subint = NULL;

	cmdtab = (tcl_cmds *) nmalloc(sizeof(tcl_cmds) * 2);
	cmdtab[0].name = callback->name;
	cmdtab[0].func = run_callback;
	cmdtab[1].name = NULL;
	cmdtab[2].name = NULL;
	add_tcl_commands(cmdtab);
	nfree(cmdtab);

	return callback;
}


/*** TCL bridge ***/

API_METHOD(bind)
{
	PyObject *callable;
	callback_t *callback;
	char *type, *flags, *parm, *bind;

	API_CHECK();

	PyArg_ParseTuple(args, "sssO", &type, &flags, &parm, &callable);

	callback = install_callback(callable);

	bind = (char *) nmalloc(sizeof(char) * (60 + strlen(callback->name)));
	sprintf(bind,
	    "bind $python::arg_type $python::arg_flags $python::arg_parm %s",
	    callback->name);

	Tcl_SetVar(interp, "python::arg_type", type, 0);
	Tcl_SetVar(interp, "python::arg_flags", flags, 0);
	Tcl_SetVar(interp, "python::arg_parm", parm, 0);
	Tcl_Eval(interp, bind);
	Tcl_UnsetVar(interp, "python::arg_type", 0);
	Tcl_UnsetVar(interp, "python::arg_flags", 0);
	Tcl_UnsetVar(interp, "python::arg_parm", 0);

	nfree(bind);

	return Py_None;
}

API_METHOD(putlog)
{
	char *message;

	API_CHECK();

	PyArg_ParseTuple(args, "s", &message);
	Tcl_SetVar(interp, "python::arg_msg", message, 0);
	Tcl_Eval(interp, "putlog $python::arg_msg");
	Tcl_UnsetVar(interp, "python::arg_msg", 0);

	return Py_None;
}

PyMethodDef api_table[] = {
	API_ENTRY(bind),
	API_ENTRY(putlog),
	API_END
};

