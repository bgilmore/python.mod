#ifndef _PYMOD_H_
#define _PYMOD_H_

#include <Python.h>
#include "../module.h"

#define MODULE_NAME			"python"
#define MODULE_VER_MAJOR	0
#define MODULE_VER_MINOR	1
#define MAKING_PYTHON

/* replace "global" macro with pointer initialized in python_init */
#undef global
Function *global;

#endif
