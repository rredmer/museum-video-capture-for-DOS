/* video subroutine in C */

#define LINE    25
#define COL     80
#define WB      0x0007                 /* character attribute white & black   */

typedef struct
    {
    int left;
    int top;
    int right;
    int bottom;
} Rect;

extern repch( int, int );
extern setatr( int );
extern addstr( char * );
extern addch( int );
extern move( int, int );

box( Rect *rp )
{
    register int i, col;

    col = rp->right - rp->left - 1;
    move( rp->top, rp->left );
    setatr( WB );
    addch( 218 );
    repch( 196, col );
    addch( 191 );
    for ( i = rp->top + 1; i < rp->bottom; i++ )
        {
        move( i, rp->left );
        addch( 179 );
        repch( ' ', col );
        addch( 179 );
        }
    move( rp->bottom, rp->left );
    addch( 192 );
    repch( 196, col );
    addch( 217 );
}
dbox( Rect *rp )
{
    register int i, col;

    col = rp->right - rp->left - 1;
    move( rp->top, rp->left );
    setatr( WB );
    addch( 201 );
    repch( 205, col );
    addch( 187 );
    for ( i = rp->top + 1; i < rp->bottom; i++ )
        {
        move( i, rp->left );
        addch( 186 );
        repch( ' ', col );
        addch( 186 );
        }
    move( rp->bottom, rp->left );
    addch( 200 );
    repch( 205, col );
    addch( 188 );
}
clrbox( Rect *rp )
{
    register int i, col;

    col = rp->right - rp->left - 1;
    setatr( WB );
    for ( i = rp->top + 1; i <= rp->bottom - 1; i++ )
        {
        move( i, rp->left + 1 );
        repch( ' ', col );
        }
}
movein( Rect *rp, int line, int col )
{
    move( rp->top + line + 1, rp->left + col + 1 );
}

/*** end of videoc.c ***/

