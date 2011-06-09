#ifndef _BRIDGE_H_
#define _BRIDGE_H_

#include <Python.h>

PyTypeObject TclBridgeType;

typedef struct {
  PyObject_HEAD
  PyObject *attr;
} TclBridge;

#endif
