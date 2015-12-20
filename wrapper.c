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
#include "adb_client.h"
#include "adb_auth.h"
#include "usb_vendors.h"
#include "sysdeps.h"
// #include "adb_trace.h"
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>


static unsigned short adb_port = 7305;

static void *fdevent_loop_thread(void *data) {
    fdevent_loop();
    return NULL;
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

    adb_set_tcp_specifics(adb_port);
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

int adb_create_forward(unsigned short local, unsigned short remote,
                       transport_type ttype, const char* serial,
                       int no_rebind) {
    char *err;
    atransport *transport = acquire_one_transport(CS_ANY, ttype, serial, &err);
    if (!transport) {
        return 1;
    }

    char local_name[32];
    char remote_name[32];
    snprintf(local_name, sizeof(local_name), "tcp:%u", local);
    snprintf(remote_name, sizeof(remote_name), "tcp:%u", remote);
    int r = install_listener(local_name, remote_name, transport, no_rebind);
    return r==0;
}

int adb_remove_forward(unsigned short local, transport_type ttype,
                       const char* serial) {
    char *err;
    atransport *transport = acquire_one_transport(CS_ANY, ttype, serial, &err);
    if(!transport) {
        return 1;
    }
    char local_name[32];
    snprintf(local_name, sizeof(local_name), "tcp:%u", local);
    int r = remove_listener(local_name, transport);
    return r==0;
}

int adb_remove_forward_all(void) {
    remove_all_listeners();
    return 1;
}


static int _do_sync_push(const char *lpath, const char *rpath, int show_progress) {
    struct stat st;
    unsigned mode;
    int fd;

    if((fd = _adb_connect("sync:")) < 0) {
        return 1;
    }

    if(stat(lpath, &st)) {
        sync_quit(fd);
        return 2;
    }

    if(S_ISDIR(st.st_mode)) {
        if(copy_local_dir_remote(fd, lpath, rpath, 0, 0)) {
            return 3;
        } else {
            sync_quit(fd);
        }
    } else {
        if(sync_readmode(fd, rpath, &mode)) {
            return 4;
        }
        if((mode != 0) && S_ISDIR(mode)) {
            /* if we're copying a local file to a remote directory,
            ** we *really* want to copy to remotedir + "/" + localfilename
            */
            const char *name = strrchr(lpath, '/');
            if(name == 0) {
                name = lpath;
            } else {
                name++;
            }
            int  tmplen = strlen(name) + strlen(rpath) + 2;
            char *tmp = malloc(strlen(name) + strlen(rpath) + 2);
            if(tmp == 0) return 1;
            snprintf(tmp, tmplen, "%s/%s", rpath, name);
            rpath = tmp;
        }
        if(sync_send(fd, lpath, rpath, st.st_mtime, st.st_mode, show_progress)) {
            return 5;
        } else {
            sync_quit(fd);
            return 0;
        }
    }

    return 0;
}

static char *_send_shellcommand(transport_type transport, char* serial, char* cmd) {
    int fd;

    if((fd=_adb_connect(cmd))<0) {
        return NULL;
    }

    char buf[4096<<2];
    char *ptr=buf;
    int left=sizeof(buf);
    int len = 0;
    while(left>0) {
        int n = adb_read(fd, ptr, left);
        if(n>0) {
            ptr += n;
            left -= n;
            len += n;
        } else if(n<=0) {
            break;
        }
    }
    adb_close(fd);
    return strndup(buf, len);
}

static int _delete_file(transport_type transport, char* serial, char* filename) {
    char buf[4096];
    char* quoted;

    snprintf(buf, sizeof(buf), "shell:rm -f ");
    quoted = escape_arg(filename);
    strncat(buf, quoted, sizeof(buf)-1);
    free(quoted);

    char *result = _send_shellcommand(transport, serial, buf);
    int ret = -1;
    if(result&&strstr(result, "OKAY")) {
        ret = 0;
    }
    free(result);
    return ret;
}

static char* _pm_command(transport_type transport, char* serial,
                         int argc, char** argv) {
    char buf[4096];

    snprintf(buf, sizeof(buf), "shell:pm");

    while(argc-- > 0) {
        char *quoted = escape_arg(*argv++);
        if(quoted[0]!='\"') {
            strncat(buf, " ", sizeof(buf) - 1);
            strncat(buf, quoted, sizeof(buf) - 1);
        }
        free(quoted);
    }

    return _send_shellcommand(transport, serial, buf);
}

static char *_am_command(transport_type transport, char *serial,
                         int argc, char **argv) {
    char buf[4096];

    snprintf(buf, sizeof(buf), "shell:am");

    while(argc-- > 0) {
        char *quoted = escape_arg(*argv++);
        if(quoted[0]!='\"') {
            strncat(buf, " ", sizeof(buf) - 1);
            strncat(buf, quoted, sizeof(buf) - 1);
        }
        free(quoted);
    }

    return _send_shellcommand(transport, serial, buf);
}


static int _install_app(transport_type transport, char* serial, int argc, char** argv) {
    static const char *const DATA_DEST = "/data/local/tmp/%s";
    static const char *const SD_DEST = "/sdcard/tmp/%s";
    const char* where = DATA_DEST;
    int i;
    struct stat sb;

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-s")) {
            where = SD_DEST;
        }
    }

    // Find last APK argument.
    // All other arguments passed through verbatim.
    int last_apk = -1;
    for (i = argc - 1; i >= 0; i--) {
        char* file = argv[i];
        char* dot = strrchr(file, '.');
        if (dot && !strcasecmp(dot, ".apk")) {
            if (stat(file, &sb) == -1 || !S_ISREG(sb.st_mode)) {
                return -1;
            }

            last_apk = i;
            break;
        }
    }

    if (last_apk == -1) {
        return -1;
    }

    char* apk_file = argv[last_apk];
    char apk_dest[PATH_MAX];
    snprintf(apk_dest, sizeof apk_dest, where, get_basename(apk_file));
    int err = _do_sync_push(apk_file, apk_dest, 0 /* no show progress */);
    if (err) {
        goto cleanup_apk;
    } else {
        argv[last_apk] = apk_dest; /* destination name, not source location */
    }

    char *result=_pm_command(transport, serial, argc, argv);
    if(result==NULL||strstr(result, "Success")==NULL) {
        err=5;
    }
    free(result);

cleanup_apk:
    _delete_file(transport, serial, apk_dest);
    return err;
}

static int _uninstall_app(transport_type transport, char* serial, int argc, char** argv) {
    /* if the user choose the -k option, we refuse to do it until devices are
       out with the option to uninstall the remaining data somehow (adb/ui) */
    if (argc == 3 && strcmp(argv[1], "-k") == 0) {
        printf(
            "The -k option uninstalls the application while retaining the data/cache.\n"
            "At the moment, there is no way to remove the remaining data.\n"
            "You will have to reinstall the application with the same signature, and fully uninstall it.\n"
            "If you truly wish to continue, execute 'adb shell pm uninstall -k %s'\n", argv[2]);
        return -1;
    }

    /* 'adb uninstall' takes the same arguments as 'pm uninstall' on device */
    char *result = _pm_command(transport, serial, argc, argv);
    free(result);
    return 0;
}

int adb_install_app(transport_type ttype, char *serial,
                    const char *apk, int r, int s) {
    char argv1[5] = "-r";
    char argv2[5] = "-s";
    char *argv[]= {
        "install",
        r?argv1:"",
        r?argv2:"",
        (char*)apk,
    };
    return _install_app(ttype, serial, 4, argv) == 0;
}

int adb_uninstall_app(transport_type ttype, char *serial,
                      const char *package) {
    char *argv[]= {
        "uninstall",
        (char*)package,
    };
    char *result= _pm_command(ttype, serial, 2, argv);
    free(result);
    return 1;
}

int adb_start_activity(transport_type ttype, char *serial, const char *activty) {
    int argc=2;
    char *argv[]= {
        "start",
        (char*)activty,
    };
    int ret=0;
    char *result= _am_command(ttype, serial, argc, argv);
    if(result&&strstr(result, "Error")==NULL) {
        ret=1;
    }
    free(result);
    return ret;
}
