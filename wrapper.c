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

/*
 * 启动adb守护进程
 * 返回1表示成功，0表示失败
 */
int start_server(int is_daemon, unsigned short port) {
    int pfds[2];
    if(pipe(pfds)) { /* 创建管道失败 */
        return 0;
    }
    pid_t pid = fork();
    if(pid<0) {
        close(pfds[0]);
        close(pfds[1]);
        return 0;
    } else if(pid==0) {
        /* 子进程执行adb_main函数 */
        setproctitle("adbd");
        if(daemon(0, 1)) {
            _exit(1);
        }
        close(pfds[0]);
        dup2(pfds[1], 2);
        dup2(pfds[0], 1);
        int r = adb_main(is_daemon, port);
        close(pfds[1]);
        _exit(r);
    }
    close(pfds[1]);
    char buf[16];
    int n = read(pfds[0], buf, sizeof(buf));
    close(pfds[0]);
    waitpid(pid, NULL, WNOHANG);
    return strncmp(buf, "OK", 2) == 0;
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
