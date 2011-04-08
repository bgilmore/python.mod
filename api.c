#include <stdarg.h>
#include "api.h"

extern Function *global;

PyObject * api_bind API_METHOD
{
	PyObject *callable;
	char *type, *flags, *parm;
	PyArg_ParseTuple(args, "sssO", &type, &flags, &parm, &callable);

	fprintf(stderr, "bind request for type=%s, flags=%s, parm=%s\n", type, flags, parm);

	return Py_None;
}

PyObject * api_putlog API_METHOD
{
	char *message;
	PyArg_ParseTuple(args, "s", &message);

	Tcl_SetVar(interp, "python::arg_msg", message, 0);
	Tcl_Eval(interp, "putlog $python::arg_msg");
	Tcl_UnsetVar(interp, "python::arg_msg", 0);

	return Py_None;
}

PyMethodDef api_table[] = {
    {"bind", api_bind, METH_VARARGS, NULL},
    {"putlog", api_putlog, METH_VARARGS, NULL},
    {NULL, NULL, 0, NULL}
};

