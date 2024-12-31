#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <sys\stat.h>
#include "stdfox.h"
#ifdef FOXPRO
#include <fcntl.h>
#include <pro_ext.h>
#else
    #include <malloc.h>
#endif

void __far *xmalloc( size_t size )
{
#ifdef FOXPRO
   MHANDLE  handle;
   MHANDLE __far *ptr;
/*
char slen[30];
sprintf(slen, "xmalloc() - %u",size);
puts(slen);
*/
        if( !_MemAvail(size) || !(handle = _AllocHand(size+sizeof(MHANDLE))) )
                return NULL;

   _HLock(handle);

   ptr   = _HandToPtr(handle);
   *ptr++= handle;

   return ptr;
    #else
        return malloc( size );
    #endif
}

void __far *_xfmalloc( size_t size )
{
#ifdef FOXPRO
   return xmalloc(size);
    #else
        return _fmalloc( size );
    #endif
}

void xfree( void __far *mem )
{
#ifdef FOXPRO
   MHANDLE handle;
//puts("xfree()");
   handle = * ((MHANDLE __far*)mem - 1);
   _HUnLock(handle);
   _FreeHand(handle);
    #else
        free( mem );
    #endif
}

void _xffree( void __far *mem )
{
#ifdef FOXPRO
   xfree(mem);
    #else
        _ffree( mem );
    #endif
}

#ifdef FOXPRO
int     SavedFD = -1;

int SaveFD()
{
        char filename[_MAX_PATH];
        extern char SetupPath[];

        sprintf(filename,"%s\\foo.bar",SetupPath);
        SavedFD = open(filename,O_CREAT|O_TRUNC|O_RDWR,S_IREAD|S_IWRITE);
        if(SavedFD == -1)
                _UserError("Unable to Reserve file descriptor");
}

int ReleaseFD()
{
        if(SavedFD!=-1) {
                close(SavedFD);
                SavedFD = -1;
        }
}
#endif

#undef FOXPRO

int xopen( char *filename, int mode )
{
#ifdef FOXPRO
   int   newmode = 0;
   FCHAN chan;

   if(mode & O_RDONLY)
      newmode |= FO_READONLY;
   else
   if(mode & O_WRONLY)
      newmode |= FO_WRITEONLY;
   else
   if(mode & O_RDWR)
      newmode |= FO_READWRITE;
   //
   // Create file if file is to be truncated OR
   //    does not exist and should be created THEN
   //       Create the file and return the opened file w/proper mode
   //
   if( mode&O_TRUNC || (
       ((chan = _FOpen(filename, newmode)) == -1) && mode&O_CREAT ))
   {
      chan = _FCreate(filename,FC_NORMAL);
      _FClose(chan);
      chan = _FOpen(filename,newmode);
   }
   return chan;
    #else
        return open( filename, mode, S_IREAD | S_IWRITE );
    #endif
}

int xclose( int chan )
{
#ifdef FOXPRO
   return _FClose(chan);
    #else
        return close( chan );
    #endif
}

int xread( int chan, void *buf, unsigned cnt )
{
#ifdef FOXPRO
/*
char slen[10];
itoa(cnt,slen,10);
puts(slen);
*/
   return _FRead(chan,buf,cnt);
    #else
        unsigned chrs;
        chrs = read( chan, buf, cnt );
        return chrs;
    #endif
}

int xgets( int chan, char *buf, unsigned cnt )
{
#ifdef FOXPRO
   int chrs = _FGets(chan,buf,cnt);
   if(chrs == 0 && !_FEOF(chan))
      chrs = 1;
   return chrs;
    #else
        unsigned chrs;
        long curloc = tell ( chan );

        chrs = read( chan, buf, cnt );
        if ( chrs )
            {
            char *ptr;
            ptr = strchr( buf, '\n' );
            if ( ptr )
                {
                long offset;
                * ++ptr = '\0';
                chrs = ptr - buf;
                offset = curloc + chrs + 1;
                lseek( chan, offset, SEEK_SET );
                }
            }
        return chrs;
    #endif
}

int xwrite( int chan, void *buf, unsigned cnt )
{
#ifdef FOXPRO
   return _FWrite(chan,buf,cnt);
    #else
        return write( chan, buf, cnt );
    #endif
}

int xlseek( int chan, long offset, int origin )
{
#ifdef FOXPRO
   int mode;

   if(origin == SEEK_SET)
      mode = 0;
   else
   if(origin == SEEK_CUR)
      mode = 1;
   if(origin == SEEK_END)
      mode = 2;

   return _FSeek(chan,offset,mode);
    #else
        return lseek( chan, offset, origin );
    #endif
}

