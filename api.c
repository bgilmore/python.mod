#include "api.h"

PyObject * api_version(PyObject *self, PyObject *args)
{
	return Py_BuildValue("(i,i)", MODULE_VER_MAJOR, MODULE_VER_MINOR);
}

