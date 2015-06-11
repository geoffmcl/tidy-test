/*\
 *   httpio.h
\*/

#ifndef __HTTPIO_H__
#define __HTTPIO_H__

#include <tidy.h>

#ifdef WIN32
# include <winsock2.h>
# define ECONNREFUSED WSAECONNREFUSED
#else
# include <sys/socket.h>
# include <netdb.h>
# include <netinet/in.h>
#ifndef __BEOS__
# include <arpa/inet.h>
#endif
#endif /* WIN32 */

// TIDY_STRUCT
typedef struct _HTTPInputSource
{
    TidyInputSource tis;    //  This declaration must be first and must not be changed!

    tmbstr pHostName;
    tmbstr pResource;
    unsigned short nPort, nextBytePos, nextUnGotBytePos, nBufSize;
    SOCKET s;
    char buffer[1024];
    char unGetBuffer[16];

} HTTPInputSource;

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
