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
#include <sys/wait.h>
#include <stdlib.h>

int install_app(transport_type transport, char* serial, int argc, char** argv);

/*
 * 启动adb守护进程
 * 返回1表示成功，0表示失败
 */
int start_server(unsigned short port) {
    return launch_server(port) == 0;
}

/*
 * 关闭adb守护进程
 */
int kill_server(unsigned short port) {
    adb_set_tcp_specifics(port);
    int fd = _adb_connect("host:kill");
    if(fd == -1) {
        return 0;
    }
    return 1;
}

/* 安装apk，r表示重新安装 */
int install_apk(const char *path, int r) {
    int argc = 2;
    char *argv[3] = {
        "install",
        (char*)path,
    };
    if(r) {
        argc = 3;
        argv[1] = "-r";
        argv[2] = (char*)path;
    }
    return install_app(kTransportUsb, NULL, argc, argv) == 0;
}
