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

#include "api.h"
#include "bridge.h"
#include "pymod.h"
#include "../module.h"

#undef global
static Function *global = NULL;
extern uint8_t api_available;

extern khash_t(callbacks) *callback_table;
static khash_t(pymods) *pymod_table;
static uint32_t pymod_base_id = 0;

extern PyMethodDef api_table[];
static Tcl_Namespace *ns = NULL;

/*** configurables ***/

static char python_path[2048];

static tcl_strings tcl_stringtab[] = {
	{"python-path", python_path, 2048, 0},
	{NULL,          NULL,        0,    0}
};

int python_isolate, python_isolated = -1;

static tcl_ints tcl_inttab[] = {
	{"python-isolate", &python_isolate},
	{NULL,             NULL}
};


/*** TCL api ***/

static int tcl_load_python STDVAR
{
	PyThreadState *subint = NULL;
	PyObject *modname, *module;
	pymod_t *pymod = NULL;
	uint32_t pymod_id;
	int32_t k, r;

	API_ACTIVE();

	// fix the value of 'python-isolate' for the lifetime of the module
	if (python_isolated == -1) {
		switch (python_isolate) {
			case 0:
				python_isolated = 0;
				break;

			case 1:
				python_isolated = 1;
				break;

			default:
				Tcl_AppendResult(irp, "Invalid 'python-isolate' mode", NULL);
				goto err;
		}
	}

	if (python_isolated) {
		subint = Py_NewInterpreter();
		PyThreadState_Swap(subint);
		Py_InitModule("eggdrop", api_table);
	}

	modname = PyString_FromString(argv[1]);
	if (modname == NULL) {
		Tcl_AppendResult(irp, "Unable to parse module name", NULL);
		goto err;
	}

	module = PyImport_Import(modname);
	Py_DECREF(modname);

	if (module == NULL) {
		if (PyErr_Occurred() != NULL)
			PyErr_PrintEx(0);

		Tcl_AppendResult(irp, "Unable to load '", argv[1] ,"' module", NULL);
		goto err;
	}

	pymod_id = pymod_base_id++;
	k = kh_put(pymods, pymod_table, pymod_id, &r);
	pymod = &(kh_val(pymod_table, k));

	pymod->id = pymod_id;
	pymod->subint = subint;
	pymod->module = module;
	
	API_INACTIVE();
	return TCL_OK;

err:
	if (subint != NULL)
		Py_EndInterpreter(subint);

	API_INACTIVE();
	return TCL_ERROR;
}

static tcl_cmds tcl_commandtab[] = {
  {"loadpython",       tcl_load_python},
  {NULL,               NULL}
};


/*** housekeeping ***/

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
	kh_destroy(pymods, pymod_table);
	kh_destroy(callbacks, callback_table);

	rem_tcl_commands(tcl_commandtab);
	rem_tcl_strings(tcl_stringtab);
	rem_tcl_ints(tcl_inttab);
	Tcl_DeleteNamespace(ns);

	Py_Finalize();

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
	PyObject *api = NULL,
			 *mod = NULL;

	/* fixup external linkage */
	global = global_funcs;

	module_register(MODULE_NAME, python_table, \
			MODULE_VER_MAJOR, MODULE_VER_MINOR);

	if (!module_depend(MODULE_NAME, "eggdrop", 106, 0)) {
		module_undepend(MODULE_NAME);
		return "This module requires Eggdrop 1.6.0 or later.";
	}

	memset(python_path, 0, 2048);
	python_isolate = 0;

	Py_InitializeEx(0);
	mod = Py_InitModule("eggdrop", api_table);
	api = _PyObject_New(&TclBridgeType);
    PyModule_AddObject(mod, "api", api);

	ns = Tcl_CreateNamespace(interp, "python", NULL, NULL);
	add_tcl_commands(tcl_commandtab);
	add_tcl_strings(tcl_stringtab);
	add_tcl_ints(tcl_inttab);

	pymod_table = kh_init(pymods);
	callback_table = kh_init(callbacks);
	
	return NULL;
}

