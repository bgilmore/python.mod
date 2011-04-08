#include <stdarg.h>
#include "api.h"

extern Function *global;

static char * run_script(const char *fmt, ...)
{
	va_list argp;
	const char *src;
	char *dst, *chunk, *script = NULL;
	char buf[256];
	uint32_t bufsz, chunksz;

	va_start(argp, fmt);

	bufsz = strlen(fmt) * 2;
	script = (char *) nmalloc(bufsz + 1);
	if (script == NULL)
		return NULL;
	else
		dst = script;

	for (src = fmt; *src; src++, dst++) {
		if (*src != '%') {
			*dst = *src; // copy source bytes to dest
			continue;
		}
		
		switch(*++src)
		{
			case 's':
				chunk = va_arg(argp, char *);
				break;

			case 'd':
				snprintf(buf, 255, "%d", va_arg(argp, int));
				chunk = buf;
				break;

			case '%':
				chunk = "%";
				break;

			default:
				return NULL;
		}


		while (bufsz - (dst - script) < strlen(chunk)) {
			bufsz *= 2;
			nrealloc(script, bufsz + 1);
		}

		chunksz = strlen(chunk);
		strncpy(dst, chunk, chunksz);
		dst += chunksz;
	}

	fprintf(stderr, "generated: [%s]\n", script);
	return script;
}
		

PyObject * api_putlog API_METHOD
{
	char *escaped, *str, *script;

	PyArg_ParseTuple(args, "s", &str);

	escaped = str_escape(str, '"', '\\');
	/*script = (char *) nmalloc(strlen(escaped) + 10);
	sprintf(script, "putlog \"%s\"", escaped);*/
	script = run_script("putlog \"%s\"", escaped);
	nfree(escaped);

	do_tcl("python.mod", script);
	nfree(script);

	return Py_None;
}

