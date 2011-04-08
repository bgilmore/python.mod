#ifndef _API_H_
#define _API_H_

#include <Python.h>
#include "khash.h"
#include "../module.h"

#define MODULE_NAME			"python"
#define MODULE_VER_MAJOR	0
#define MODULE_VER_MINOR	1
#define MAKING_PYTHON

#define APIDEF_METHOD(symbol)	static PyObject * api_##symbol(PyObject *self, PyObject *args)
#define APIDEF_KWMETHOD(symbol)	static PyObject * api_##symbol(PyObject *self, PyObject *args, PyObject *kw)

#define API_METHOD(symbol)		{#symbol, (PyCFunction)api_##symbol, METH_VARARGS}
#define API_KWMETHOD(symbol)	{#symbol, (PyCFunction)api_##symbol, METH_VARARGS|METH_KEYWORDS}
#define API_END					{NULL}

#define API_ACTIVE()		api_available = 1
#define API_INACTIVE()		api_available = 0
#define API_CHECK()			if (api_available == 0) { \
								PyErr_SetString(PyExc_RuntimeError, "eggdrop methods may only be used at load time and in event handlers"); \
								return NULL; \
							}

typedef uint32_t callback_id_t;
typedef struct {
	callback_id_t	id;
	char			*name;
	PyThreadState	*subint;
	PyObject		*callable;
} callback_t;

/* khash needs access to stdlib */
#undef malloc
#undef free
KHASH_MAP_INIT_INT(callbacks, callback_t);

#endif
