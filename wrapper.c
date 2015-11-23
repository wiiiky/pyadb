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
#include "wrapper.h"
#include "util.h"
#include "adb_client.h"
#include "adb_auth.h"
#include "usb_vendors.h"
// #include "adb_trace.h"
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>


static unsigned short adb_port = 5544;

static void *fdevent_loop_thread(void *data) {
    fdevent_loop();
}

int adb_init(unsigned short port) {
    atexit(usb_cleanup);
    signal(SIGPIPE, SIG_IGN);

    adb_port = port;

    init_transport_registration();
    usb_vendors_init();
    usb_init();
    local_init(DEFAULT_ADB_LOCAL_TRANSPORT_PORT);
    adb_auth_init();

    char local_name[30];
    build_local_name(local_name, sizeof(local_name), adb_port);
    if(install_listener(local_name, "*smartsocket*", NULL, 0)) {
        return 0;
    }
    pthread_t tid;
    if(pthread_create(&tid, NULL, fdevent_loop_thread,NULL)) {
        return 0;
    }


    return 1;
}

void adb_devices(char *buffer, unsigned int size, int use_long) {
    memset(buffer, 0, size);
    list_transports(buffer, size, use_long);
}

void adb_list_forward(char *buffer, unsigned int size) {
    memset(buffer, 0, size);
    format_listeners(buffer, size);
}

const char *adb_create_forward(unsigned short local, unsigned short remote,
                               transport_type ttype, const char* serial,
                               int no_rebind) {
    char *err;
    atransport *transport = acquire_one_transport(CS_ANY, ttype, serial, &err);
    if (!transport) {
        return err;
    }

    char local_name[32];
    char remote_name[32];
    snprintf(local_name, sizeof(local_name), "tcp:%u", local);
    snprintf(remote_name, sizeof(remote_name), "tcp:%u", remote);
    int r = install_listener(local_name, remote_name, transport, no_rebind);
    if(r == 0) {
        return "OKAY";
    }

    const char* message;
    switch (r) {
    case INSTALL_STATUS_CANNOT_BIND:
        message = "cannot bind to socket";
        break;
    case INSTALL_STATUS_CANNOT_REBIND:
        message = "cannot rebind existing socket";
        break;
    default:
        message = "internal error";
    }
    return message;
}
