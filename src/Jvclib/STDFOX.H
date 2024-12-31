/*==========================================
 *    Function Prototypes
 *-----------------------------------------*/
void __far *xmalloc( size_t size );
void __far *_xfmalloc( size_t size );
void xfree( void __far *mem );
void _xffree( void __far *mem );

int xopen( char *filename, int mode );
int xclose( int chan );
int xread( int chan, void *buf, unsigned cnt );
int xgets( int chan, void *buf, unsigned cnt );
int xwrite( int chan, void *buf, unsigned cnt );
int xlseek( int chan, long offset, int origin );

