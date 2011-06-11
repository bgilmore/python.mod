#ifndef _PYMOD_H_
#define _PYMOD_H_

#include <pthread.h>
#include <python2.6/Python.h>
#include <sys/queue.h>

#include "../module.h"

#define MODULE_NAME			"python"
#define MODULE_VER_MAJOR	0
#define MODULE_VER_MINOR	1
#define MAKING_PYTHON

/* replace "global" macro with pointer initialized in python_init */
#undef global
Function *global;

/* module accounting */
#define MODLOAD_FAIL		0
#define MODLOAD_SUCCESS		1

struct module {
	char                  *name;
	uint8_t               status;

	PyThreadState         *irp;
	PyGILState_STATE      gst;

	pthread_t             thread;
	pthread_mutex_t       mtx;
	pthread_cond_t        loaded;

	TAILQ_ENTRY(module)   tq;
};

#endif
