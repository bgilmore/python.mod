#ifndef _API_H_
#define _API_H_

#include <Python.h>
#include "../module.h"

#define MODULE_NAME			"python"
#define MODULE_VER_MAJOR	0
#define MODULE_VER_MINOR	1
#define MAKING_PYTHON

#define API_METHOD			(PyObject *self, PyObject *args)

typedef uint32_t callback_id_t;
typedef struct {
	callback_id_t	id;
	PyThreadState	*subint;
	PyObject		*callable;
} callback_t;

PyObject * api_putlog API_METHOD;


#endif
