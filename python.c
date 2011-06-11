/* Copyright (c) 2011, Brandon Gilmore <brandon@mg2.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of the software nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "python.h"
#include "pythread.h"

TAILQ_HEAD(mh, module) modules;

static void dealloc_module(struct module *mod)
{
	nfree(mod->name);

	Py_EndInterpreter(mod->irp);
	pthread_mutex_destroy(&mod->mtx);
	pthread_cond_destroy(&mod->loaded);

	nfree(mod);
}

/*** TCL api ***/

static int tcl_loadmodule_python STDVAR
{
	struct module *mod;

	mod = (struct module *) nmalloc(sizeof(struct module));
	if (mod == NULL) {
		dprintf(DP_LOG, "*** allocation error ***");
		return TCL_ERROR;
	}

	memset(mod, 0, sizeof(struct module));
	mod->name = strdup(argv[1]);

	pthread_mutex_init(&mod->mtx, NULL);
	pthread_cond_init(&mod->loaded, NULL);

	pthread_mutex_lock(&mod->mtx);
	pthread_create(&mod->thread, NULL, pythread_load, (void *) mod);
	pthread_cond_wait(&mod->loaded, &mod->mtx);

	if (mod->status != MODLOAD_SUCCESS) {
		dealloc_module(mod);
		Tcl_AppendResult(irp, "Unable to load '", argv[1], "' module", NULL);
		return TCL_ERROR;
	} else {
		TAILQ_INSERT_TAIL(&modules, mod, tq);
		return TCL_OK;
	}
}

static tcl_cmds tcl_commandtab[] = {
  {"loadmodule.py",    tcl_loadmodule_python},
  {NULL,               NULL}
};


/*** module interface ***/

static int python_expmem(void)
{
	return 0;
}

static void python_report(int idx, int details)
{
	if (details) {
		dprintf(idx, "    Memory usage not instrumented\n");
	}
}

static char * python_close()
{
	rem_tcl_commands(tcl_commandtab);

	module_undepend(MODULE_NAME);
	return NULL;
}


/*** module initialization ***/

EXPORT_SCOPE char * python_start();

static Function python_table[] = {
	(Function) python_start,
	(Function) python_close,
	(Function) python_expmem,
	(Function) python_report,
};

char * python_start(Function *global_funcs)
{
	/* fixup external linkage */
	global = global_funcs;

	TAILQ_INIT(&modules);

	module_register(MODULE_NAME, python_table, \
			MODULE_VER_MAJOR, MODULE_VER_MINOR);

	if (!module_depend(MODULE_NAME, "eggdrop", 106, 0)) {
		module_undepend(MODULE_NAME);
		return "This module requires Eggdrop 1.6.0 or later.";
	}

	add_tcl_commands(tcl_commandtab);

	Py_InitializeEx(0);
	PyEval_InitThreads();

	return NULL;
}

