#ifndef _PYMOD_H_
#define _PYMOD_H_

#include <Python.h>
#include "khash.h"

typedef uint32_t pymod_id_t;
typedef struct {
	pymod_id_t		id;
	PyThreadState	*subint;
	PyObject		*module;
} pymod_t;

/* khash needs access to stdlib */
#undef malloc
#undef free
KHASH_MAP_INIT_INT(pymods, pymod_t);

#endif
