/*
**      tgafile.c :     TGA file operation
*/

#define ATVISTA

#include        "hrcap.h"
#include "stdfox.h"
#include "pro_ext.h"

#define ESC             0x1B

#define BLKSIZE 60*1024U               /* read/write max buffer byte size     */

typedef struct
    {
    unsigned char idBytes;
    char cmapType;
    char dataType;
    char cmapDef[5];
    int xorigin;
    int yorigin;
    int width;
    int height;
    char psize;
    char descriptor;
} TGAHeader;

int FoxFError;

static MHANDLE SavedBuf;

static unsigned char *cmpbufStr;
static unsigned char *cmpbufEnd;
static unsigned char *cmpbufPos;
static int tgafh;

#ifdef FOXPRO
void    SaveMem() {
        if( !_MemAvail(0xff00) || !(SavedBuf = _AllocHand(0xff00)) )
                ErrorMessage("Unable to pre-allocate file buffer");
}
#endif

static Boolean CheckSame( char *s0, char *s1, int len )
{
    while ( len-- )
        if ( *s0++ != *s1++ )
            return false;
    return true;
}

static int FlushCData( )
{
    unsigned wlen, blen;

    blen = cmpbufPos - cmpbufStr;
    wlen = xwrite( tgafh, cmpbufStr, blen );
    if ( wlen != blen ) wlen = -1;
    return wlen;
}

static int PutCData( char *bp, unsigned len )
{
    unsigned wlen, blen;

    if ( cmpbufPos + len > cmpbufEnd )
        {
        blen = cmpbufPos - cmpbufStr;
        wlen = xwrite( tgafh, cmpbufStr, blen );
        if ( wlen != blen )
            return - 1;
        cmpbufPos = cmpbufStr;
        }
    movemem( bp, cmpbufPos, len );
    cmpbufPos += len;
    return len;
}

static Compress( char *bstr, char *bend, int pix )
{
    char *bp, len;
    int ret = 0;

    while ( bstr < bend )
        {
        if ( CheckSame( bstr, bstr + pix, pix ) )
            {
            for ( bp = bstr + pix; ( bp < bstr + pix * 128 ) && ( bp < bend ); bp += pix )
                {
                if ( CheckSame( bp - pix, bp, pix ) == false )
                    break;
                }
            len = ( bp - bstr ) / pix + 127;
            PutCData(( char * ) &len, 1 );
            ret = PutCData( bp - pix, pix );
            }
        else
            {
            for ( bp = bstr + pix; ( bp < bstr + pix * 128 ) && ( bp < bend ); bp += pix )
                {
                if ( CheckSame( bp, bp + pix, pix ) == true )
                    break;
                }
            len = ( bp - bstr ) / pix - 1;
            PutCData(( char * ) &len, 1 );
            ret = PutCData( bstr, bp - bstr );
            }
        bstr = bp;
        }
    return ret;
}

static int SaveType10( GPtr port, Rect *r )
{
    EventRecord event;
    unsigned height, widthBytes, pixBytes, buffBytes;
    unsigned maxWidth, maxHeight, cnt;
    unsigned long vramAddr, vramOffset;
    char *buffPtr, *bstr, *bend;
    int result;
    unsigned len;
    int type, ret;

    /* get byte counts in a pixel */
    pixBytes = port->psize / 8;
    if ( pixBytes == 4 )
        pixBytes = 3;

    /* get write buffer */
    widthBytes = ( r->right - r->left ) * pixBytes;
    buffBytes = widthBytes * (BLKSIZE / widthBytes );

    /* get rest room for wrost compression */
    if ( ( buffPtr = ( char * ) _xfmalloc( buffBytes ) ) == 0 )
        return - 3;                    /* memory allocation error             */

    if ( ( cmpbufStr = ( char * ) _xfmalloc( BLKSIZE ) ) == 0 )
        {
        _xffree( buffPtr );
        return - 3;
        }
    cmpbufPos = cmpbufStr;
    cmpbufEnd = cmpbufStr + BLKSIZE;

    /* get physical address in the Vista memory */
    vramAddr = port->baseAddr +
    ( port->portRect.bottom - 1 - r->top ) * port->rowBits +
    r->left * port->psize;
    vramOffset = port->rowBits * (-1 );

    maxHeight = r->bottom - r->top;
    maxWidth = r->right - r->left;

    /* set memory type for _ReadVstaMem */
    type = ( port->psize / 8 ) + 3;

    bend = buffPtr + buffBytes;

    bstr = buffPtr;
    for ( height = 0; height < maxHeight; height++ )
        {
//              PutPercent(height,maxHeight);
        if ( bstr >= bend )
            {
            ret = Compress( buffPtr, bend, pixBytes );
            if ( ret == -1 )
                {
                ret = -2;
                goto END;
                }
            bstr = buffPtr;
            }
        _ReadVstaMem(( int * ) bstr, vramAddr, maxWidth, type );
        bstr += widthBytes;
        vramAddr += vramOffset;

        GetEvent( &event );
        if ( event.what == keydown && event.message == ESC )
            goto END;
        }
    /* flush data buffer */
    if ( bstr != buffPtr )
        {
        ret = Compress( buffPtr, bstr, pixBytes );
        if ( ret == -1 )
            ret = -2;
        ret = FlushCData( );
        if ( ret == -1 )
            ret = -2;
        }
    END :;
    _xffree( buffPtr );
    _xffree( cmpbufStr );
    return ret;
}

static int SaveType11( GPtr port, Rect *r )
{
    EventRecord event;
    unsigned height, widthBytes, buffBytes;
    unsigned maxWidth, maxHeight, cnt;
    unsigned long vramAddr, vramOffset;
    char *buffPtr, *bstr, *bend;
    int result;
    unsigned len;
    int type, ret;

    /* get write buffer */
    widthBytes = r->right - r->left;
    buffBytes = widthBytes * (BLKSIZE / widthBytes );

    /* get rest room for wrost compression */
    if ( ( buffPtr = ( char * ) _xfmalloc( buffBytes ) ) == 0 )
        return - 3;                    /* memory allocation error             */

    if ( ( cmpbufStr = ( char * ) _xfmalloc( BLKSIZE ) ) == 0 )
        {
        _xffree( buffPtr );
        return - 3;
        }
    cmpbufPos = cmpbufStr;
    cmpbufEnd = cmpbufStr + BLKSIZE;

    /* get physical address in the Vista memory */
    vramAddr = port->baseAddr +
    ( port->portRect.bottom - 1 - r->top ) * port->rowBits +
    r->left * port->psize;
    vramOffset = port->rowBits * (-1 );

    maxHeight = r->bottom - r->top;
    maxWidth = r->right - r->left;

    /* set memory type for _ReadVstaMem */
    type = port->psize / 8;

    bend = buffPtr + buffBytes;

    bstr = buffPtr;
    for ( height = 0; height < maxHeight; height++ )
        {
//              PutPercent(height,maxHeight);
        if ( bstr >= bend )
            {
            ret = Compress( buffPtr, bend, 1 );
            if ( ret == -1 )
                {
                ret = -2;
                goto END;
                }
            bstr = buffPtr;
            }
        _ReadVstaMem(( int * ) bstr, vramAddr, maxWidth, type );
        bstr += widthBytes;
        vramAddr += vramOffset;

        GetEvent( &event );
        if ( event.what == keydown && event.message == ESC )
            goto END;
        }
    /* flush data buffer */
    if ( bstr != buffPtr )
        {
        ret = Compress( buffPtr, bstr, 1 );
        if ( ret == -1 )
            ret = -2;
        ret = FlushCData( );
        if ( ret == -1 )
            ret = -2;
        }
    END :;
    _xffree( buffPtr );
    _xffree( cmpbufStr );
    return ret;
}

static int SaveType2( GPtr port, Rect *r )
{
    EventRecord event;
    unsigned height, widthBytes, pixBytes, buffBytes;
    unsigned maxHeight, cnt;
    unsigned long vramAddr, vramOffset;
    unsigned char *buffPtr, *tp;
    int type, ret = 0;

    /* get byte counts in a pixel */
    if ( port->psize == 32 )
        pixBytes = 3;
    else
        pixBytes = port->psize / 8;

    /* get write buffer */
    widthBytes = ( r->right - r->left ) * pixBytes;
    buffBytes = widthBytes * (BLKSIZE / widthBytes );
#ifdef FOXPRO
        _HLock(SavedBuf);
        if ((buffPtr = _HandToPtr(SavedBuf)) == 0)
    #else
        if ( ( buffPtr = ( char * ) _xfmalloc( buffBytes ) ) == 0 )
    #endif
            return - 3;                /* memory allocation error             */

    /* get physical address in the Vista memory */
    vramAddr = port->baseAddr +
    ( port->portRect.bottom - 1 - r->top ) * port->rowBits +
    r->left * port->psize;
    vramOffset = port->rowBits * (-1 );

    maxHeight = r->bottom - r->top;

    /* set memory type for _ReadVstaMem */
    type = ( port->psize / 8 ) + 3;

    for ( height = 0, tp = buffPtr; height < maxHeight; height++ )
        {
//              PutPercent(height,maxHeight);
        _ReadVstaMem(( int * ) tp, vramAddr, widthBytes / pixBytes, type );
        tp += widthBytes;
        vramAddr += vramOffset;
        if ( tp >= buffPtr + buffBytes )
            {
            cnt = xwrite( tgafh, buffPtr, tp - buffPtr );
            if ( cnt != tp - buffPtr )
                {
                ret = -2;
                goto END;              /* data write error                    */
                }
            tp = buffPtr;
            }

        GetEvent( &event );
        if ( event.what == keydown && event.message == ESC )
            goto END;
        }
    if ( tp != buffPtr )
        {                              /* flush data buffer                   */
        cnt = xwrite( tgafh, buffPtr, tp - buffPtr );
        if ( cnt != tp - buffPtr )
            ret = -2;                  /* data write error                    */
        }
END :
#ifdef FOXPRO
        _HUnLock(SavedBuf);
    #else
        _xffree( buffPtr );
    #endif
    return ret;
}

static int SaveType3( GPtr port, Rect *r )
{
    EventRecord event;
    unsigned height, widthBytes, buffBytes;
    unsigned maxHeight, cnt;
    unsigned long vramAddr, vramOffset;
    unsigned char *buffPtr, *tp;
    int type, ret = 0;

    /* get write buffer */
    widthBytes = ( r->right - r->left );
    buffBytes = widthBytes * (BLKSIZE / widthBytes );
    if ( ( buffPtr = ( char * ) _xfmalloc( buffBytes ) ) == 0 )
        return - 3;                    /* memory allocation error             */

    /* get physical address in the Vista memory */
    vramAddr = port->baseAddr +
    ( port->portRect.bottom - 1 - r->top ) * port->rowBits +
    r->left * port->psize;
    vramOffset = port->rowBits * (-1 );

    maxHeight = r->bottom - r->top;

    /* set memory type for _ReadVstaMem */
    type = port->psize / 8;

    for ( height = 0, tp = buffPtr; height < maxHeight; height++ )
        {
//              PutPercent(height,maxHeight);
        _ReadVstaMem(( int * ) tp, vramAddr, widthBytes, type );
        tp += widthBytes;
        vramAddr += vramOffset;
        if ( tp >= buffPtr + buffBytes )
            {
            cnt = xwrite( tgafh, buffPtr, tp - buffPtr );
            if ( cnt != tp - buffPtr )
                {
                ret = -2;
                goto END;              /* data write error                    */
                }
            tp = buffPtr;
            }

        GetEvent( &event );
        if ( event.what == keydown && event.message == ESC )
            goto END;
        }
    if ( tp != buffPtr )
        {                              /* flush data buffer                   */
        cnt = xwrite( tgafh, buffPtr, tp - buffPtr );
        if ( cnt != tp - buffPtr )
            ret = -2;                  /* data write error                    */
        }
END :
    _xffree( buffPtr );
    return ret;
}

int SaveTgaFile( char *filename, GPtr port, Rect *r, int fileType )
{
    TGAHeader tgaHead;
    int ret, i, cnt;

    tgaHead.idBytes = 0;
    tgaHead.cmapType = 0;
    tgaHead.dataType = fileType;
    for ( i = 0; i < 5; i++ )
        tgaHead.cmapDef[i] = 0;
    tgaHead.xorigin = 0;
    tgaHead.yorigin = 0;
    tgaHead.width = r->right - r->left;
    tgaHead.height = r->bottom - r->top;
    tgaHead.descriptor = 0x20;         /* origin in upper-left corner         */

    tgaHead.psize = ( port->psize == 32 ) ? 24 : port->psize;
    if ( fileType == TGAType3 || fileType == TGAType11 )
        tgaHead.psize = 8;

    tgafh = xopen( filename, O_CREAT | O_TRUNC | O_BINARY | O_RDWR );// ,S_IREAD|S_IWRITE);
    if ( tgafh == -1 )
        return - 1;                    /* file open error                     */

    /* write header spec to the TGA file */
    cnt = xwrite( tgafh, ( char * ) &tgaHead, sizeof ( TGAHeader ) );
    if ( cnt != sizeof ( TGAHeader ) )
        {
        xclose( tgafh );
        return - 2;                    /* write error                         */
        }

    switch ( fileType )
        {
        case TGAType2 :
            ret = SaveType2( port, r );
            break;
        case TGAType3 :
            ret = SaveType3( port, r );
            break;
        case TGAType10 :
            ret = SaveType10( port, r );
            break;
        case TGAType11 :
            ret = SaveType11( port, r );
            break;
        }

    xclose( tgafh );
    return ret;
}

static int GetCData( char *bp, unsigned len )
{
    unsigned rlen, blen;

    blen = cmpbufEnd - cmpbufPos;
    if ( len > blen )
        {
        movemem( cmpbufPos, cmpbufStr, blen );
        rlen = xread( tgafh, cmpbufStr + blen, BLKSIZE - blen );
        if ( rlen == -1 ) rlen = 0;
        blen += rlen;
        cmpbufPos = cmpbufStr;
        cmpbufEnd = cmpbufStr + blen;
        }
    if ( blen == 0 )
        return 0;
    else
        {
        len = min( len, blen );
        movemem( cmpbufPos, bp, len );
        cmpbufPos += len;
        return len;
        }
}

static unsigned DeCompress( char *bstr, char *bend, int pixBytes )
{
    unsigned length;
    unsigned char len, *bp;
    int ret;

    bp = bstr;

    while ( bp < bend )
        {
        ret = GetCData( &len, 1 );     /* get length                          */
        if ( ret == 0 )
            break;
        if ( len < 128 )
            {
            length = ( ( unsigned ) len + 1 ) * pixBytes;
            ret = GetCData( bp, length );
            if ( ret == 0 )
                break;
            }
        else
            {
            length = ( ( unsigned ) len - 127 ) * pixBytes;
            ret = GetCData( bp, pixBytes );
            if ( ret == 0 )
                break;
            CopyPat( bp, ( unsigned ) len - 127, pixBytes );
            }
        bp += length;
        }
    return bp - bstr;
}

static int LoadType10( GPtr port, TGAHeader *header )
{
    EventRecord event;
    unsigned height, widthBytes, pixBytes, buffBytes;
    int maxWidth, maxHeight;
    unsigned long vramAddr, vramOffset;
    unsigned char *buffPtr, *bstr, *bend, *bp;
    int readHeight;
    unsigned len;
    int type;

    /* get byte counts in a pixel */
    pixBytes = header->psize / 8;

    /* get write buffer */
    widthBytes = header->width * pixBytes;
    buffBytes = widthBytes * (BLKSIZE / widthBytes );
    if ( ( buffPtr = ( char * ) _xfmalloc( buffBytes + 128 * 4 ) ) == 0 )
        return - 3;                    /* memory allocation error             */

    if ( ( cmpbufStr = ( char * ) _xfmalloc( BLKSIZE ) ) == 0 )
        {
        _xffree( buffPtr );
        return - 3;
        }
    cmpbufPos = cmpbufEnd = cmpbufStr;

    /* get physical load address in the Vista memory */
    if ( header->descriptor & 0x20 )
        {
        vramAddr = port->baseAddr +
        ( port->portRect.bottom - 1 ) * port->rowBits;
        vramOffset = port->rowBits * (-1 );
        }
    else
        {
        vramAddr = port->baseAddr;
        vramOffset = port->rowBits;
        }

    maxHeight = min( port->portRect.bottom, header->height );
    maxWidth = min( port->portRect.right, header->width );

    /* set memory type for _WriteVstaMem */
    type = ( header->psize / 8 ) * 3 + ( port->psize / 8 ) - 2;

    bstr = bend = buffPtr + buffBytes;
    height = 0;
    while ( height < maxHeight )
        {
//              PutPercent(height,maxHeight);
        len = bend - bstr;
        if ( len < widthBytes )
            {
            movemem( bstr, buffPtr, len );
            bstr = buffPtr;
            bend = bstr + len;
            len = DeCompress( bend, buffPtr + buffBytes, pixBytes );
            bend += len;
            }
        _WriteVstaMem(( int * ) bstr, vramAddr, maxWidth, type );
        bstr += widthBytes;
        vramAddr += vramOffset;
        height++;

        GetEvent( &event );
        if ( event.what == keydown && event.message == ESC )
            goto END;
        }
END :
    _xffree( buffPtr );
    _xffree( cmpbufStr );
    return 0;
}

static int LoadType11( GPtr port, TGAHeader *header )
{
    EventRecord event;
    unsigned height, widthBytes, buffBytes;
    int maxWidth, maxHeight;
    unsigned long vramAddr, vramOffset;
    unsigned char *buffPtr, *bstr, *bend, *bp;
    int readHeight, ret = 0;
    unsigned len;
    int type;

    /* get write buffer */
    widthBytes = header->width;
    buffBytes = widthBytes * (BLKSIZE / widthBytes );
    if ( ( buffPtr = ( char * ) _xfmalloc( buffBytes + 128 * 4 ) ) == 0 )
        return - 3;                    /* memory allocation error             */

    if ( ( cmpbufStr = ( char * ) _xfmalloc( BLKSIZE ) ) == 0 )
        {
        _xffree( buffPtr );
        return - 3;
        }
    cmpbufPos = cmpbufEnd = cmpbufStr;

    /* get physical load address in the Vista memory */
    if ( header->descriptor & 0x20 )
        {
        vramAddr = port->baseAddr +
        ( port->portRect.bottom - 1 ) * port->rowBits;
        vramOffset = port->rowBits * (-1 );
        }
    else
        {
        vramAddr = port->baseAddr;
        vramOffset = port->rowBits;
        }

    maxHeight = min( port->portRect.bottom, header->height );
    maxWidth = min( port->portRect.right, header->width );

    /* set memory type for _WriteVstaMem */
    type = ( header->psize / 8 ) * 3 + ( port->psize / 8 ) - 2;

    bstr = bend = buffPtr + buffBytes;
    height = 0;
    while ( height < maxHeight )
        {
//              PutPercent(height,maxHeight);
        len = bend - bstr;
        if ( len < widthBytes )
            {
            movemem( bstr, buffPtr, len );
            bstr = buffPtr;
            bend = bstr + len;
            len = DeCompress( bend, buffPtr + buffBytes, 1 );
            bend += len;
            }
        _WriteVstaMem(( int * ) bstr, vramAddr, maxWidth, type );
        bstr += widthBytes;
        vramAddr += vramOffset;
        height++;

        GetEvent( &event );
        if ( event.what == keydown && event.message == ESC )
            goto END;
        }
END :
    _xffree( buffPtr );
    _xffree( cmpbufStr );
    return ret;
}

static int LoadType2( GPtr port, TGAHeader *header )
{
    EventRecord event;
    unsigned height, widthBytes, pixBytes, buffBytes;
    int maxWidth, maxHeight;
    unsigned long vramAddr, vramOffset;
    unsigned char *buffPtr, *bp;
    int readHeight, ret = 0;
    long len;
    int type;

    /* get byte counts in a pixel */
    pixBytes = header->psize / 8;

    /* get read buffer */
    widthBytes = header-> width * pixBytes;
    buffBytes = widthBytes * (BLKSIZE / widthBytes );
    if ( ( buffPtr = ( char * ) _xfmalloc( buffBytes ) ) == 0 )
        return - 3;                    /* memory allocation error             */

    /* get physical load address in the Vista memory */
    if ( header->descriptor & 0x20 )
        {
        vramAddr = port->baseAddr +
        ( port->portRect.bottom - 1 ) * port->rowBits;
        vramOffset = port->rowBits * (-1 );
        }
    else
        {
        vramAddr = port->baseAddr;
        vramOffset = port->rowBits;
        }

    /* set memory type for WriteVstaMem */
    type = ( header->psize / 8 ) * 3 + ( port->psize / 8 ) - 2;

    maxHeight = min( port->portRect.bottom, header->height );
    maxWidth = min( port->portRect.right, header->width );

    height = 0;
    while ( height < maxHeight )
        {
//              PutPercent(height,maxHeight);
        len = xread( tgafh, buffPtr, buffBytes );
        if ( len == 0 )
            break;
        else if ( len == -1 )
            {
            ret = -2;
            goto END;
            }
        readHeight = ( unsigned ) len / widthBytes;
        bp = buffPtr;
        while ( readHeight-- )
            {
            _WriteVstaMem(( int * ) bp, vramAddr, maxWidth, type );
            bp += widthBytes;
            vramAddr += vramOffset;
            if ( ++height >= maxHeight )
                goto END;
            }

        GetEvent( &event );
        if ( event.what == keydown && event.message == ESC )
            goto END;
        }
END :
    _xffree( buffPtr );
    return ret;
}

static int LoadType3( GPtr port, TGAHeader *header )
{
    EventRecord event;
    unsigned height, widthBytes, buffBytes;
    int maxWidth, maxHeight;
    unsigned long vramAddr, vramOffset;
    unsigned char *buffPtr, *bp;
    int readHeight, ret = 0;
    long len;
    int type;

    /* get read buffer */
    widthBytes = header-> width;
    buffBytes = widthBytes * (BLKSIZE / widthBytes );
    if ( ( buffPtr = ( char * ) _xfmalloc( buffBytes ) ) == 0 )
        return - 3;                    /* memory allocation error             */

    /* get physical load address in the Vista memory */
    if ( header->descriptor & 0x20 )
        {
        vramAddr = port->baseAddr +
        ( port->portRect.bottom - 1 ) * port->rowBits;
        vramOffset = port->rowBits * (-1 );
        }
    else
        {
        vramAddr = port->baseAddr;
        vramOffset = port->rowBits;
        }

    /* set memory type for WriteVstaMem */
    type = ( header->psize / 8 ) * 3 + ( port->psize / 8 ) - 2;

    maxHeight = min( port->portRect.bottom, header->height );
    maxWidth = min( port->portRect.right, header->width );

    height = 0;
    while ( height < maxHeight )
        {
//              PutPercent(height,maxHeight);
        len = xread( tgafh, buffPtr, buffBytes );
        if ( len == 0 )
            break;
        else if ( len == -1 )
            {
            ret = -2;
            goto END;
            }
        readHeight = ( unsigned ) len / widthBytes;
        bp = buffPtr;
        while ( readHeight-- )
            {
            _WriteVstaMem(( int * ) bp, vramAddr, maxWidth, type );
            bp += widthBytes;
            vramAddr += vramOffset;
            if ( ++height >= maxHeight )
                goto END;
            }

        GetEvent( &event );
        if ( event.what == keydown && event.message == ESC )
            goto END;
        }
END :
    _xffree( buffPtr );
    return ret;
}

int LoadTgaFile( char *filename, GPtr port, int *fileType )
{
    TGAHeader tgaHead;
    int ret;
    long len;

    if ( ( tgafh = xopen( filename, O_RDONLY | O_BINARY ) ) == -1 )
        return - 1;                    /* file open error                     */

    len = xread( tgafh, ( char * ) &tgaHead, sizeof ( TGAHeader ) );
    if ( len != ( long ) sizeof ( TGAHeader ) )
        {
        xclose( tgafh );
        return - 4;                    /* format error                        */
        }

    /* check TGA file format */
    if ( ( tgaHead.dataType == TGAType2 || tgaHead.dataType == TGAType10 ||
                tgaHead.dataType == TGAType3 || tgaHead.dataType == TGAType11 ) &&
            ( tgaHead.psize == 8 || tgaHead.psize == 16 ||
                tgaHead.psize == 24 || tgaHead.psize == 32 ) )
        ;
    else
        {
        xclose( tgafh );
        return - 4;                    /* format error                        */
        }

    /* skip identification field */
    len = ( long ) ( sizeof ( TGAHeader ) + tgaHead.idBytes );

    /* skip color map data field if it exist */
    if ( tgaHead.cmapType == 1 )
        len += ( long ) ( ( * ( int * ) &tgaHead.cmapDef[2] ) *
            ( int ) ( ( tgaHead.cmapDef[4] + 7 ) / 8 ) );
    xlseek( tgafh, len, 0 );

    switch ( tgaHead.dataType )
        {
        case TGAType2 :
            ret = LoadType2( port, &tgaHead );
            break;
        case TGAType3 :
            ret = LoadType3( port, &tgaHead );
            break;
        case TGAType10 :
            ret = LoadType10( port, &tgaHead );
            break;
        case TGAType11 :
            ret = LoadType11( port, &tgaHead );
            break;
        }

    xclose( tgafh );
    if ( ret == 0 )
        *fileType = tgaHead.dataType;

    return ret;
}

/*** end of tgafile.c ***/
