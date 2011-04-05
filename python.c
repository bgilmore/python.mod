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

#define MODULE_NAME			"python"
#define MODULE_VER_MAJOR	0
#define MODULE_VER_MINOR	1
#define MAKING_PYTHON

#include "src/mod/module.h"

#undef global
static Function *global = NULL;

/*** housekeeping ***/

static int python_expmem(void)
{
	Context;
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
	Context;
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
	global = global_funcs;

	Context;

	module_register(MODULE_NAME, python_table, \
			MODULE_VER_MAJOR, MODULE_VER_MINOR);

	if (!module_depend(MODULE_NAME, "eggdrop", 106, 0)) {
		module_undepend(MODULE_NAME);
		return "This module requires Eggdrop 1.6.0 or later.";
	}
	
	return NULL;
}

