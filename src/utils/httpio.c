/*\
 *   httpio.c
\*/

#include <tidy.h>
#include "httpio.h"
#include "sprtf.h"

#ifndef SPRTF
#define SPRTF printf
#endif

////////////////////////////////////////////////////////////////
// some tidy internal utilities
// #define TidyAlloc(allocator, size) ((allocator)->vtbl->alloc((allocator), (size)))
#define tmbstrdup strdup
#define MemAlloc malloc
#define MemFree  free

//#define CHKMEM(a) if (!a) { SPRTF("Memory FAILED!\n"); exit(2); }

/* lexer character types
*/
#define digit       1u
#define letter      2u
#define namechar    4u
#define white       8u
#define newline     16u
#define lowercase   32u
#define uppercase   64u
#define digithex    128u

#define MAP(c) ((unsigned)c < 128 ? lexmap[(unsigned)c] : 0)
static uint lexmap[128];
static Bool dn_init = no;

static void MapStr( ctmbstr str, uint code )
{
    while ( *str )
    {
        uint i = (byte) *str++;
        lexmap[i] |= code;
    }
}

void InitMap(void)
{
    MapStr("\r\n\f", newline|white);
    MapStr(" \t", white);
    MapStr("-.:_", namechar);
    MapStr("0123456789", digit|digithex|namechar);
    MapStr("abcdefghijklmnopqrstuvwxyz", lowercase|letter|namechar);
    MapStr("ABCDEFGHIJKLMNOPQRSTUVWXYZ", uppercase|letter|namechar);
    MapStr("abcdefABCDEF", digithex);
    dn_init = yes;
}


uint ToLower(uint c)
{
    uint map;

    if (!dn_init)
        InitMap();
    
    map = MAP(c);

    if (map & uppercase)
        c += 'a' - 'A';

    return c;
}

/* like strndup but using an allocator */
// tmbstr tmbstrndup( TidyAllocator *allocator, ctmbstr str, uint len )
tmbstr tmbstrndup( ctmbstr str, uint len )
{
    tmbstr s = NULL;
    if ( str && len > 0 )
    {
        tmbstr cp = s = (tmbstr) malloc( 1+len );
        if (!cp) {
            SPRTF("Memory allocation FAILED on %d bytes\n", 1+len);
            exit(2);
        }
        while ( len-- > 0 &&  (*cp++ = *str++) )
          /**/;
        *cp = 0;
    }
    return s;
}

/* returns byte count, not char count */
uint tmbstrlen( ctmbstr str )
{
    uint len = 0;
    if ( str ) 
    {
        while ( *str++ )
            ++len;
    }
    return len;
}

int tmbstrncasecmp( ctmbstr s1, ctmbstr s2, uint n )
{
    uint c;

    while (c = (uint)(*s1), ToLower(c) == ToLower((uint)(*s2)))
    {
        if (c == '\0')
            return 0;

        if (n == 0)
            return 0;

        ++s1;
        ++s2;
        --n;
    }

    if (n == 0)
        return 0;

    return (*s1 > *s2 ? 1 : -1);
}


ctmbstr tmbsubstr( ctmbstr s1, ctmbstr s2 )
{
    uint len1 = tmbstrlen(s1), len2 = tmbstrlen(s2);
    int ix, diff = len1 - len2;

    for ( ix = 0; ix <= diff; ++ix )
    {
        if ( tmbstrncasecmp(s1+ix, s2, len2) == 0 )
            return (ctmbstr) s1+ix;
    }
    return NULL;
}
////////////////////////////////////////////////////////////////////////////////////

int makeConnection ( HTTPInputSource *pHttp )
{
    struct sockaddr_in sock;
    struct hostent *pHost;

    /* Get internet address of the host. */
    if (!(pHost = gethostbyname ( pHttp->pHostName )))
    {
        return -1;
    }
    /* Copy the address of the host to socket description.  */
    memcpy (&sock.sin_addr, pHost->h_addr, pHost->h_length);

    /* Set port and protocol */
    sock.sin_family = AF_INET;
    sock.sin_port = htons( pHttp->nPort );

    /* Make an internet socket, stream type.  */
    if ((pHttp->s = socket (AF_INET, SOCK_STREAM, 0)) == -1)
        return -1;

    /* Connect the socket to the remote host.  */
    if (connect (pHttp->s, (struct sockaddr *) &sock, sizeof( sock )))
    {
        if (errno == ECONNREFUSED)
            return ECONNREFUSED;
        else
            return -1;
    }
    return 0;
}


int parseURL( HTTPInputSource *pHttp, tmbstr url )
{
  int i, j = 0;
  ctmbstr pStr;

    pStr = tmbsubstr( url, "://" );

    /* If protocol is there, but not http, bail out, else assume http.  */
    if (NULL != pStr)
    {
        if (tmbstrncasecmp( url, "http://", 7 ))
            return -1;
    }

    if (NULL != pStr)
        j = pStr - url + 3;
    for (i = j; url[i] && url[i] != ':' && url[i] != '/'; i++) {}
    if (i == j)
        return -1;

    /* Get the hostname.  */
    pHttp->pHostName = tmbstrndup (&url[j], i - j );

    if (url[i] == ':')
    {
        /* We have a colon delimiting the hostname.  It should mean that
        a port number is following it */
        pHttp->nPort = 0;
        if (isdigit( url[++i] ))    /* A port number */
        {
            for (; url[i] && url[i] != '/'; i++)
            {
                if (isdigit( url[i] ))
                    pHttp->nPort = 10 * pHttp->nPort + (url[i] - '0');
                else
                    return -1;
            }
            if (!pHttp->nPort)
                return -1;
        }
        else                      /* or just a misformed port number */
            return -1;
    }
    else
        /* Assume default port.  */
        pHttp->nPort = 80;

    /* skip past the delimiting slash (we'll add it later )  */
    while (url[i] && url[i] == '/')
        i++;
    pHttp->pResource = tmbstrdup (url + i );
    return 0;
}


int fillBuffer( HTTPInputSource *in )
{
    if (0 < in->s)
    {
        in->nBufSize = recv( in->s, in->recvbuffer, MMX_BUFFER, 0);
        in->nextBytePos = 0;
        if (in->nBufSize < 0)
            return in->nBufSize;    // return error
        in->recvbuffer[in->nBufSize] = 0;
    }
    else
        in->nBufSize = 0;
    return in->nBufSize;
}


int openURL( HTTPInputSource *in, tmbstr pUrl )
{
    int rc = -1;
    int nbp, nbs;
#ifdef WIN32    
    WSADATA wsaData;
    rc = WSAStartup( 514, &wsaData );
#endif
    // setup the input
    in->tis.getByte = (TidyGetByteFunc) HTTPGetByte; 
    in->tis.ungetByte = (TidyUngetByteFunc) HTTPUngetByte;
    in->tis.eof = (TidyEOFFunc) HTTPIsEOF;
    in->tis.sourceData = in;
    in->nextBytePos = in->nextUnGotBytePos = in->nBufSize = 0;

    parseURL( in, pUrl );
    if (0 == (rc = makeConnection( in )))
    {
        char ch, lastCh = '\0';
        int blanks = 0;
        size_t len = 48 + strlen( in->pResource );
        char *getCmd = (char *)MemAlloc(len);
        if (!getCmd) {
            SPRTF("Memory FAILED on %d byes\n", (int)len);
            return 2;
        }
        sprintf( getCmd, "GET /%s HTTP/1.0\r\nAccept: text/html\r\n\r\n", in->pResource );
        len = strlen(getCmd);
        rc = send( in->s, getCmd, len, 0 );
        MemFree( getCmd );

        if (rc == -1) {
            SPRTF("send FAILED with %d!\n",rc);
            return 2;
        }
        if (rc != (int)len) {
            SPRTF("send FAILED to send req %d! got %d\n",(int)len, rc);
            return 2;
        }

        rc = fillBuffer(in);

        nbp = in->nextBytePos;
        nbs = in->nBufSize;
        /*  skip past the header information  */

        while (  (in->nextBytePos < in->nBufSize) && (rc > 0)) {
            if (1 < blanks)
                break;
            for (; in->nextBytePos < MMX_BUFFER && 0 != in->recvbuffer[ in->nextBytePos ]; 
                 in->nextBytePos++ )
            {
                ch = in->recvbuffer[ in->nextBytePos ];
                if (ch == '\r' || ch == '\n')
                {
                    if (ch == lastCh)
                    {
                        /*  Two carriage returns or two newlines in a row,
                            that's good enough */
                        blanks++;
                    }
                    if (lastCh == '\r' || lastCh == '\n')
                    {
                        blanks++;
                    }
                }
                else
                    blanks = 0;
                lastCh = ch;
                if (1 < blanks)
                {
                    /* end of header, scan to first non-white and return */
                    while ('\0' != ch && isspace( ch ))
                        ch = in->recvbuffer[ ++in->nextBytePos ];
                    break;
                }
            }
            if (1 < blanks)
                break;
            rc = fillBuffer(in);
        }
    }
    if (rc > 0)
        return 0;
    return rc;
}


void closeURL( HTTPInputSource *source )
{
    if (0 < source->s)
        closesocket( source->s );
    source->s = -1;
    source->tis.sourceData = 0;
#ifdef WIN32
    WSACleanup();
#endif
}


int HTTPGetByte( HTTPInputSource *source )
{
    if (source->nextUnGotBytePos)
        return source->unGetBuffer[ --source->nextUnGotBytePos ];
    if (0 != source->nBufSize && source->nextBytePos >= source->nBufSize)
    {
        fillBuffer( source );
    }
    if (0 == source->nBufSize)
        return EndOfStream;
    return source->recvbuffer[ source->nextBytePos++ ];
}

void HTTPUngetByte( HTTPInputSource *source, uint byteValue )
{
    if (source->nextUnGotBytePos < 16 )  /* Only you can prevent buffer overflows */
        source->unGetBuffer[ source->nextUnGotBytePos++ ] = (char) byteValue;
}

Bool HTTPIsEOF( HTTPInputSource *source )
{
    if (source->nextUnGotBytePos)
        /* pending ungot bytes, not done */
        return no;

    if (   0 != source->nBufSize 
        && source->nextBytePos >= source->nBufSize)
        /* We've consumed the existing buffer, get another */
        fillBuffer( source );

    if (source->nextBytePos < source->nBufSize)
        /*  we have stuff in the buffer, must not be done. */
        return no;

    /* Nothing in the buffer, and the last receive failed, must be done.  */
    return yes;
}

// eof
