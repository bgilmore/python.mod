#ifndef _API_H_
#define _API_H_

#include <Python.h>
#include "../module.h"

#define MODULE_NAME			"python"
#define MODULE_VER_MAJOR	0
#define MODULE_VER_MINOR	1
#define MAKING_PYTHON

PyObject * api_test(PyObject *self, PyObject *args);

#endif
