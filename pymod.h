#ifndef _PYMOD_H_
#define _PYMOD_H_

#include <Python.h>

typedef uint32_t pymod_id_t;
typedef struct {
	pymod_id_t		id;
	PyThreadState	*subint;
	PyObject		*module;
} pymod_t;

//typedef struct _pymod pymod_t;

#endif
