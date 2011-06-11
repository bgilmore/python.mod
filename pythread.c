#include "python.h"
#include "pythread.h"

void * pythread_load(void *mod)
{
	struct module *self = mod;

	self->status = MODLOAD_SUCCESS;

	pthread_mutex_lock(&self->mtx);
	pthread_cond_signal(&self->loaded);
	pthread_mutex_unlock(&self->mtx);

	return NULL;
}

