#ifndef _PYMOD_H_
#define _PYMOD_H_

#include <Python.h>
#include "khash.h"
#include "../module.h"

#define MODULE_NAME			"python"
#define MODULE_VER_MAJOR	0
#define MODULE_VER_MINOR	1
#define MAKING_PYTHON

/* replace "global" macro with pointer initialized in python_init */
#undef global
Function *global;

/* khash needs access to stdlib */
#undef malloc
#undef free

/* module table */
typedef uint32_t pymod_id_t;
typedef struct {
	pymod_id_t		id;
	PyThreadState	*subint;
	PyObject		*module;
} pymod_t;

KHASH_MAP_INIT_INT(pymods, pymod_t);

#endif
