#include <stdarg.h>
#include "api.h"

extern Function *global;

PyObject * api_putlog API_METHOD
{
	char *message;
	PyArg_ParseTuple(args, "s", &message);

	Tcl_SetVar(interp, "python::arg_msg", message, 0);
	Tcl_Eval(interp, "putlog $python::arg_msg");
	Tcl_UnsetVar(interp, "python::arg_msg", 0);

	return Py_None;
}

