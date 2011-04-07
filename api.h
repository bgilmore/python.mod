#ifndef _API_H_
#define _API_H_

#include <Python.h>
#include "../module.h"

#define MODULE_NAME			"python"
#define MODULE_VER_MAJOR	0
#define MODULE_VER_MINOR	1
#define MAKING_PYTHON

typedef uint32_t callback_id_t;
typedef struct {
	callback_id_t	id;
	PyThreadState	*subint;
	PyObject		*callable;
} callback_t;

PyObject * api_test(PyObject *self, PyObject *args);

#endif
