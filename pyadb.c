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
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fdevent.h>
#include <string.h>
#include "adb.h"


static PyObject *start_server(PyObject *self, PyObject *args){
    int pfds[2];
    if(pipe(pfds)){
        Py_RETURN_FALSE;
    }
    pid_t pid = fork();
    if(pid<0){
        close(pfds[0]);
        close(pfds[1]);
        Py_RETURN_FALSE;
    }else if(pid==0){
        close(pfds[0]);
        dup2(pfds[1], 2);
        dup2(pfds[0], 1);
        int r = adb_main(1, DEFAULT_ADB_PORT);
        close(pfds[1]);
        exit(r);
    }
    close(pfds[1]);
    char buf[16];
    int n = read(pfds[0], buf, sizeof(buf));
    close(pfds[0]);
    if(strncmp(buf,"OK",2)){
        Py_RETURN_FALSE;
    }
    Py_RETURN_TRUE;
}

static PyMethodDef ADBMethods[] = {
    {"start_server", start_server, METH_VARARGS},
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
PyInit_pyadb(void){
    return PyModule_Create(&ADBModule);
}

