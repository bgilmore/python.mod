#ifndef _BRIDGE_H_
#define _BRIDGE_H_

#include <Python.h>

PyAPI_DATA(PyTypeObject) TclBridgeType;

typedef struct {
  PyObject_HEAD
  PyObject *attr;
} TclBridge;

void bridge_init(void);
void bridge_cleanup(void);

#endif
