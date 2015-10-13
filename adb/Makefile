#SRCS+= adb.c
SRCS+= adb_client.c
SRCS+= adb_auth_host.c
SRCS+= commandline.c
SRCS+= console.c
SRCS+= file_sync_client.c
SRCS+= fdevent.c
SRCS+= get_my_path_linux.c
SRCS+= services.c
SRCS+= sockets.c
SRCS+= transport.c
SRCS+= transport_local.c
SRCS+= transport_usb.c
SRCS+= usb_linux.c
SRCS+= usb_vendors.c

#VPATH+= libcutils
SRCS+= libcutils/socket_inaddr_any_server.c
SRCS+= libcutils/socket_local_client.c
SRCS+= libcutils/socket_local_server.c
SRCS+= libcutils/socket_loopback_client.c
SRCS+= libcutils/socket_loopback_server.c
SRCS+= libcutils/socket_network_client.c
SRCS+= libcutils/load_file.c

#VPATH+= libzipfile
SRCS+= libzipfile/centraldir.c
SRCS+= libzipfile/zipfile.c

#VPATH+= libmincrypt
SRCS+= libmincrypt/rsa.c
SRCS+= libmincrypt/sha256.c
SRCS+= libmincrypt/sha.c
SRCS+= libmincrypt/p256.c
SRCS+= libmincrypt/p256_ec.c
SRCS+= libmincrypt/p256_ecdsa.c

CPPFLAGS+= -DADB_HOST=1
CPPFLAGS+= -DHAVE_FORKEXEC=1
CPPFLAGS+= -DHAVE_SYMLINKS
CPPFLAGS+= -DHAVE_TERMIO_H
CPPFLAGS+= -D_GNU_SOURCE
CPPFLAGS+= -D_XOPEN_SOURCE
CPPFLAGS+= -D_FILE_OFFSET_BITS=64
CPPFLAGS+= -DHAVE_OFF64_T
CPPFLAGS+= -I.
CPPFLAGS+= -Iinclude

CFLAGS+= $(RPM_OPT_FLAGS)
LIBS= -lrt -lpthread -lz -lcrypto
LDFLAGS+= $(RPM_LD_FLAGS)

CC= $(TOOLCHAIN)gcc
LD= $(TOOLCHAIN)gcc

#OBJS= $(SRCS:.c=.o)

all: adb.so

adb.so: $(OBJS)
	$(LD) -o $@ $(CPPFLAGS) $(SRCS) -shared -fPIC

clean:
	rm -rf adb.so
