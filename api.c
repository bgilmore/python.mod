#include "api.h"

extern Function *global;

PyObject * api_test(PyObject *self, PyObject *args)
{
	do_tcl("python.mod", "putlog \"hello world\"");
	return Py_None;
}

