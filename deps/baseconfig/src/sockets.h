/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Single include to mask most sockets api differnces between platforms.
 *
 * @copyright Baseconfig is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#ifndef BASE_SOCKETS_H
#define BASE_SOCKETS_H

#ifdef _WIN32
// We only want the old sockets interface for ABI compat.
#ifdef BUILD_WITH_WINSOCK
#include <winsock.h>
typedef int socklen_t;
typedef char raw_type;
#else // Otherwise we use the winsock2 interface.
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#define LastSocketError WSAGetLastError()

#define SOCKEISCONN WSAEISCONN
#define SOCKEINPROGRESS WSAEINPROGRESS
#define SOCKEALREADY WSAEALREADY
#define SOCKEADDRINUSE WSAEADDRINUSE
#define SOCKEADDRNOTAVAIL WSAEADDRNOTAVAIL
#define SOCKEBADF WSAEBADF
#define SOCKECONNREFUSED WSAECONNREFUSED
#define SOCKEINTR WSAEINTR
#define SOCKENOTSOCK WSAENOTSOCK
#define SOCKEWOULDBLOCK WSAEWOULDBLOCK
#define SOCKEINVAL WSAEINVAL
#define SOCKETIMEDOUT WSAETIMEDOUT
#define SOCKEAGAIN WSAEWOULDBLOCK

#else

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h> // for getaddrinfo() and freeaddrinfo()
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // for close()
typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define opensocket open
#define closesocket close
#define ioctlsocket ioctl
#define LastSocketError (errno)

#define SOCKEISCONN EISCONN
#define SOCKEINPROGRESS EINPROGRESS
#define SOCKEALREADY EALREADY
#define SOCKEADDRINUSE EADDRINUSE
#define SOCKEADDRNOTAVAIL EADDRNOTAVAIL
#define SOCKEBADF EBADF
#define SOCKECONNREFUSED ECONNREFUSED
#define SOCKEINTR EINTR
#define SOCKENOTSOCK ENOTSOCK
#define SOCKEWOULDBLOCK EWOULDBLOCK
#define SOCKEINVAL EINVAL
#define SOCKETIMEDOUT ETIMEDOUT
#define SOCKEAGAIN EAGAIN

#endif // _WIN32

#endif // BASE_SOCKETS_H
