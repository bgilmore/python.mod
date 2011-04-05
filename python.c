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
#undef global
Function *global = NULL;

/*** configurables ***/

static char python_path[2048];

static tcl_strings tcl_stringtab[] = {
	{"python-path", python_path, 2048, 0},
	{NULL,          NULL,        0,    0}
};

static int python_isolate, python_isolated = -1;

static tcl_ints tcl_inttab[] = {
	{"python-isolate", &python_isolate},
	{NULL,             NULL}
};


/*** TCL api ***/

static int tcl_load_python STDVAR
{
	PyThreadState *subint;
	PyObject *modname, *module;

	// fix the value of 'python-isolate' for the lifetime of the module
	if (python_isolated == -1) {
		switch (python_isolate) {
			case 0:
				python_isolated = 0;
				break;

			case 1:
				python_isolated = 1;
				// create interp list
				break;

			default:
				Tcl_AppendResult(irp, "Invalid 'python-isolate' mode", NULL);
				return TCL_ERROR;
		}
	}

	if (python_isolated) {
		subint = Py_NewInterpreter();
		PyThreadState_Swap(subint);
	}

	modname = PyString_FromString(argv[1]);
	if (modname == NULL) {
		Tcl_AppendResult(irp, "Unable to parse module name", NULL);
		return TCL_ERROR;
	}

	module = PyImport_Import(modname);
	Py_DECREF(modname);

	if (module == NULL) {
		Tcl_AppendResult(irp, "Unable to load '", argv[1] ,"' module", NULL);
		return TCL_ERROR;
	}

	return TCL_OK;
}

static tcl_cmds tcl_commandtab[] = {
  {"loadpython", tcl_load_python},
  {NULL,         NULL}
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
	Py_Finalize();

	rem_tcl_commands(tcl_commandtab);
	rem_tcl_strings(tcl_stringtab);
	rem_tcl_ints(tcl_inttab);

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

static PyMethodDef api_table[] = {
    {"test", api_test, METH_VARARGS, NULL},
    {NULL, NULL, 0, NULL}
};

char * python_start(Function *global_funcs)
{
	global = global_funcs;

	memset(python_path, 0, 2048);
	python_isolate = 0;

	Py_InitializeEx(0);
	Py_InitModule("eggdrop", api_table);

	module_register(MODULE_NAME, python_table, \
			MODULE_VER_MAJOR, MODULE_VER_MINOR);

	if (!module_depend(MODULE_NAME, "eggdrop", 106, 0)) {
		module_undepend(MODULE_NAME);
		return "This module requires Eggdrop 1.6.0 or later.";
	}

	add_tcl_commands(tcl_commandtab);
	add_tcl_strings(tcl_stringtab);
	add_tcl_ints(tcl_inttab);
	
	return NULL;
}

