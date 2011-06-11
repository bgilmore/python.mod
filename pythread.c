#include "python.h"
#include "pythread.h"

static void * pythread_runloop(struct module *self)
{
	while(1) {
		sleep(60);
	}

	return NULL;
}

void * pythread_load(void *arg)
{
	PyObject *modname, *module;
	struct module *self = arg;

	self->irp = Py_NewInterpreter();
	self->gst = PyGILState_Ensure();
	PyThreadState_Swap(self->irp);

	modname = PyString_FromString(self->name);
	if (modname == NULL) {
		if (PyErr_Occurred() != NULL)
			PyErr_PrintEx(0);

		self->status = MODLOAD_FAIL;
		goto out;
	}

	module = PyImport_Import(modname);
	Py_DECREF(modname);

	if (module == NULL) {
		if (PyErr_Occurred() != NULL)
			PyErr_PrintEx(0);

		self->status = MODLOAD_FAIL;
		goto out;
	}

	self->status = MODLOAD_SUCCESS;
	
out:
	pthread_mutex_lock(&self->mtx);
	pthread_cond_signal(&self->loaded);
	pthread_mutex_unlock(&self->mtx);

	PyGILState_Release(self->gst);
	return pythread_runloop(self);
}


