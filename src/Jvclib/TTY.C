/*
**      tty.c
*/

#include        <stdio.h>
#include        <stdlib.h>
#include        <ctype.h>

#include        "hrcap.h"
#include        "term.h"

static int maxline, maxcol;
static int line, col, bl;
static char *buffp;

static view( )
{
    int i, l;
    char *p;

    curoff( );
    l = ( bl + 2 ) & 0x7;
    for ( i = 0; i < maxline - 1; i++ )
        {
        movein( &MsgBox, i, 0 );
        p = buffp + l * (maxcol + 1 );
        addstr( p );
        l = ( l + 1 ) & 0x7;
        }
    curon( );
}

static DrawTTY( int c )
{
    int i;
    char *p;

    switch ( c )
        {
        case 0x0D :
            col = 0;
            movein( &MsgBox, line, col );
            break;
        case 0x0A :
            if ( ++line >= maxline )
                {
                line = maxline - 1;
                view( );
                }
            movein( &MsgBox, line, 0 );
            repch( ' ', maxcol );
            bl = ( bl + 1 ) & 0x7;
            for ( i = 0, p = buffp + bl * (maxcol + 1 ); i < maxcol; i++ ) *p++= ' ';
            movein( &MsgBox, line, col );
            break;
        case 0x08 :
            if ( --col < 0 ) col = 0;
            movein( &MsgBox, line, col );
            break;
        default :
            if ( col < maxcol )
                {
                * ( buffp + bl * (maxcol + 1 ) + col ) = c;
                movein( &MsgBox, line, col );
                addch( c );
                col++;
                }
        }
}

DoTTY( )
{
    int ch, i;
    char *p;

    maxline = 8;
    maxcol = MsgBox.right - MsgBox.left - 1;
    if ( ( buffp = malloc( maxline * (maxcol + 1 ) ) ) == 0 )
        {
        ErrorMessage( "Not enough memory for TTY" );
        return;
        }
    for ( i = 0, p = buffp; i < maxline * (maxcol + 1 ); i++ ) *p++= ' ';
    for ( i = 0, p = buffp + maxcol; i < maxline; i++, p += maxcol + 1 ) *p = '\0';

    line = col = bl = 0;
    movein( &MsgBox, line, col );
    setatr( WB );

    curon( );                          /* cursor on                           */
    for (;; )
        {
        if ( keyhit( ) )
            {
            if ( ( ch = keyread( ) ) == 0x4F00 )/* END key                    */
                break;
            else
                Write_Modem( ch & 0x00FF );
            }
        if ( ( ch = Read_Modem( ) ) >= 0 )
            DrawTTY( ch );
        }
    free( buffp );
    curoff( );                         /* cursor off                          */
    clrbox( &MsgBox );
}

/*** end of tty.c ***/
