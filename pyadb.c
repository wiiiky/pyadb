/*
 * Copyright (C) 2015 Wiky L
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.";
 */
#include <Python.h>
#include "wrapper.h"


static PyObject *py_start_server(PyObject *self, PyObject *args, PyObject *keywds) {
    unsigned short port = DEFAULT_ADB_PORT;
    static char *kwlist[] = {"port", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "|H", kwlist, &port)) {
        return NULL;
    }
    if(!start_server(1, port)) {
        Py_RETURN_FALSE;
    }
    Py_RETURN_TRUE;
}

static PyObject *py_kill_server(PyObject *self, PyObject *args, PyObject *keywds) {
    unsigned short port = DEFAULT_ADB_PORT;
    static char *kwlist[] = {"port", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "|H", kwlist, &port)) {
        return NULL;
    }
    if(!kill_server(port)) {
        Py_RETURN_FALSE;
    }
    Py_RETURN_TRUE;
}

static PyMethodDef ADBMethods[] = {
    {
        "start_server", (PyCFunction)py_start_server, METH_VARARGS | METH_KEYWORDS,
        "启动adb守护进程"
    },
    {
        "kill_server", (PyCFunction)py_kill_server, METH_VARARGS | METH_KEYWORDS,
        "关闭adb守护进程"
    },
    {NULL, NULL, 0, NULL}
};


static struct PyModuleDef ADBModule = {
    PyModuleDef_HEAD_INIT,
    "pyadb",
    NULL,
    -1,
    ADBMethods
};

PyMODINIT_FUNC
PyInit_pyadb(void) {
    return PyModule_Create(&ADBModule);
}

