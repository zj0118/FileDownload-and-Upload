#pragma once
#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <minwindef.h>
#include <tchar.h>
#include <winnt.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>

#include <assert.h>
#include <sys/select.h>
#include <unistd.h>

#include <stdio.h>

typedef int SOCKET;

#define INVALID_SOCKET -1

#define SOCKET_ERROR -1

#define closesocket(s) close(s)

#define MAX_PATH          260

typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;
typedef int                 INT;
typedef unsigned int        UINT;
typedef unsigned int* PUINT;
#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

typedef char TCHAR;
typedef const char* LPCSTR, * PCSTR;
typedef const char* LPCTSTR * PCTSTR;
typedef void* HANDLE;

#define _tcscpy_s	strcpy
#define strcpy_s	strcpy
#endif
