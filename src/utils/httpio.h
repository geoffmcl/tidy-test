/*\
 *   httpio.h
\*/

#ifndef __HTTPIO_H__
#define __HTTPIO_H__

#include <tidy.h>

#ifdef WIN32
// CAN NOT DEFINE THIS!!! # define IPV6STRICT 1
# include <winsock2.h>
#ifndef ECONNREFUSED
# define ECONNREFUSED WSAECONNREFUSED
#endif
#else
# include <sys/socket.h>
# include <netdb.h>
# include <netinet/in.h>
#ifndef __BEOS__
# include <arpa/inet.h>
#endif
#endif /* WIN32 */

#ifndef MMX_BUFFER
#define MMX_BUFFER 1024*32
#endif
#ifndef MMX_UNGET
#define MMX_UNGET   64
#endif
// TIDY_STRUCT
typedef struct _HTTPInputSource
{
    TidyInputSource tis;    //  This declaration must be first and must not be changed!

    tmbstr pHostName;
    tmbstr pResource;
    unsigned short nPort;
    unsigned int nextBytePos, nextUnGotBytePos, nBufSize;
    SOCKET s;
    char recvbuffer[MMX_BUFFER];
    char unGetBuffer[MMX_UNGET];

} HTTPInputSource, *PHTTPInputSource;

#ifdef __cplusplus
extern "C" {
#endif

/*  get next byte from input source */
extern int HTTPGetByte( HTTPInputSource *source );

/*  unget byte back to input source */
extern void HTTPUngetByte( HTTPInputSource *source, uint byteValue );

/* check if input source at end */
extern Bool HTTPIsEOF( HTTPInputSource *source );

extern int parseURL( HTTPInputSource* source, tmbstr pUrl );

extern int openURL( HTTPInputSource* source, tmbstr pUrl );

extern void closeURL( HTTPInputSource *source );

#ifdef __cplusplus
}
#endif


#endif
/* eof */
