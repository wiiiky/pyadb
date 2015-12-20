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

static PyObject *pdb_init(PyObject *self, PyObject *args, PyObject *keywds) {
    static char *kwlist[] = {"port", NULL};
    int port=7305;
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "|H", kwlist, &port)) {
        return NULL;
    }
    if(adb_init(port)) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyObject *pdb_devices(PyObject *self, PyObject *args, PyObject *keywds) {
    static char *kwlist[] = {"use_long", NULL};
    int use_long=0;
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "|p", kwlist, &use_long)) {
        return NULL;
    }
    char buf[4096];
    adb_devices(buf, sizeof(buf), use_long);
    return Py_BuildValue("s", buf);
}

static PyObject *pdb_list_forward(PyObject *self) {
    char buf[4096];
    adb_list_forward(buf, sizeof(buf));
    return Py_BuildValue("s", buf);
}

static PyObject *pdb_create_forward(PyObject *self, PyObject *args, PyObject *keywds) {
    static char *kwlist[] = {"local", "remote","ttype", "serial", "no_rebind", NULL};
    unsigned short local, remote;
    int ttype=kTransportAny;
    char *serial=NULL;
    int no_rebind=0;
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "HH|isp", kwlist,
                                     &local, &remote,&ttype, &serial,
                                     &no_rebind)) {
        return NULL;
    }
    if(adb_create_forward(local, remote, ttype, serial, no_rebind)) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyObject *pdb_remove_forward(PyObject *self, PyObject *args, PyObject *keywds) {
    static char *kwlist[] = {"local", "ttype", "serial", NULL};
    unsigned short local;
    int ttype=kTransportAny;
    char *serial=NULL;
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "H|is", kwlist,
                                     &local, &ttype, &serial)) {
        return NULL;
    }
    if(adb_remove_forward(local, ttype, serial)) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyObject *pdb_remove_forward_all(PyObject *self) {
    if(adb_remove_forward_all()) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyObject *pdb_install_apk(PyObject *self, PyObject *args, PyObject *keywds) {
    static char *kwlist[] = {"file", "ttype", "serial", "reinstall","sdcard", NULL};
    int ttype=kTransportAny;
    char *serial=NULL;
    char *file=NULL;
    int reinstall = 0;
    int sdcard = 0;
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "s|ispp", kwlist,
                                     &file, &ttype, &serial,
                                     &reinstall, &sdcard)) {
        return NULL;
    }
    if(adb_install_app(ttype, serial, file, reinstall, sdcard)) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyObject *pdb_uninstall_apk(PyObject *self, PyObject *args, PyObject *keywds) {
    static char *kwlist[] = {"package", "ttype", "serial", NULL};
    int ttype=kTransportAny;
    char *serial=NULL;
    char *package=NULL;
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "s|is", kwlist,
                                     &package, &ttype, &serial)) {
        return NULL;
    }
    if(adb_uninstall_app(ttype, serial, package)) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyObject *pdb_start_activity(PyObject *self, PyObject *args, PyObject *keywds) {
    static char *kwlist[] = {"activity", "ttype", "serial", NULL};
    int ttype=kTransportAny;
    char *serial=NULL;
    char *activity=NULL;
    if (!PyArg_ParseTupleAndKeywords(args, keywds, "s|is", kwlist,
                                     &activity, &ttype, &serial)) {
        return NULL;
    }
    if(adb_start_activity(ttype, serial, activity)) {
        Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
}

static PyMethodDef PDBMethods[] = {
    {
        "pdb_init", (PyCFunction)pdb_init, METH_VARARGS | METH_KEYWORDS,
        "初始化adb，这会创建两个线程，一个线程监听USB设备，一个线程作为ADB消息主循环"
    },
    {
        "pdb_devices", (PyCFunction)pdb_devices, METH_VARARGS | METH_KEYWORDS,
        "返回当前连接的设备"
    },
    {
        "pdb_list_forward", (PyCFunction)pdb_list_forward, METH_NOARGS,
        "返回当前重定向列表"
    },
    {
        "pdb_create_forward", (PyCFunction)pdb_create_forward, METH_VARARGS |METH_KEYWORDS,
        "创建一个TCP重定向"
    },
    {
        "pdb_remove_forward", (PyCFunction)pdb_remove_forward, METH_VARARGS |METH_KEYWORDS,
        ""
    },
    {
        "pdb_remove_forward_all", (PyCFunction)pdb_remove_forward_all, METH_NOARGS,
        ""
    },
    {
        "pdb_install_apk", (PyCFunction)pdb_install_apk, METH_VARARGS | METH_KEYWORDS,
        ""
    },
    {
        "pdb_uninstall_apk", (PyCFunction)pdb_uninstall_apk, METH_VARARGS | METH_KEYWORDS,
        ""
    },
    {
        "pdb_start_activity", (PyCFunction)pdb_start_activity, METH_VARARGS|METH_KEYWORDS,
        "",
    },
    {NULL, NULL, 0, NULL}
};


static struct PyModuleDef PDBModule = {
    PyModuleDef_HEAD_INIT,
    "pdb",
    NULL,
    -1,
    PDBMethods
};

PyMODINIT_FUNC PyInit_pyadb(void) {
    PyObject *m = PyModule_Create(&PDBModule);
    PyObject_SetAttrString(m, "kTransportAny", Py_BuildValue("i", kTransportAny));
    PyObject_SetAttrString(m, "kTransportUsb", Py_BuildValue("i", kTransportUsb));
    PyObject_SetAttrString(m, "kTransportLocal",Py_BuildValue("i", kTransportLocal));
    PyObject_SetAttrString(m, "kTransportHost", Py_BuildValue("i", kTransportHost));
    return m;
}

